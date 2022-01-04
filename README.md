# WaniKani Frame of Shame
 Stop ignoring those reviews you've got stacking up! The WaniKani Frame of Shame grabs all your waiting reviews and displays them so you can't ignore them anymore. It works by lighting up an LED for each review thats waiting, colored by SRS level. When you complete the review, the light turns off again. Keep reviewing to keep the lights off!
 
![front](https://user-images.githubusercontent.com/38126042/147902599-b20d6197-573f-4e7b-bed8-b5094bee06c0.jpg)

Easy to make, no soldering required!
**Estimated cost: ~$20**

## Materials
 - 16x16 WS2812B LED Matrix 
   - https://www.aliexpress.com/item/4000544584524.html
 - V2 Nodemcu ESP8266
   - https://www.aliexpress.com/item/32665100123.html
 - 3x Male to Female Dupont Jumper Cables
   - https://www.aliexpress.com/item/4000848184096.html
 - Frame that fits at least 160 x 160mm
 - Micro USB Cable


## Instructions
* Make a hole in the backing board of the frame for the input wires to go through. You can remove the output port and the extra voltage wires.

![wiring guide](https://user-images.githubusercontent.com/38126042/147903458-fe6f4511-f9a7-40a1-a9a4-806df57007de.png)
* Fit the LED matrix into the frame. I used double sided tape make it lie flat on backing board. 
* Download and open [WaniKani.ino](https://github.com/oabmorgan/WaniKani-Frame-of-Shame/blob/main/WaniKani/WaniKani.ino) (you need the [Arduino IDE installed and set up for the ESP8266 (2.7.4)](https://arduino-esp8266.readthedocs.io/en/latest/installing.html) if it's not already) and make the following changes.
  * ` const char* ssid = "SSID"; ` Fill in your WiFi SSID
  * ` const char* password = "PASSWORD"; ` Fill in your WiFi Password
  * `const char* apiKey = "WK-API-KEY";` Fill in your [WaniKani API Key](https://www.wanikani.com/settings/personal_access_tokens)
  * `const char* timezone = "TIMEZONECODE";` Fill in your [timezone code](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv)
* You also need to make sure you have the libraries listed at the top installed. [How?](https://www.arduino.cc/en/guide/libraries#toc3)
* Upload the sketch and then unplug the usb cable.
* Use Dupont jumper cables to attach the LED matrix to the board
  * ` RED -> VIN`
  * ` GREEN -> D4`
  * ` WHITE -> GND`
* Reattach the usb cable and you're done!

* Optionally, i've included a [case to 3D Print](https://github.com/oabmorgan/WaniKani-Frame-of-Shame/tree/main/STLs).
![back](https://user-images.githubusercontent.com/38126042/147902615-890019ac-f4f1-4f56-8e00-347b9b43f138.jpg)

## Required Libraries
- ArduinoJson
- FastLED
