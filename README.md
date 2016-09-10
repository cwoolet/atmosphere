#atmosphere
######Automated, small scale, and low power plant watering system for anyone.   
##Overview
This project uses the Weather Underground's free developer [api](https://www.wunderground.com/weather/api/) and an ESP8266 to pull in 
real time weather data to make informed and automated decisions of watering outdoor plants. 

Check out the details below to learn more! **This is still a work in progess and not all details of operation are complete, but the code is fully functional.**

##Schematic
Here is the overview of the simple system done in [Digikeys's schemeit](http://www.digikey.com/schemeit/):

![Image of Schematic](https://github.com/cwoolet/atmosphere/blob/master/images/schematic.png)

##Bill of Materials 
* The SOC is [Adafruit's Huzzah]( https://www.adafruit.com/product/2471)
* R1 is a 10KΩ Pot
  * Manually adjusts the watering duration
* R2 is a 15KΩ Resistor
  * Provides voltage division allowing a 1V range Huzzah is looking for
* R3 is a 220Ω Resistor 
  * Current limiting resistor to FET/BJT
* Q1 is the ubiquitous  TIP120 
* D1 is a Flyback diode 1N4007
* M1 is the [Water Pump](http://www.lightobject.com/36Lmin-Mini-DC-Brushless-Submersible-Water-oil-Pump-P573.aspx)
* BT1 is the 6V Battery
  * 4 C Cells is the typical operation
* Water reservoir
* 5/8 tubing

##Operation
atmosphere works by connecting to avaliable wifi network and pulls in the current weather conditions and forecast. With this information, atmosphere can make a decision to water or not. Within the code, the user can define parameters for watering duration, time to water, rain accumulated threshold, and as well what percent chance of rain is a limit to watering. 

The Adafruit Huzzah comes preprogramed with NodeMCU but check out their [guide](https://learn.adafruit.com/adafruit-huzzah-esp8266-breakout/using-arduino-ide) to use the Arduino IDE. 
