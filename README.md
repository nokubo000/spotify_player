<div align="center">
  <h2> Arduino Spotify Player </h2>
</div>

## 💡 Overview
The Arduino Spotify Player allows users to scan a RFID card which sends the card’s Spotify code to the Spotify API to play the song/album on Spotify. 
It also receives information about the current song playing on Spotify and displays it on an LCD screen. Key features include:

## ✨ Features

- **📱 Play Songs/Albums on Spotify:** Plays the given song or album on any device with the Spotify app open and logged into the associated API's account.
- **🔍 Display Current Song Information:** Displays the song title and artist(s) name on an 16x2 LCD screen for the current song playing on Spotify.
- **🎯 Reliability & Precise Timing:** Built for reliability and precise timing for user inputs and outputs.
- **🔧 Customization & Flexibility:** Tailor the Arduino Spotify Player to your music taste by customizing the RFID cards to your favorite songs and albums on Spotify.

## Tech Stack

- **Arduino IDE**: A text editor that connects to the Arduino hardware to upload programs and communicate with them.
- **Python**: A programming language that lets you integrate systems effectively to build applications such as web applications, software, etc.
- **Spotify for Developers Web API**: API that allows applications to interact with Spotify by retrieving content data, creating and managing playlists, or controlling playback.
- **Spotipy**: A lightweight Python library for the Spotify Web API, used to get full access to all of the music data provided by the Spotify platform.
- **FreeRTOS**: Arduino library that is a class of RTOS. Designed to be small enough to be run on a microcontroller.
- **KiCAD**: Open source PCB design / electronics CAD software. 
- **JSON**: (JavaScript Object Notation) A lightweight format for storing and transporting data.
- **Serial Communication**: Transmitting data sequentially, sending one single bit at a time over a single wire, cable, or optical medium.
- **RFID**: Arduino library for reading/writing RFID values.
- **LiquidCrystal**: Arduino library for writing to the LCD screen.

## 📖 Sources and external API's

- [Spotify for Developers Web API](https://developer.spotify.com/) to create tokens to use the Spotify API and read API documentation.
- [spotipy](https://spotipy.readthedocs.io/en/2.26.0/) to use the lightweight Python library for the Spotify Web API.
- [How to Use Spotify's API with Python | Write a Program to Display Artist, Tracks, and More](https://www.youtube.com/watch?v=WAmEZBEeNmg) Video example of how to use the Spotipy API.
- [Arduino FreeRTOS](https://docs.arduino.cc/libraries/freertos/) Arduino FreeRTOS library documentation.
- [Interfacing RFID-RC522 With Arduino MEGA a Simple Sketch](https://www.instructables.com/Interfacing-RFID-RC522-With-Arduino-MEGA-a-Simple-/) Arduino RFID library examples.
- [Liquid Crystal Displays (LCD) with Arduino](https://docs.arduino.cc/learn/electronics/lcd-displays/) Arduino LiquidCrystal library documentation and examples.
- [Arduino Interfacing With LCD Without Potentiometer](https://www.instructables.com/Arduino-Interfacing-With-LCD-Without-Potentiometer/) Example of wiring 16x2 LCD with Arduino without a potentiometer.
- [ArduinoJSON](https://arduinojson.org/v6/) ArduinoJSON library documentation and examples.
- [Sending JSON over serial in Python to Arduino](https://stackoverflow.com/questions/55698070/sending-json-over-serial-in-python-to-arduino) Example of how to communicate with Arduino and Python using Serial communication.

