/*
  KinoWheels Implementation with RobotDyne Mega 2560 with Eth Ethernet Shield an POE
  Initialize code for MEGA ETH, Ethernet module W5500
  

  
 */
#include <SPI.h>
#include<Ethernet.h>
#include <EthernetUdp.h>
#include <SD.h>
#include <KinoWheelsStruct.h>
#define SS     10    //W5500 CS
#define RST    7    //W5500 RST For mega RST 11
#define CS  4     //SD CS pin
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
#if defined(WIZ550io_WITH_MACADDRESS) // Use assigned MAC address of WIZ550io
;
#else
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
#endif


struct 
// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
EthernetUDP Udp;
IPAddress remote_ip(192,168,0,55);
File myFile;

float pulsesX, XA_SIG=0, XB_SIG=1, pulsesY, YA_SIG=0, YB_SIG=1, pulsesZ, ZA_SIG=0, ZB_SIG=1;
unsigned int localPort = 8888;  

boolean state=true;
void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(SS, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(SS, LOW);
  digitalWrite(CS, HIGH);

  attachInterrupt(digitalPinToInterrupt(3), XA_RISE, RISING); // Pin 3
  attachInterrupt(digitalPinToInterrupt(2), XB_RISE, RISING); // Pin 2
  attachInterrupt(digitalPinToInterrupt(18), YA_RISE, RISING); // Pin 18
  attachInterrupt(digitalPinToInterrupt(19), YB_RISE, RISING); // Pin 19
  attachInterrupt(digitalPinToInterrupt(20), ZA_RISE, RISING); // Pin 20
  attachInterrupt(digitalPinToInterrupt(21), ZB_RISE, RISING); // Pin 21
  
  delay(100);

  /* If you want to control Reset function of W5500 Ethernet controller */
  digitalWrite(RST,HIGH);

    // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("Start!");
  // this check is only needed on the Leonardo:
   while (!Serial) {
     ; // wait for serial port to connect. Needed for Leonardo only
   }

  // start the Ethernet connection:
  #if defined(WIZ550io_WITH_MACADDRESS) // Use assigned MAC address of WIZ550io
    if (Ethernet.begin() == 0) {
  #else
    if (Ethernet.begin(mac) == 0) {
  #endif  
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Udp.begin(localPort);
  Serial.println();
  digitalWrite(SS, HIGH);
  digitalWrite(CS,LOW);
  delay(10);
  if (!SD.begin(CS)) 
  {
    Serial.println("No SD card!");
    return;
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  SD.remove("test.txt");
  delay(50);

}

void blinkLed(){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);            
}

//X-Axis

void XA_RISE(){
detachInterrupt(digitalPinToInterrupt(2));
//delay(1);
XA_SIG=1;

if(XB_SIG==0)
pulsesX++;//moving forward
if(XB_SIG==1)
pulsesX--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(2), XA_FALL, FALLING);
}

void XA_FALL(){
detachInterrupt(digitalPinToInterrupt(2));
//delay(1);
XA_SIG=0;

if(XB_SIG==1)
pulsesX++;//moving forward
if(XB_SIG==0)
pulsesX--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(2), XA_RISE, RISING);
}

void XB_RISE(){
detachInterrupt(digitalPinToInterrupt(3));
//delay(1);
XB_SIG=1;

if(XA_SIG==1)
pulsesX++;//moving forward
if(XA_SIG==0)
pulsesX--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(3), XB_FALL, FALLING);
}

void XB_FALL(){
detachInterrupt(digitalPinToInterrupt(3));
//delay(1);
XB_SIG=0;

if(XA_SIG==0)
pulsesX++;//moving forward
if(XA_SIG==1)
pulsesX--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(3), XB_RISE, RISING);
}

//Y-Axis

void YA_RISE(){
detachInterrupt(digitalPinToInterrupt(18));
//delay(1);
YA_SIG=1;

if(YB_SIG==0)
pulsesY++;//moving forward
if(YB_SIG==1)
pulsesY--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(18), YA_FALL, FALLING);
}

void YA_FALL(){
detachInterrupt(digitalPinToInterrupt(18));
//delay(1);
YA_SIG=0;

if(YB_SIG==1)
pulsesY++;//moving forward
if(YB_SIG==0)
pulsesY--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(18), YA_RISE, RISING);
}

void YB_RISE(){
detachInterrupt(digitalPinToInterrupt(19));
//delay(1);
YB_SIG=1;

if(YA_SIG==1)
pulsesY++;//moving forward
if(YA_SIG==0)
pulsesY--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(19), YB_FALL, FALLING);
}

void YB_FALL(){
detachInterrupt(digitalPinToInterrupt(19));
//delay(1);
YB_SIG=0;

if(YA_SIG==0)
pulsesY++;//moving forward
if(YA_SIG==1)
pulsesY--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(19), YB_RISE, RISING);
}

//Z-Axis

void ZA_RISE(){
detachInterrupt(digitalPinToInterrupt(20));
//delay(1);
ZA_SIG=1;

if(ZB_SIG==0)
pulsesZ++;//moving forward
if(ZB_SIG==1)
pulsesZ--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(20), ZA_FALL, FALLING);
}

void ZA_FALL(){
detachInterrupt(digitalPinToInterrupt(20));
//delay(1);
ZA_SIG=0;

if(ZB_SIG==1)
pulsesZ++;//moving forward
if(ZB_SIG==0)
pulsesZ--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(20), ZA_RISE, RISING);
}

void ZB_RISE(){
detachInterrupt(digitalPinToInterrupt(21));
//delay(1);
ZB_SIG=1;

if(ZA_SIG==1)
pulsesZ++;//moving forward
if(ZA_SIG==0)
pulsesZ--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(21), ZB_FALL, FALLING);
}

void ZB_FALL(){
detachInterrupt(digitalPinToInterrupt(21));
//delay(1);
ZB_SIG=0;

if(ZA_SIG==0)
pulsesZ++;//moving forward
if(ZA_SIG==1)
pulsesZ--;//moving reverse
blinkLed();
attachInterrupt(digitalPinToInterrupt(21), ZB_RISE, RISING);
}

// Send UDP Messgae
void send_message(struct kinowheelsdata *h)
{ 
  int i;
 
  byte *ptr = (byte *) h;
  for (i = 0; i < sizeof(struct kinowheelsdata); i++) {    
    Udp.write(*ptr++);
  }
}

float mult = .01;
// Main Loop
void loop() 
{
  //Serial.print("x = ");
  Serial.print(pulsesX*mult);
  Serial.print(",");
  //Serial.print("y = ");
  Serial.print(pulsesY*mult);
  Serial.print(",");
  //Serial.print("z = ");
  Serial.println(pulsesZ*mult);
  //Serial.println(" ");
  //Serial.println("end");
  delay(20);
  
  KinoWheelsData KWD;
  KWD.X = pulsesX*mult;
  KWD.Y = pulsesY*mult;
  KWD.Z = pulsesZ*mult;
  
  Udp.beginPacket(remote_ip, localPort);  
  send_message(&KWD);
  Udp.endPacket();

    //Serial.print("IP address = ");
  //for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    //Serial.print(Ethernet.localIP()[thisByte], DEC);
    //Serial.print(".");
  
  //}
  //Serial.println();
  //Serial.println();  
}
