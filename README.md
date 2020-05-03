# KinoWheelsXYZ
Kino Wheels Project with extra Axis

This is a derrivative of the original Keno Wheels project found here : https://www.kinowheels.com This adds a third axis to have Pan / Tilt / Roll or whatever you want on your other axis is up to you.


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
The System seems to have issues with the data I'm sending it, I've hard coded some random number values so that it sees what it sais it wants, yet it sends no data, the light in the interface doesn't turn Green. So at this point it's not functional. I hope to get some help from someone who can tell me what I'm doing wrong, and then I will start to post updates.
