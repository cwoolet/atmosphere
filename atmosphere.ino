/****************************
The MIT License (MIT)
Copyright (c) 2016 by cwoolet  
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************/
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <string.h>
#include <EEPROM.h>

// 20 minutes between update checks. The free developer account has a limit
// on the number of calls at 10 calls per min
#define PULL_RATE_MINS          20
#define DEEP_SLEEP_TIME    (PULL_RATE_MINS*60*1000000) // 1uS per tick 
 
#define PIN_PUMP 15
#define PIN_VAR_POT 13
#define PIN_ANALOG_IN 0 

#define RAIN_PRECENT_THRESHOLD            .5
#define RAIN_AMOUNT_TODAY_THRESHOLD       10.0
#define RAIN_ALERT_THRESHOLD              100.0
#define WATERING_HOUR                     20   // this is out of 0-23
#define WATERING_LENGTH                   5


union CONDITIONS_EEPROM_UNION
{
  char buf[5*sizeof(char) + 3*sizeof(float)];
  struct {
    char watered;     // 0: no , 1: yes
    char currentTemp;
    char day;         // 0-31
    char hour;        // 0-23
    char min;         // 0-59
    float rainToday;  // millimeters
    float rainHr;     // millimeters
    float QPF;        // https://en.wikipedia.org/wiki/Quantitative_precipitation_forecast
  };
};

union CONDITIONS_EEPROM_UNION conditions = {};


//In arduino-land this runs first
void setup()
{
  int V_A0 = 0;
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only // just be safe why not
  }
  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_VAR_POT, OUTPUT);
  pinMode(PIN_ANALOG_IN, INPUT);  

  initEEPROM();
  readEEPROM();

  // delay for some time to press the button
  delay(2000); 
  /** caution this work for a noramlly closed switch **/
  // this is the test function to get the timing right for the pump
  while(!digitalRead(PIN_ANALOG_IN)){
    Serial.println("Priming Pump");
    
    V_A0 = getAverageAnalogIn();
    Serial.println("Pump run time is:");
    Serial.println(V_A0*30);
    delay(1000);
  }
  if( V_A0 != 0){
    waterPlants();
  }
  /** end test **/
  
  digitalWrite(PIN_VAR_POT, LOW);
  digitalWrite(PIN_PUMP, LOW);
  connectWiFI();
}

// Loop as arduino speak for main
void loop()
{
  // "loops" because of ESP.deepSleep
  unsigned char day = conditions.day;
  handleWunderApi();
  // when a day has passed, reset the watered flag and forecast conditions
  if( day != conditions.day){
    conditions.watered   = 0;
    conditions.QPF       = 0;
    conditions.rainToday = 0;
    conditions.rainHr    = 0;
  }
  
  decideToWater();
  writeEEROM();
  
  //All actions are performed for this cycle..
  //Sleep and reboot us back to setup() in PULL_RATE_MINS many mins  
  ESP.deepSleep(DEEP_SLEEP_TIME);
}

/*** Algorthims ***/ 
void decideToWater( void )
{
  if( conditions.watered ){
    return;
  }
  
  if ( conditions.QPF < RAIN_PRECENT_THRESHOLD && conditions.rainToday < RAIN_AMOUNT_TODAY_THRESHOLD && conditions.hour >= WATERING_HOUR ) {
    Serial.println("Watering the plants!");
    waterPlants();
    conditions.watered = 1;
  }
  if ( conditions.rainToday > RAIN_ALERT_THRESHOLD ) {
    // TODO: blink the lights to alert draining may be needed 
    Serial.println("Wow tons of water today");
  }

}

void waterPlants( void )
{
  int V_A0 = getAverageAnalogIn();
  
  Serial.println("Pump On for");
  Serial.print(V_A0*30);
  digitalWrite(PIN_PUMP, HIGH);
  delay(30 * V_A0);
  
  digitalWrite(PIN_PUMP, LOW);
}



