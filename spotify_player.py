###!
 # @file spotify_player.py
 # @brief Making API requests to Spotify
 # This program plays the song/album on Spotify sent by Serial Communication, and also sends the current song being played
 # on Spotify to the Serial
###

from dotenv import load_dotenv
import os
import json
import time
import serial
from pprint import pprint
import random
import unidecode

import spotipy
from spotipy.oauth2 import SpotifyOAuth

load_dotenv()

CLIENT_ID = os.getenv('CLIENT_ID')
CLIENT_SECRET = os.getenv('CLIENT_SECRET')
REDIRECT_URI= os.getenv('REDIRECT_URI')
SCOPE = 'user-read-currently-playing,user-read-playback-state,user-modify-playback-state'

sp = spotipy.Spotify(auth_manager=SpotifyOAuth(client_id=CLIENT_ID,
                                               client_secret=CLIENT_SECRET,
                                               redirect_uri=REDIRECT_URI,
                                               scope=SCOPE)) ## Spotify object

try:
    ser = serial.Serial('/dev/cu.usbmodem1301') ## Serial Object for serial communication
    time.sleep(2)
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")

def start_album(album_code: str) -> None:
    ##
    # @brief Plays the given album code on Spotify
    # @param album_code The Spotify album code as a string to play on Spotify
    #
    # Plays the given album code on Spotify
    # Acknowledgements: The Spotipy Library Documentation
    # @see read_from_serial()
    try:
        sp.start_playback(context_uri=album_code)
    except Exception as e:
        print(f"Error starting album playback: {e}")


def start_song(song_code: str) -> None:
    ##
    # @brief Plays the given song/track code on Spotify
    # @param song_code The Spotify song code as a string to play on Spotify
    #
    # Plays the given song on Spotify
    # Acknowledgements: The Spotipy Library Documentation
    # @see read_from_serial()
    try:
        sp.start_playback(uris=[song_code])
    except Exception as e:
        print(f"Error starting song playback: {e}")

def get_all_artists(song_info) -> str:
    ##
    # @brief Returns a string of all artists name for the given song_info
    # @param song_info The JSON object with the song information to retrieve the artists from
    #
    # Returns the name of all artists from the given song_info JSON object as a comma seperated string
    # @see get_currently_playing_song()
    song_info_artists_len = len(song_info["item"]["artists"])
    artists = []
    for i in range(0, song_info_artists_len):
        artists.append(song_info["item"]["artists"][i]["name"])
    artists_str = ", ".join(artists)
    return artists_str

def get_currently_playing_song(prev_song_str) -> str:
    ##
    # @brief Returns a string with the song title and artist(s) name of the current song playing on Spotify.
    #.       If the song currently playing on Spotify has changed, then send the new song information to Serial as a JSON object
    # @param prev_song_str The most recent song info that was sent to Serial
    #
    # If the current song playing on Spotify has changed since the most recent song info sent to Serial,
    # then we get the information about the current song playing, and send the new song's title and artist
    # to Serial. Return a string with the song title and artist(s) name of the current song.
    # Acknowledgements: The Spotipy Library Documentation
    #
    # @see main(), get_all_artists(), send_song_info()
    try:
        song_info = sp.current_user_playing_track()
        song_title = song_info["item"]["name"] # song title
        song_artists = get_all_artists(song_info) # all artist names as comma seperated string
        result_str = "Song: " + song_title + " By: " + song_artists
        if result_str != prev_song_str:
            send_song_info(song_title, song_artists)
        return result_str
    except Exception as e:
        print(f"Error getting current song info: {e}")

def send_song_info(current_song_title: str, current_song_artists: str) -> None:
    ##
    # @brief Sends the given new song information to Serial as a JSON object
    # @param current_song_title String of the current song title to send in the JSON object
    # @param current_song_artists String of the current song's artist(s) to send in the JSON object
    #
    # Add the given current_song_title and current_song_artists to a JSON object, and send the JSON object to Serial.
    # Acknowledgements: Inspired by: https://stackoverflow.com/questions/55698070/sending-json-over-serial-in-python-to-arduino
    #
    # @see get_currently_playing_song()
    data = {}
    data["current_song"] = unidecode.unidecode(current_song_title)
    data["current_artists"] = unidecode.unidecode(current_song_artists)
    data=json.dumps(data)
    if ser.isOpen():
        ser.write(data.encode('utf-8'))
        ser.flush()
    else:
        print("Error opening serial port")

def read_from_serial() -> None:
    ##
    # @brief Plays the given song/track code on Spotify
    #
    # Reads the value from Serial, and plays the given album/song.
    # Acknowledgements: https://stackoverflow.com/questions/55698070/sending-json-over-serial-in-python-to-arduino
    #
    # @see main(), start_song(), start_album()
    try:
        time.sleep(3)
        ser.timout = 3 # Read data after 3 sec
        if ser.inWaiting() > 0:
            ReceivedString = ser.readline().rstrip().decode() # Get rid of \r\n, and convert from bytes to string
            myJson = json.loads(ReceivedString) # Convert to JSON object
            if "request_song" in myJson:
                start_song(myJson["request_song"])
            elif "request_album" in myJson:
                start_album(myJson["request_album"])
    except Exception as e:
        print(f"Error reading JSON from Serial: {e}")

def main():
    ## Main program entry.
    # @brief Main method to continously check song/album values being sent and played
    #
    # Continuously checks if a card value has been sent, or if the song has changed,
    # communicating with the Serial
    prev_song_str = ""
    while True:
        try:
            read_from_serial()
            time.sleep(0.2)
            tempReturn = get_currently_playing_song(prev_song_str)
            prev_song_str = tempReturn
        except Exception as e:
            print(f"Error in Main: {e}")
    ser.close()

if __name__ == '__main__':
    main()