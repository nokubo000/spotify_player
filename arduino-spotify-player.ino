/**
 * @file arduino-spotify-player.ino
 * This program waits for an RFID card to be scanned, then sends the scanned card's
 * Spotify code to the Serial to play the song/album on Spotify.
 * Also recieves information about the current song playing on Spotify and displays
 * it on an LCD screen.
 */

/*
PINOUT:
RC522 MODULE    MEGA
SDA             D9 (PWM)
SCK             D52
MOSI            D51
MISO            D50
IRQ             N/A
GND             GND
RST             D8 (PWM)
3.3V            3.3V (Power)
*/

#include <SPI.h>
#include <RFID.h>
#include <Arduino_FreeRTOS.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h> 
#include <queue.h>

const int SDA_DIO = 9; ///< constant for the pin number of the SDA (SS)
const int RST_DIO = 8; ///< constant for the pin number of the RST (reset)

const char* FOLKLORE_CARD = "911343659194"; ///< constant holding the "Folklore" RFID card value
const char* VAMPIRE_CARD = "123442493173"; ///< constant holding the "Vampire" RFID card value
const char* FEARLESS_CARD = "15524814259214"; ///< constant holding the "Fearless" RFID card value

const char* FOLKLORE_SPOTIFY_CODE = "spotify:album:2fenSS68JI1h4Fo296JfGr"; ///< constant holding the "Folklore" Spotify code
const char* VAMPIRE_SPOTIFY_CODE = "spotify:track:3k79jB4aGmMDUQzEwa46Rz"; ///< constant holding the "Vampire" Spotify code
const char* FEARLESS_SPOTIFY_CODE = "spotify:album:4hDok0OAJd57SGIT8xuWJH"; ///< constant holding the "Fearless" Spotify code

const char DEBUT[] = "spotify:album:7mzrIsaAjnXihW3InKjlC3"; ///< constant holding the Debut Taylor Swift Album Spotify code

StaticJsonDocument<200> card_info_doc;  ///< JSON Document to hold all card information
RFID RC522(SDA_DIO, RST_DIO); ///< Instance of the RFID library
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  ///< Instance of the LiquidCrystal library

// Declare a queue
QueueHandle_t textLengthQueue;  ///< Queue with the maximum length of the 2 strings on the LCD screen

/**
 * @brief Setup the LCD screen
 *
 * Sets up the LCD screen's pins, and sets the initial text on the screen
 * Acknowledgments: Inspired by https://www.instructables.com/Arduino-Interfacing-With-LCD-Without-Potentiometer/
 * @see setup()
 */
void setupLCD() {
  // Init LCD interface
  analogWrite(6,75);
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("Song: ");
  lcd.setCursor(0, 1);
  lcd.print("By: ");
  // setup the initial display length
  int maxDisplayLength = 6;
  xQueueSend(textLengthQueue, &maxDisplayLength, portMAX_DELAY);
}

/**
 * @brief Setup the RFID scanner
 *
 * Sets up the SPI interface and RFID reader
 * Acknowledgments: Inspired by https://www.instructables.com/Interfacing-RFID-RC522-With-Arduino-MEGA-a-Simple-/
 * @see setup()
 */
void setupRFID() {
  SPI.begin(); // Begin SPI interface
  RC522.init(); // Init RFID reader
}

/**
 * @brief Setup the RFID card info
 *
 * Sets up the JSON Document with the RFID card value and spotify code
 * @see setup()
 */
void setupCardInfo() {
  card_info_doc[FOLKLORE_CARD]["type"] = "album";
  card_info_doc[FOLKLORE_CARD]["spotify_code"] = FOLKLORE_SPOTIFY_CODE;

  card_info_doc[VAMPIRE_CARD]["type"] = "song";
  card_info_doc[VAMPIRE_CARD]["spotify_code"] = VAMPIRE_SPOTIFY_CODE;

  card_info_doc[FEARLESS_CARD]["type"] = "album";
  card_info_doc[FEARLESS_CARD]["spotify_code"] = FEARLESS_SPOTIFY_CODE;
}

/**
 * @brief Setup the Serial Communication, Tasks, LCD, and RFID scanner 
 *
 * Setup the Serial Communication to 9600, Tasks, LCD, and RFID scanner 
 * @see setupCardInfo(), setupLCD(), setupRFID()
 */
void setup() {
  Serial.begin(9600);
  setupCardInfo(); // setup card info json

  xTaskCreate(readRFID, "readRFID", 1000, NULL, 1, NULL);
  xTaskCreate(readSerial, "readSerial", 1000, NULL, 1, NULL);
  xTaskCreate(scrollText, "scrollText", 1000, NULL, 1, NULL);

  // create a queue capable of holding 10 int values.
  static const int QUEUE_SIZE = 10;
  textLengthQueue = xQueueCreate(QUEUE_SIZE, sizeof(int));

  setupLCD();
  setupRFID();

  // start scheduler
  vTaskStartScheduler();

}

