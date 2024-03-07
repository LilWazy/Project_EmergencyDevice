// Include Library
#include <SPI.h>                                         // SPI Library                                        
#include <LoRa.h>                                        // Lora Library
// Declare Component Pins
#define ss 5                                              // LoRa radio chip select
#define rst 14                                            // LoRa radio reset
#define dio0 2                                            // LoRa hardware interrupt pin

// Data Variabel
int recipient;                                            // Recipient device address 
byte sender;                                              // Sender device address
byte incomingMsgId;                                       // Message count data
float Latitude;                                           // Latitude data
float Longitude;                                          // Longitude data
float Altitude;                                           // Altitude data
byte msgCount = 0;                                        // first count of outgoing messages
byte localAddress = 0xCC;                               // Address of this device
byte destination = 0xFF;                                // Destination to send to
byte destination2 = 0xBB;

void LoRaSet(){
  //LoRa Setting
  Serial.println("LoRa Repeater");                        // Mention type of device in Serial Monitor
  LoRa.setPins(ss, rst, dio0);                            // Set pin ss, rst and dio0 LoRa
  if (!LoRa.begin(433E6)) {                               // Set freq LoRa to 433hz for Asian Region
    Serial.println("Starting LoRa failed!");              // Message if LoRa did not running 
    while (1);                                            // Keep run for once
  } 
  Serial.println("LoRa Initializing OK!");                // Message if LoRa work running
  delay(1000);
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

void LoRaSend(){
  // Sending Packet
  LoRa.beginPacket();                                               // LoRa begin sending packet
  LoRa.write(destination);                                            // LoRa write message destination
  LoRa.write(localAddress);                                               // LoRa write device address
  LoRa.write(incomingMsgId);                                        // LoRa write message count
  LoRa.write((uint8_t *)(&Latitude), sizeof(Latitude));             // LoRa write Latitude into uint8_t from float
  LoRa.write((uint8_t *)(&Longitude), sizeof(Longitude));           // LoRa write Longitude into uint8_t from float
  LoRa.write((uint8_t *)(&Altitude), sizeof(Altitude));             // LoRa write Altitude into uint8_t from float
  // Send LoRa Message
  Serial.println(""); // Spacing
  Serial.println("Sending packet! ");                               // Message device sending packet
  Serial.println("Send Packet to: 0x" + String(destination, HEX)); 
  Serial.println("Latitude: " + String(Latitude,6));                // Print Latitude data in Serial Monitor
  Serial.println("Longitude: " + String(Longitude,6));              // Print Longitude data in Serial Monitor
  Serial.println("Altitude: " + String(Altitude,2));                // Print Altitude data in Serial Monitor
  Serial.println("RSSI: " + String(LoRa.packetRssi()));             // Print RSSI data in Serial Monitor            
  int result = LoRa.endPacket();                                    // LoRa end sending packet
  if(result){                                                       // if result is endPacket function then serial print
    Serial.println("Packet transmission success");                  // Print message in serial monitor
  }else{
    Serial.println("Packet transmission failed");                   // Print message in serial monitor
  } 
}

void LoRaSend2(){
  // Send LoRa Message
  Serial.println("______________________________________________"); // Spacing
  Serial.println("Repeat Feedback!");                               // Message dev ice sending packet
  Serial.println("Repeat Feedback to: 0x" + String(destination2, HEX) + " from: 0x" + String(destination, HEX));                                  
  // Sending Packet
  LoRa.beginPacket();                                               // LoRa begin sending packet
  LoRa.write(destination2);                                         // LoRa write device address
  LoRa.write(localAddress);                                          // LoRa write message destination
  LoRa.endPacket();                                    // LoRa end sending packet
}

void setup() {
  //Initializing IO
  Serial.begin(115200);                                             // initialize serial
  LoRaSet();
}

void loop() {
  // Receiving Message Packet
  int packetSize = LoRa.parsePacket();                    // PacketSize is parse of the Packet
  if (packetSize){
      LoRaReceive();
      if (recipient == destination){
        LoRaSend();
      }                                        // If packetSize is true then do the program inside
      if (recipient == destination2){
        LoRaSend2();        
      }
  }  
}
