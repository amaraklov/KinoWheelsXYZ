#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>
#include <Encoder.h>

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

byte mac[] = {0x04, 0xE9, 0xE5, 0x05, 0x83, 0xA5};
//IPAddress remoteIp(192, 168, 0, 55);  // <- EDIT!!!!
//unsigned short remotePort = 8888;
unsigned short localPort = 8887;
EthernetUDP udp;

// Encoder Setup
// Pin numbers to the pins connected to your encoder.
Encoder X_Encoder(1, 2);
Encoder Y_Encoder(4, 5);
Encoder Z_Encoder(7, 8);

// Initial Encoder Positons.
int cacheX = -999;
int cacheY = -999;
int cacheZ = -999;

bool dataStream;
bool dataReset;

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;

  // Initialize Ethernet libary
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Failed to initialize Ethernet library"));
    return;
  }

  Serial.print("Sending from : ");
  Serial.println(Ethernet.localIP());
  // Enable UDP
  udp.begin(localPort);

  
}

// Get JSON Data
void getUDPJsonData(){
  DynamicJsonDocument doc(200);
  int packetSize = udp.parsePacket();
  if(packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = udp.remoteIP();
    for (int i =0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(udp.remotePort());
    
    // read the packet into packetBufffer
    udp.read(packetBuffer,48); 
    Serial.print("Data Read : ");
    Serial.println(packetBuffer);           
    char json[] = "{\"SendData\": false,\"ResetData\": false}";    
    
    DeserializationError error = deserializeJson(doc, packetBuffer);
    
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    else {
      Serial.print(F("deserializeJson() succeeded: "));
      Serial.println(packetBuffer);
    }

  
    dataStream = bool (doc["SendData"]);
    dataReset = doc["ResetData"];
    if (dataStream){
      Serial.println("Told to Stream Data");
    }
    if (dataReset){
      Serial.println("Told to Reset Data");
    }
  }
}

//Reply With Sensor Data
void sendUDPJsonData(){
  StaticJsonDocument<100> doc;    
  // Grab Data From Encoder
  int X_Pos, Y_Pos, Z_Pos;
  X_Pos = X_Encoder.read();
  Y_Pos = Y_Encoder.read();
  Z_Pos = Z_Encoder.read();

  // If Streaming is desired
  if (dataStream) {
            
    // If Data has Changed
    if (X_Pos != cacheX || Y_Pos != cacheY || Z_Pos != cacheZ) {
      float mult = .15; // So that each rotation = 360 degrees
      
      // Create the Json Document
      doc["SubjectName"].set("KinoWheels");
      JsonArray values = doc.createNestedArray("Rotation");        
      values.add(X_Pos*mult);
      values.add(Y_Pos*mult);
      values.add(Z_Pos*mult);
  
      // Log
      Serial.print(F("Sending to "));
      Serial.print(udp.remoteIP());
      Serial.print(F(" on port "));
      Serial.println(udp.remotePort());
      serializeJson(doc, Serial);
    
      // Send UDP packet
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      serializeJson(doc, udp);
      udp.println();
      udp.endPacket();
    }
  }
  // Update position for next comparrison
  cacheX = X_Pos;
  cacheY = Y_Pos;
  cacheZ = Z_Pos;  
}

void loop() {
  getUDPJsonData();
  sendUDPJsonData();     
  // Wait (May not be needed)
  delay(10);
}
