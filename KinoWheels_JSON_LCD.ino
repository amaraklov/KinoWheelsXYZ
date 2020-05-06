#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>
#include <Encoder.h>
#include <LiquidCrystal.h>

#define UDP_TX_PACKET_MAX_SIZE 64

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
bool debugLog = false;

// This device is using DHCP if you need to set a static IP, you need to do that here.
byte mac[] = {0x04, 0xE9, 0xE5, 0x05, 0x83, 0xA5};
IPAddress ip(192, 168, 0, 88);
IPAddress myDns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
unsigned short localPort = 8887;
EthernetUDP udp;

//LCD Setup
// select the pins used on the LCD panel
LiquidCrystal lcd(33, 34, 30, 29, 28, 27);
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5


// Encoder Setup
// Pin numbers to the pins connected to your encoder.
Encoder X_Encoder(1, 2);
Encoder Y_Encoder(4, 5);
Encoder Z_Encoder(7, 8);

// Initial Encoder Positons.
int cacheX = -999;
int cacheY = -999;
int cacheZ = -999;

// Setup Variables for exposing read json values
bool dataStream;
bool dataReset;

// LCD Display Update Rate counter
int counter = 0;

void setup() {
  // Initialize serial port
  if (debugLog) {
  Serial.begin(9600);
  //while (!Serial) continue;
  delay(1000);
  }

  // LCD Init
  lcd.begin(16,2);
  lcd.clear();            // start the library
  
  // start the Ethernet connection:
  if (debugLog) {
    Serial.println("Trying to get an IP address using DHCP");
  }
  if (Ethernet.begin(mac) == 0) {
    if (debugLog) {
      Serial.println("Failed to configure DHCP, falling back to static IP");
    }
    // initialize the Ethernet device not using DHCP:
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
  }

   // show the IP
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rec:");
  lcd.print(Ethernet.localIP());
  lcd.setCursor(0, 1);
  lcd.print("Prt:");
  lcd.print(localPort);
  lcd.print(" Waiting");

  if (debugLog) {
    Serial.print("Sending from : ");
    Serial.println(Ethernet.localIP());
  }
  // Enable UDP
  udp.begin(localPort);
}

// Show LCD is Sending Data
void SendDisplay(bool data){
  // show the IP
  lcd.clear();
  lcd.setCursor(0, 0);  
  lcd.print(udp.remoteIP());
  lcd.setCursor(0, 1);
  if (data){
    lcd.print("*Send* ");
  }
  else {
    lcd.print("       ");
  }
  lcd.print("Prt:");
  lcd.print(udp.remotePort());
  
}

void StopDisplay(bool data){
    // show the IP
  lcd.clear();
  lcd.setCursor(0, 0);    
  lcd.print(Ethernet.localIP());
  lcd.setCursor(0, 1);
  if (data){
    lcd.print("*Read* ");
  }
  else {
    lcd.print("       ");
  }
  lcd.print("Prt:");
  lcd.print(localPort);
  
}

// -------- Listen ---------- for JSON Data
void getUDPJsonData(){  
  int packetSize = udp.parsePacket();
  if(packetSize)
  {
    StaticJsonDocument<100> readDoc;    // Init Json Memory Heap 
    //Get UDP Data
    if (debugLog) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
    }
    IPAddress remote = udp.remoteIP();
    if (debugLog) {
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
    }
   
    // read the packet into packetBufffer
    udp.read(packetBuffer,48);
    if (debugLog) {
      Serial.print("Data Read : ");
      Serial.println(packetBuffer);
    }
    //char json[] = "{\"SendData\": false,\"ResetData\": false}"; // For sizing memory for the decoder.   
   
    DeserializationError error = deserializeJson(readDoc, packetBuffer);
   
    // Test if parsing succeeds.
    if (debugLog) {
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
      else {
        Serial.print(F("deserializeJson() succeeded: "));
        Serial.println(packetBuffer);
      }
    }

 
    dataStream = bool (readDoc["SendData"]);
    dataReset = readDoc["ResetData"];
    if (debugLog) {
      if (dataStream){
        Serial.println("Told to Stream Data");
      }
      if (dataReset){
        Serial.println("Told to Reset Data");
      }
    }
  }
}

// ------- Reply --------- With Sensor Data
void sendUDPJsonData(){
  int X_Pos, Y_Pos, Z_Pos;
  if (dataReset){
    // Reset Position
    X_Pos = 0;
    Y_Pos = 0;
    Z_Pos = 0;
  }
  else {
    // Grab Data From Encoder    
    X_Pos = X_Encoder.read();
    Y_Pos = Y_Encoder.read();
    Z_Pos = Z_Encoder.read();
  }
  // If Streaming is desired
  if (dataStream) {
    
    // If Data has Changed
    //if ((X_Pos != cacheX) || (Y_Pos != cacheY) || (Z_Pos != cacheZ)) {
      DynamicJsonDocument writeDoc(256); // Init Json Memory Heap 
      float mult = .15; // So that each rotation = 360 degrees
      
      // Create the Json Document      
      JsonObject KinoWheels = writeDoc.createNestedObject("KinoWheels");
      KinoWheels["Name"] = "KinoWheels";
      JsonArray values = KinoWheels.createNestedArray("Rot");      
      values.add(X_Pos*mult);
      values.add(Y_Pos*mult);
      values.add(Z_Pos*mult);      
      
      if (debugLog){
        // Log
        Serial.print(F("Sending to "));
        Serial.print(udp.remoteIP());
        Serial.print(F(" on port "));
        Serial.println(udp.remotePort());       
        serializeJson(writeDoc, Serial); // Debug String to see what was sent.
      }

      
      // Send UDP packet
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      serializeJson(writeDoc, udp);      
      udp.endPacket();           
    //}
   
  }
  // Update position for next comparrison
  cacheX = X_Pos;
  cacheY = Y_Pos;
  cacheZ = Z_Pos; 
 
}


void loop() { 
  getUDPJsonData();
  sendUDPJsonData(); 
  delay(10);
  if (dataStream){
    if (!(counter % 100)){
      SendDisplay(!(counter % 200));
    }
  }
  else {
    if (!(counter % 100)){
      StopDisplay(!(counter % 200));
    }
  }  
  counter = (counter%20000)+1;  
}
