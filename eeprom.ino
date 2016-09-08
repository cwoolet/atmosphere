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
// alternating bits to see if the eeprom is initalized or not, EE default as 0x00s or 0xFF
#define EE_SIGNATURE_VALUE 0xA5

void initEEPROM()
{
  int i = 0;
  char temp = 0;
  EEPROM.begin(512);
  temp = EEPROM.read(0);
  if ( temp != EE_SIGNATURE_VALUE) {
    EEPROM.write(0, EE_SIGNATURE_VALUE);
    for (i = 1; i < 512; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.end();
    Serial.print(F("EEPROM IS INITALIZED"));
  } else {
    Serial.print(F("EEPROM IS READY"));
  }
}

// READ AND WRITE EEPROM MUST START AT ADDR 1, NOT ADDR 0, THAT CONTAINS THE SIGNATURE. DO NOT ADD MORE THAT 511 BYTES OF EEPROM VARS
void readEEPROM()
{
  int i = 0;
  char buffer[128] = {};

  EEPROM.begin(512);
  for (i = 0; i < sizeof(CONDITIONS_EEPROM_UNION); i++) {
    conditions.buf[i] = EEPROM.read(i+1);
  }
  EEPROM.end();

  sprintf(buffer, "\r\nR EEPROM: WATERED?%d, TEMP: %d, RAINx1000(QPF %d TODAY:%d 1HR:%d) Date( %d:%d of %d)",
          conditions.watered, conditions.currentTemp, (int)(1000 * conditions.QPF), (int)(1000 * conditions.rainToday), (int)(1000 * conditions.rainHr), conditions.hour, conditions.min, conditions.day);
  Serial.print(buffer);
}

void writeEEROM()
{
  int i = 0;
  char buffer[128] = {};
  
  EEPROM.begin(512);
  for (i = 0; i < sizeof(CONDITIONS_EEPROM_UNION); i++) {
    EEPROM.write(i+1, conditions.buf[i]);
  }
  EEPROM.end();

  sprintf(buffer, "\r\nW EEPROM: WATERED?%d, TEMP: %d, RAINx1000(QPF %d TODAY:%d 1HR:%d) Date( %d:%d of %d)",
          conditions.watered, conditions.currentTemp, (int)(1000 * conditions.QPF), (int)(1000 * conditions.rainToday), (int)(1000 * conditions.rainHr), conditions.hour, conditions.min, conditions.day);
  Serial.print(buffer);
}

