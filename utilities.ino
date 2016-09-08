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

#define NUMBER_OF_SAMPLES 64
int getAverageAnalogIn( void )
{
  // the largest the A0 reads is 1024
  unsigned long analogAccumlator = 0;
  digitalWrite(PIN_VAR_POT, HIGH);
  delay(200);  //let it settle
  // accumulate an aggrigate 
  for (char i = 0; i < NUMBER_OF_SAMPLES; i++) {
    analogAccumlator += analogRead(A0);
  }
  digitalWrite(PIN_VAR_POT, LOW);
  // return the average 
  return (analogAccumlator / NUMBER_OF_SAMPLES);
}

void updateInternalClock(const char * time)
{
  char i = 0;
  int temp = 0;
  char buffer[30] = {}; // sufficently large buffer for scanf

  Serial.println(F("\r\nUpdate Internal Clock.. "));

  if (time == 0) {
    //no wifi pulled info, keep track of elapsed time internally
    Serial.println(F("\r\nNo time data received, estamate time "));
    conditions.min  += PULL_RATE_MINS;
    conditions.hour += conditions.min / 60;
    conditions.min  %= 60;
    //increment the day,used only to see if a day has elapsed so accuracy here is irreleavent
    conditions.day  += conditions.hour / 24;
    conditions.hour %= 24;
    // year and month are left out becasuse they do not matter for accurate daily watering
  } else {
    // "Sun, 21 Aug 2016 13:23:42 -0400"
    // big endian

    temp = (time[6] << 8) + time[5];
    conditions.day = atoi((const char *)&temp);

    temp = (time[18] << 8) + time[17];
    conditions.hour = atoi((const char *)&temp);

    temp = (time[21] << 8) + time[20];
    conditions.min = atoi((const char *)&temp);
  }

  sprintf(buffer, "Time is: %d:%d of %d\r\n", conditions.hour, conditions.min, conditions.day);
  Serial.print(buffer);
}
