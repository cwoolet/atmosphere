#atmosphere
######Automated, small scale, plant watering for forgetful people.  
##Overview
This project uses the Weather Underground's free developer [api](https://www.wunderground.com/weather/api/) to pull in 
real time weather data and make informed and automated decisions of watering of your outdoor plants.  

##Schematic
Here is the overview of the system done in [Digikeys's schemeit](http://www.digikey.com/schemeit/)

![Image of Schematic](https://github.com/cwoolet/atmosphere/blob/master/images/schematic.png)

##Bill of Materials 
* THe SOC is [Adafruit's Huzzah]( https://www.adafruit.com/product/2471)
* R1 is a 10KΩ Pot
  * Sets our watering time
* R2 is a 15KΩ Resistor
  * Provides voltage division allowing a 1V range Huzzah is looking for
* R3 is a 220Ω Resistor 
  * Current limiting resistor to FET/BJT
* Q1 is the ubiquitous  TIP120 
* D1 is a Flyback diode 1N4007
* M1 is the [Water Pump](http://www.lightobject.com/36Lmin-Mini-DC-Brushless-Submersible-Water-oil-Pump-P573.aspx)
* BT1 is the 6V Battery
  * 4 C Cells in a battery holder in my case
* Water reservoir
* 5/8 tubing

##Operation
TODO
