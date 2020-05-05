# KinoWheelsXYZ
Kino Wheels Project with extra Axis as well as a LiveLink JSON based Plugin for Unreal Engine.

This is a derrivative of the original Keno Wheels project found here : https://www.kinowheels.com This adds a third axis to have Pan / Tilt / Roll or whatever you want on your other axis is up to you. And to have the ability to connect it via Ethernet.

The Sketch wiring diagram is a lot simpler, no resistors as the teensy is able to cope with the volatge going into the digital pns.

![Arduino Sketch](/images/KinoWheels_bb.png)

The Arduino Sketch is based on a Teensy 3.6 and is using 6 interrupt pins for the encoders.
It Expects a connection handshake, to tell it where to send the data back to. It Listens on port 8887 (Which can be changed in the sketch). It Returns the data on the port that the request was sent on (8888) If you want to change that do so in unreal.
JSON String Expected:
{"SendData": true,"ResetData": false}

The Data it (Arduino) spits out is a JSON String which is structured as follows:
{
"KinoWheels":
  {
  "Name":"KinoWheels",
  "Rot":[832.95,264,436.65]
  }
}
The Two KinoWheels thing is redundant, but I figure if someone wants to hook up 2+ you need to be able to disciminate, and it's also not possible to name the Root index with a name, so this is the best way I found to handle that situation.

In the array, the values depend on if you have an axis to be read, this could allow for further wheels to be supported by appending another axis to the array which would have to be decoded, but as it currently stands it supports 3. or 2 or 1. If you have less wheels you will just recieve 0.0 on them and so will Unreal. 

The Unreal Plugin, needs to be added to an Unreal Engine, I used 4.24 but it depends how pupular this is if I maintain this code, and if it makes sense, it's derived off an example that Epic Generously supplied and is decoding straight up JSON, so in theory if they release their JSON plugin this may become obsolete.

For those unfamiliar with unreal's plugin configuration, since there is no "Pre-Compiled" version I'm including at this point, you need to add at least one C++ Class which can be done from within the engine, Call it "MyClass" or "MyThrowawayClass" once that's done Grab MS Visual Studio Community or whatever works for you to compile Unreal Code. and Re-Compile the plugin, at that point you should have a "KinoWheelsLiveLink" Source in the Live LinkPanel, and you should get a dialog for the IP. I need to change that because the Arduino is waiting for a connection to tell it to send data, or to stop. And it will then send data back to the requestor. So the IP in the dialog should be the arduino, and it should then automatically send back data. This is going to get worked on, as well as sending the arduino, or "Zero" out the values, and to start and stop sending data. Which should be a toggle and a boolean state (Check Box) once done.

Current Status :
The System Works

Future Plans

Arduino Hardware:
1) (IP) to add an LCD Display to display the device IP, so that you know what it is as it's trying to use DHCP and will only fall back to hard coded IP once DHCP is unsuccessfull.
2) use the SD Card, to "Backup the data" locally in case there's a network outage.
3) include buttons for data start / stop, and reset.

Software:
1) add manual blueprint functions for data start data stop, and reset.
2) add start stop / reset in the LiveLink Properties
3) Add better IP Negotiation, so that you don'y enter the universal reciver ip, which is you, but the IP of the device, and it will then negotioate to it.

Note if you want to use POE (Power over ethernet) the adapter I used was this thing :
UCTRONICS IEEE 802.3af Micro USB Active PoE Splitter Power Over Ethernet 48V to 5V 2.4A for Tablets, Dropcam or Raspberry Pi (48V to 5V 2.4A) 
I'm not sponsored by amazon, but if you want an easy to use link here it is:
This one is cheaper and only support 10/100 :

https://www.amazon.com/gp/product/B01MDLUSE7/ref=ppx_yo_dt_b_asin_title_o06_s00?ie=UTF8&psc=1

This one is a little more but supports Gigabit 10/10/1000 :

https://www.amazon.com/UCTRONICS-PoE-Splitter-Gigabit-Raspberry/dp/B07CNKX14C/ref=psdc_1194444_t1_B01MDLUSE7
