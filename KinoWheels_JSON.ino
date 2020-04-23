#include <SPI.h>         // needed for Arduino versions later than 0018
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Encoder.h>

bool debug = 0;
bool debugStream = 1;

// Network Interface
byte mac[] = { 0x04, 0xE9, 0xE5, 0x05, 0x83, 0xA5 };   // Be sure this address is unique in your network
IPAddress ip(192, 168, 0, 105);
IPAddress remoteip(192,168,0,55);
unsigned int port = 8887;      // local port to listen on
unsigned int rPort = 8888; // Port to send data on

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Pin numbers to the pins connected to your encoder.
Encoder X_Encoder(1, 2);
Encoder Y_Encoder(4, 5);
Encoder Z_Encoder(7, 8);

// Initial Encoder Positons.
int cacheX = -999;
int cacheY = -999;
int cacheZ = -999;

void initEthernet(){
  Serial.println("Initializing Ethernet");
  Ethernet.init(10);
  delay (1000);
  Ethernet.begin(mac, ip);
  Serial.println("Ethernet ready");
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP()); 
  if (Ethernet.linkStatus()==0) { 
    // give the Ethernet shield a second to initialize:    
    delay (10000);
    initEthernet();
  }
}

void setup() {
  
  Serial.begin(9600);  
  
  // start the Ethernet and UDP:    
  initEthernet(); // Initialize Network Connection
  Udp.begin(port);  // Start UDP
  
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  Serial.print("Currently sending Data to : ");
  Serial.print(Udp.remoteIP());
  Serial.print(":");
  Serial.println(Udp.remotePort()); 
}

// Main Loop
void loop() {
  // Grab Data From Encoder
  int X_Pos, Y_Pos, Z_Pos;
  X_Pos = X_Encoder.read();
  Y_Pos = Y_Encoder.read();
  Z_Pos = Z_Encoder.read();

  // If Data has Changed
  if (X_Pos != cacheX || Y_Pos != cacheY || Z_Pos != cacheZ) {

    // Initialize JSON size of data
    const size_t capacity = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(2);
    StaticJsonDocument<capacity> doc;

    float mult = .15; // So that each rotation = 360 degrees
    // Create the Json Document
    doc["SubjectName"].set("KinoWheels");
    JsonArray values = doc.createNestedArray("Rotation");        
    values.add(X_Pos*mult);
    values.add(Y_Pos*mult);
    values.add(Z_Pos*mult);
    
    if (debug){
      // Log
      Serial.print(F("Sending to "));      
      Serial.print(remoteip);
      Serial.print(F(" on port "));      
      Serial.println(rPort);
      serializeJson(doc, Serial);
    }
    
    if (debugStream){
      Serial.print(X_Pos*mult);
      Serial.print(" ");  
      Serial.print(Y_Pos*mult);  
      Serial.print(" ");  
      Serial.println(Z_Pos*mult);  
    }
    
    Udp.beginPacket(remoteip, rPort);
    serializeJson(doc, Udp);    
    Udp.println();
    Udp.endPacket();     

    // Update position for next comparrison
    cacheX = X_Pos;
    cacheY = Y_Pos;
    cacheZ = Z_Pos;
  }
}
