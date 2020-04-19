float pulsesX, XA_SIG=0, XB_SIG=1, pulsesY, YA_SIG=0, YB_SIG=1, pulsesZ, ZA_SIG=0, ZB_SIG=1;

void setup(){
pinMode(LED_BUILTIN, OUTPUT);
attachInterrupt(digitalPinToInterrupt(3), XA_RISE, RISING); // Pin 3
attachInterrupt(digitalPinToInterrupt(2), XB_RISE, RISING); // Pin 2
attachInterrupt(digitalPinToInterrupt(18), YA_RISE, RISING); // Pin 18
attachInterrupt(digitalPinToInterrupt(19), YB_RISE, RISING); // Pin 19
attachInterrupt(digitalPinToInterrupt(20), ZA_RISE, RISING); // Pin 20
attachInterrupt(digitalPinToInterrupt(21), ZB_RISE, RISING); // Pin 21
Serial.begin(115200);
delay(100);
Serial.println("Connected");
}//setup

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

void loop(){
Serial.print("x");
Serial.print(pulsesX);
Serial.print("y");
Serial.print(pulsesY);
Serial.print("z");
Serial.print(pulsesZ);
Serial.println("end");
delay(20);
}