// Doesn't do anything because FREE RTOS scheduler takes care of everything, but we still need it to avoid errors
void loop() { }

/*
 * @brief Scrolls through the text on the LCD
 * @param pvParameters - void pointer to the pvParameters
 *
 * Scrolls through the LCD text being displayed. Shifts to the left once every 600 ms
 * Acknowledgments: Inspired by https://docs.arduino.cc/learn/electronics/lcd-displays/
 */
void scrollText(void *pvParameters) {
  int maxDisplayLength;
  while (1) {
    if (xQueuePeek(textLengthQueue, &maxDisplayLength, portMAX_DELAY)==pdTRUE) {
      int positionCounter = 0;
      static unsigned long prevShifted = millis();
      if ((positionCounter < maxDisplayLength) && ((millis() - prevShifted) >= 600)) {
        lcd.scrollDisplayLeft();
        positionCounter++;
        prevShifted = millis();
      }
      if (positionCounter >= maxDisplayLength) {
        positionCounter = 0;
      }
    }

  }
}

/**
 * @brief Waits for the RFID scanner to scan a card
 *
 * Checks if a card has been detected.
 * If a card is detected, gets the serial number value and checks that the card is different from the previous scan
 * If the card is different, then find the associated Spotify code and send the code to Serial.
 * If the card is not recognized then nothing happens
 * Acknowledgments: RFID inspired by https://www.instructables.com/Interfacing-RFID-RC522-With-Arduino-MEGA-a-Simple-/
 *                  JSON inspired by: https://arduinojson.org/v6/api/json/serializejson/
 */
void readRFID(void *pvParameters) {
  while (1) {
    static String value = "";
    // Has a card been detected?
    if (RC522.isCard()) {
      // If so then get its serial number
      RC522.readCardSerial();
      String pastCardSerial = value;
      value = "";
      for(int i = 0; i < 5; i++) {
        value += RC522.serNum[i];
      }
      
      if (!pastCardSerial.equals(value)) { // if the card scanned is different from the prev scanned card
        if (card_info_doc[value]) {
            // sendSerial
            String send_str;
            StaticJsonDocument<512> json_doc;

          if (card_info_doc[value]["type"] == "song") { // song
            json_doc["request_song"] = card_info_doc[value]["spotify_code"];
          } else { // album
            json_doc["request_album"] = card_info_doc[value]["spotify_code"];
          }
          serializeJson(json_doc, send_str);
          Serial.println(send_str);
        } else { // if the value was not found in the valid card values, then reset to previous valid card value
          value = pastCardSerial;
        }

      } // end of "if" card scanned is different

    }
  }
}

/*
 * @brief Reads from the Serial, and updates the text on the LCD based on the JSON object received
 * @param pvParameters - void pointer to the pvParameters
 *
 * Waits for JSON object to be sent from the Serial. If the JSON object has a different song title value
 * than the current song, then update the song title on the LCD. If the JSON object has a different artist
 * name then update the artist name on the LCD.
 * Acknowledgements: JSON inspired by: https://arduinojson.org/v6/api/json/deserializejson/
 *                   LCD inspired by: https://www.instructables.com/Arduino-Interfacing-With-LCD-Without-Potentiometer/
 * 
 */
void readSerial(void *pvParameters) {
  const char* current_song;
  const char* current_artists;
  static String display_song_str = "Song: ";
  static String display_artists_str = "By: ";
  while (1) {
    StaticJsonDocument<200> sent_json_doc;
    String new_display_song_str = "Song: ";
    String new_display_artists_str = "By: ";
    String read_str = Serial.readString();
    if (read_str) {
      DeserializationError error = deserializeJson(sent_json_doc, read_str);
      current_song = sent_json_doc["current_song"];
      current_artists = sent_json_doc["current_artists"];
      new_display_song_str += current_song;
      new_display_artists_str += current_artists;

      if ((new_display_song_str != "Song: ") & (new_display_song_str != display_song_str)) {
        lcd.clear();
        display_song_str = new_display_song_str;
      }
      if ((new_display_artists_str != "By: ") & (new_display_artists_str != display_artists_str)) {
        lcd.clear();
        display_artists_str = new_display_artists_str;
      }

      lcd.setCursor(1, 0);
      lcd.print(display_song_str);
      lcd.setCursor(1, 1);
      lcd.print(display_artists_str);
      // remove from queue
      int temp;
      xQueueReceive(textLengthQueue, &temp, portMAX_DELAY);  
      // add to queue
      int maxDisplayLength = max(display_song_str.length(), display_artists_str.length());
      xQueueSend(textLengthQueue, &maxDisplayLength, portMAX_DELAY);
      
    }
    
  }

}