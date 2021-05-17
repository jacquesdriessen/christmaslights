# IoT Christmas Lights
 ESP8266 NeoPixel Christmas Lights

### Before you begin
- A project I did a long time ago, and requires some electronics and programming skills, if you are just interested in having IoT x-mas lights, nowadays there are ready made ones that are much more user friendly and fancier but arguably you will miss out on the pleasure of creating something yourself. 

### Instructions

- You will need an ESP8266 (ESP-01 will do) and some NeoPixel WS2811/12 based lights (google is your friend, anything from x-mas strings to strips available these days)
- Make sure this line is updated to reflect the number of lights that you have (mine had 100) 
`#define pixelCount 100`
- You will need an old version of the NeoPixelBus Library (the ones that the Arduino IDE offers are too different / new, I made this back in 2015). Grab it here (https://github.com/Makuna/NeoPixelBus/tree/cad11b9dc7aa8bdb82f62e51976dcdb2c69b8501)
- Build the electronics, the lights need ground / 5V and quite a bit of amps (you might want a fuse just in case and make sure your 5V power supply can provide it), the data line needs to be connected to a free ESP port (see below), ESP needs 3.3V (e.g. you will want a 5V -> 3.3V buck converter I presume)
- Update the pin for the "data line", I used pin 2, but any other pin would work as well
`NeoPixelBus strip = NeoPixelBus(pixelCount, 2, NEO_GRB);`
- It will start up with an access point called "kerstboom", connect to it and configure your wifi parameters [kerstboom is x-mas tree in dutch]
- After that navigate to the webpage called "kerstboom" where you can select the effects.
- Just search for any `kerstboom` references in the code and replace by whatever it is called in your own language.
- Should be really straightforward to add and/or change effects but requires some C skills.

### Youtube Demo:

[![](https://i9.ytimg.com/vi/mjfRuWL5KFo/mq2.jpg?sqp=CNiUiYUG&rs=AOn4CLC5d_696nnrOut48mvnGvdiAY2BGA)](https://www.youtube.com/watch?v=mjfRuWL5KFo)


