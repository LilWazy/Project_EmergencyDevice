// Include Library
#include <WiFi.h>                                         // Wifi Library
#include <WiFiClientSecure.h>                             // WifiClient Library
#include <UniversalTelegramBot.h>                         // Telegram Bot Library
#include <ArduinoJson.h>                                  // ArduinoJSON Library
#include <SPI.h>                                          // SPI Library                                        
#include <LoRa.h>                                         // Lora Library
#include <Wire.h>                                         // Wire
#include <Adafruit_SSD1306.h>                             // Library OLED SSD1306
#include <Adafruit_GFX.h>                                 // Library OLED SSD1306

// Declare Component Pins
#define ss 5                                              // LoRa radio chip select
#define rst 14                                            // LoRa radio reset
#define dio0 2                                            // LoRa hardware interrupt pin
#define Buzzer 4                                          // LoRa hardware interrupt pin

// Declare Telegram API
#define BOTtoken "..."                                    // Telegram Bot Token
#define CHAT_ID "..."                                     // Telegram Group Chat ID

// Data Variabel
int recipient;                                            // Recipient device address 
float Latitude;                                           // Latitude data
float Longitude;                                          // Longitude data
float Altitude;                                           // Altitude data
byte destination = 0xBB;
byte localAddress = 0xFF;                                 // Address of this device
byte sender;                                              // Sender device address
byte incomingMsgId;                                       // Message count data
//
uint32_t lastReportTime = 0;                              
int interval = 500;

// Internet Connection
const char* ssid = "...";                              // Wifi SSID
const char* password = "...";                   // Wifi Pass

// Call function library
WiFiClientSecure client;                                  // Wifi Library function call
UniversalTelegramBot bot(BOTtoken, client);               // Telegram Bot API setting
Adafruit_SSD1306 OLED (128, 32, &Wire); //

void LoRaSet(){
  //LoRa Setting
  Serial.println("LoRa Receiver");                        // Mention type of device in Serial Monitor
  LoRa.setPins(ss, rst, dio0);                            // Set pin ss, rst and dio0 LoRa
  if (!LoRa.begin(433E6)) {                               // Set freq LoRa to 433hz for Asian Region
    Serial.println("Starting LoRa failed!");              // Message if LoRa did not running 
    while (1);                                            // Keep run for once
  } 
  Serial.println("LoRa Initializing OK!");                // Message if LoRa work running
  delay(1000);
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.begin(ssid, password);
}

void LoRaSend(){
  // Send LoRa Message
  Serial.println("______________________________________________"); // Spacing
  Serial.println("Send Feedback to Sender!");                               // Message dev ice sending packet
  Serial.println("Send Feedback to: 0x" + String(destination, HEX));                                  
  // Sending Packet
  LoRa.beginPacket();                                               // LoRa begin sending packet
  LoRa.write(destination);                                          // LoRa write message destination
  LoRa.write(localAddress);                                         // LoRa write device address
  LoRa.endPacket();                                    // LoRa end sending packet
}

void LoRaReceive(){
  recipient = LoRa.read();                                // LoRa Read Recipient data
  sender = LoRa.read();                                   // LoRa Read Sender data
  incomingMsgId = LoRa.read();                            // LoRa Read MessageID data
  // Define Variabel into float
  const uint8_t numFloatBytes {sizeof(Latitude)};         // Latitude Float
  const uint8_t numFloatBytes2 {sizeof(Longitude)};       // Longitude Float
  const uint8_t numFloatBytes3 {sizeof(Altitude)};        // Altitude Float
  // Make into attemp buffer
  char tempBuffer[numFloatBytes];                         // Buffer for Latitude data
  char tempBuffer2[numFloatBytes2];                       // Buffer for Longitude data
  char tempBuffer3[numFloatBytes3];                       // Buffer for Altitude data
  // LoRa Read Latitude
  for (uint8_t i = 0; i < numFloatBytes;  i++) {          // Setting data to Float
    tempBuffer[i] = LoRa.read();                          // LoRa read buffer Latitude
  }
  memcpy((void *)&Latitude, tempBuffer, numFloatBytes);   // Arrange Latitude data 
  // LoRa Read Longitude
  for (uint8_t i = 0; i < numFloatBytes2; i++) {          // Setting data to Float
    tempBuffer2[i] = LoRa.read();                         // LoRa read buffer Longitude
  }  
  memcpy((void *)&Longitude, tempBuffer2, numFloatBytes2); // Arrange Longitude data
  // LoRa Read Altitude
  for (uint8_t i = 0; i < numFloatBytes3; i++) {          // Setting data to Float
    tempBuffer3[i] = LoRa.read();                         // LoRa read buffer Altitude
  }
  memcpy((void *)&Altitude, tempBuffer3, numFloatBytes3); // Arrange Altitude data

  // Receiving Message Packet
  Serial.println("_____________________________________________"); // Spacing
  Serial.println("Received Packet!");                     // Print Message received in Serial Monitor
  Serial.println("Received from: 0x" + String(sender, HEX)); // Print Sender data in Serial Monitor
  Serial.println("Message ID: " + String(incomingMsgId)); // Print Message count data in Serial Monitor
}

