// Include Library
#include <SPI.h>                                        // SPI Library
#include <LoRa.h>                                       // LoRa Library  
#include <TinyGPS++.h>                                  // GPS Library 
#include <SoftwareSerial.h>

// Declare LoRa Pins
#define ss 5                                            // LoRa radio chip select
#define rst 14                                          // LoRa radio reset
#define dio0 2                                          // LoRa hardware interrupt pin

// Declare Push Button pin and LED pin
#define pushButton 22                                   // Set push button pin 
const int ledGreen = 25;                                // Green LED for Sending indicator
const int ledRed = 27;                                  // Red LED for LoRa indicator
const int ledYellow = 26;                               // Yellow LED for GPS indicator
// setting PWM properties
const int freq = 5000;
const int ledChannelG = 0;
const int ledChannelR = 2;
const int ledChannelY = 1;
const int resolution = 8;

// Static RX TX GPS PIN and GPS baudrate
static const int RXPin = 16, TXPin = 17;                // RX TX GPS pin
static const uint32_t GPSBaud = 9600;                   // GPS baudrate

// Call GPS functions
TinyGPSPlus gps;                                        // GPS Library function call
SoftwareSerial gpsSerial(RXPin, TXPin);                 // Serial connection for RX TX GPS dev

// Data Variabel
int state;                                              // integer state for first value push button
float Latitude;                                         // Latitude data
float Longitude;                                        // Longitude data
float Altitude;                                         // Altitude data
long lastSendTime = 0;
int interval = 0;
byte localAddress = 0xBB;                               // Address of this device
byte destination = 0xFF;                                // Destination to send to
byte msgCount = 0;                                      // first count of outgoing messages
byte sender;
int recipient;                                          // Received

void LoRaSet(){
  // LoRa Sender Setup
  Serial.println("LoRa Sender");                        // Mention type of device in Serial Monitor
  LoRa.setPins(ss, rst, dio0);                          // Set pin ss, rst and dio0 LoRa
  if (!LoRa.begin(433E6)) {                             // Set freq LoRa to 433hz for Asian Region
    Serial.println("Starting LoRa failed!");            // Message if LoRa did not running 
    while (1);                                          // Keep run for once
    ledcWrite(ledChannelR, 0);                          // RED LED turn off if LoRa did not running 
  }
  Serial.println("LoRa Initializing OK!");              // Message if LoRa work running
  ledcWrite(ledChannelR, 100);                           // RED LED turn on if LoRa running
}

void LoRaSend(){
  // Send LoRa Message
  Serial.println("______________________________________________"); // Spacing
  Serial.println("Sending packet! ");                               // Message device sending packet
  Serial.println("Send Packet to: 0x" + String(destination,HEX)); 
  Serial.println("Message ID: " + String(msgCount));                // Message device sending packet                                    
  // Sending Packet
  LoRa.beginPacket();                                               // LoRa begin sending packet
  LoRa.write(destination);                                          // LoRa write message destination
  LoRa.write(localAddress);                                         // LoRa write device address
  LoRa.write(msgCount);                                             // LoRa write message count
  LoRa.write((uint8_t *)(&Latitude), sizeof(Latitude));             // LoRa write Latitude into uint8_t from float
  LoRa.write((uint8_t *)(&Longitude), sizeof(Longitude));           // LoRa write Longitude into uint8_t from float
  LoRa.write((uint8_t *)(&Altitude), sizeof(Altitude));             // LoRa write Altitude into uint8_t from float
  msgCount++;                                                       // Message count increment
  int result = LoRa.endPacket();                                    // LoRa end sending packet
  if(result){                                                       // if result is endPacket function then serial print
    Serial.println("Packet transmission success");                  // Print message in serial monitor
  }else{
    Serial.println("Packet transmission failed");                   // Print message in serial monitor
  }
}

void LoRaReceive(){
  recipient = LoRa.read();                                // LoRa Read Recipient data
  sender = LoRa.read();                                   // LoRa Read Sender data
}

void setup() {
  //Initializing IO
  Serial.begin(115200);                                 // initialize serial
  gpsSerial.begin(GPSBaud);                             // initialize GPS serial
  pinMode(pushButton, INPUT);                           // PinMode push button input

  // configure LED PWM functionalitites
  ledcSetup(ledChannelG, freq, resolution);
  ledcSetup(ledChannelY, freq, resolution);
  ledcSetup(ledChannelR, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledGreen, ledChannelG);
  ledcAttachPin(ledYellow, ledChannelY);
  ledcAttachPin(ledRed, ledChannelR);
  LoRaSet();
}

void loop() {
  // GPS Code
  while (gpsSerial.available() > 0){                    // While GPS serial > 0
    gps.encode(gpsSerial.read());                       // Read GPS data
    if (gps.location.isUpdated()){                      // GPS loc is updated
      ledcWrite(ledChannelY, 100);                    // Yellow LED turn on if GPS module running
    }
  }
  int packetSize = LoRa.parsePacket();                    // PacketSize is parse of the Packet
  state = digitalRead(pushButton);                      // State as digital read from pushButton   
  // Send message packet when button is pressed
  if (millis() - lastSendTime > interval){
      if(state == 0){                                       // If state then run code below it
          ledcWrite(ledChannelG, 100);                       // Green LED turn on if GPS module running
          // Make variabels into float
          Latitude = gps.location.lat();                      // Lattitude data from GPS module
          Longitude = gps.location.lng();                     // Longitude data from GPS module
          Altitude = gps.altitude.meters();                   // Altitude data from GPS module
          LoRaSend();

          lastSendTime = millis();
          interval = 7000;
      }
      if (state == 1){                                       // If state = 1 then turn off Green LED
          ledcWrite(ledChannelG, 0);                        // Green LED OFF
      }
  }
  if (packetSize){
        LoRaReceive();
        if (recipient == localAddress){
          // Receiving Message Packet
          Serial.println("_____________________________________________"); // Spacing
          Serial.println("Receive Feedback from Receiver!");                     // Print Message received in Serial Monitor
          Serial.println("Receive Feedback from: 0x" + String(sender, HEX)); // Print Sender data in Serial Monitor
          ledcWrite(ledChannelR, 0);
          delay(2000);
          ledcWrite(ledChannelR, 100);
        }
  }

}