void SendingTelegram(){
  // Sending data to Telegram
  String emergency = "Emergency Alert!!!\n";              // String emergency
  emergency += "Pesan darurat dari Emergency Device\n"; // Print message to Telegram
  emergency += "Codename : Cikuray 1\n\n";                 // Print device codename to Telegram
  emergency += "Lokasi : https://www.google.com/maps?q=";  // Turn Message to Telegram in Google maps link format
  emergency += String(Latitude, 6);                       // Send Latitude as String to Telegram and combined with message before
  emergency += ",";                                       // Spacing
  emergency += String(Longitude, 6);                      // Send Longitude as String to Telegram and combined with message before
  emergency += "&ll=";                          // Google Maps link
  emergency += String(Latitude, 6);                       // Send Latitude as String to Telegram and combined with message before
  emergency += ",";                                       // Spacing
  emergency += String(Longitude, 6);                      // Send Longitude as String to Telegram and combined with message before
  emergency += "\nPesan dikirim dari perangkat : 0x";     // Sender Device name
  emergency += String(sender, HEX);                       // Sender String with hex type data
  emergency += "\nPesan diterima oleh perangkat : 0x";    // Receiver Device name
  emergency += String(recipient, HEX);                    // Receiver String with hex type data
  emergency += "\nKode Pesan No : ";                      // Message count code 
  emergency += String(incomingMsgId);                     // Message count String 
  emergency += "\nLatitude : ";                           // Latitude Message
  emergency += String(Latitude, 6);                       // Latitude as String 
  emergency += "\nLongitude : ";                          // Longitude Message
  emergency += String(Longitude, 6);                      // Longitude as String
  emergency += "\nAltitude : ";                           // Altitude Message
  emergency += String(Altitude, 2);                       // Altitude as String
  emergency += "\nPesan diterima dengan RSSI : ";         // RSSI Message
  emergency += String(LoRa.packetRssi());                 // RSSI as String
  bot.sendMessage(CHAT_ID, emergency, "");                // Send string emergency to telegram with valid group chat id
  // Print all data in Serial Monitor
  Serial.println("Packet successfully send to Telegram!!");                     // Print Message received in Serial Monitor
}

void OLEDSet(){
  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  OLED.display();
  OLED.clearDisplay();
  OLED.setTextSize(2);
  OLED.setTextColor(SSD1306_WHITE);
  OLED.setCursor(0,0);
  OLED.println(F("EMERGENCY DEVICE!!! "));
  OLED.display();
}

void OLEDScreen(){
  if (millis() - lastReportTime > interval) {
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setCursor(0,0);
  OLED.print("Lat: ");
  OLED.print(Latitude,6);
  OLED.println("");
  OLED.print("Long: ");
  OLED.print(Longitude,6);
  OLED.println("");
  OLED.print("Alt: ");
  OLED.print(Altitude,2);
  OLED.println("");
  lastReportTime = millis();
  interval = 5000;
  }
  OLED.display();
}

void BuzzerM(){
  digitalWrite(Buzzer, HIGH);
  delay(2000);                                              // Buzzer on 
  digitalWrite(Buzzer, LOW);                                // Buzzer off
}
void setup() {
  //Initializing IO
  Serial.begin(115200);                                     // initialize serial
  //LoRa Setting
  LoRaSet();
  // delete old config
  WiFi.disconnect(true);
  delay(1000);
  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.begin(ssid, password);
  Serial.println("Wait for WiFi... ");
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);            // Telegram Client API
  pinMode(Buzzer, OUTPUT);                                // pinMode buzzer as output
  digitalWrite(Buzzer, LOW);                              // Buzzer off
  OLEDSet();
}

void loop() {
  int packetSize = LoRa.parsePacket();                    // PacketSize is parse of the Packet
    if (packetSize){                                     // If packetSize is true then do the program inside
      LoRaReceive();
      if (recipient == localAddress){
        BuzzerM();
        SendingTelegram(); 
        OLEDScreen();
        LoRaSend();
      }  
      digitalWrite(Buzzer, LOW);                              // Buzzer off
    }
}
