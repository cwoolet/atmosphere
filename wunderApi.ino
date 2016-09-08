/****************************
The MIT License (MIT)

Copyright (c) 2015 by bbx10node@gmail.com

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
// The majortiy of the following 3 function's:
// connectWiFI()
// connectToWunderground()
// jsonConditionsParser()
// are modified and renamed from  bbx10's code @ https://gist.github.com/bbx10/149bba466b1e2cd887bf
// my modifications are abstraction to use function pointers and adding in some addional calls and assignments for the CONDITIONS_EEPROM_UNION


// Use your own API key by signing up for a free developer account.
// http://www.wunderground.com/weather/api/
#define WU_API_KEY "**********"
#define WIFI_TIME_OUT_SECONDS 60 
#define RSP_BUFFER_SIZE 4096*2
// Specify your favorite location one of these ways.
// City and State
//#define WU_LOCATION "CA/HOLLYWOOD"
// US ZIP code
#define WU_LOCATION "46545"
// Country and city
//#define WU_LOCATION "Australia/Sydney"
#define WUNDERGROUND "api.wunderground.com"

// HTTP requests, one for conditions and another for the days forecast

const char WUNDERGROUND_REQ_CONDITIONS[] =
  "GET /api/" WU_API_KEY "/conditions/q/" WU_LOCATION ".json HTTP/1.1\r\n"
  "User-Agent: ESP8266/0.1\r\n"
  "Accept: */*\r\n"
  "Host: " WUNDERGROUND "\r\n"
  "Connection: close\r\n"
  "\r\n";

const char WUNDERGROUND_REQ_FORECAST[] =
  "GET /api/" WU_API_KEY "/forecast/q/" WU_LOCATION ".json HTTP/1.1\r\n"
  "User-Agent: ESP8266/0.1\r\n"
  "Accept: */*\r\n"
  "Host: " WUNDERGROUND "\r\n"
  "Connection: close\r\n"
  "\r\n";

const char SSID[]     = "**********";
const char PASSWORD[] = "**********";

static char respBuf[RSP_BUFFER_SIZE] = {};

void connectWiFI()
{
  int timeOut = 0;
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    if( timeOut++ > WIFI_TIME_OUT_SECONDS*2){
      Serial.println("WiFi Connection Timed Out");
      return;
    }
  }

  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

}
void connectToWunderground( const char * url, void (*jsonParser)(char*) )
{
  // Open socket to WU server port 80
  Serial.print(F("Connecting to "));
  Serial.println(WUNDERGROUND);
  memset(respBuf,0,RSP_BUFFER_SIZE);
  // Use WiFiClient class to create TCP connections
  WiFiClient httpclient;
  const int httpPort = 80;
  if (!httpclient.connect(WUNDERGROUND, httpPort)) {
    Serial.println(F("connection failed"));
    return;
  }

  // This will send the http request to the server
  Serial.print(url);
  httpclient.print(url);
  httpclient.flush();

  // Collect http response headers and content from Weather Underground
  // HTTP headers are discarded.
  // The content is formatted in JSON and is left in respBuf.
  int respLen = 0;
  bool skip_headers = true;
  while (httpclient.connected() || httpclient.available()) {
    if (skip_headers) {
      String aLine = httpclient.readStringUntil('\n');
      //Serial.println(aLine);
      // Blank line denotes end of headers
      if (aLine.length() <= 1) {
        skip_headers = false;
      }
    }
    else {
      int bytesIn;
      bytesIn = httpclient.read((uint8_t *)&respBuf[respLen], sizeof(respBuf) - respLen);
      if ( bytesIn != 0) {
        //Serial.print(F("bytesIn ")); Serial.println(bytesIn);
      }
      if (bytesIn > 0) {
        respLen += bytesIn;
        if (respLen > sizeof(respBuf)) respLen = sizeof(respBuf);
      }
      else if (bytesIn < 0) {
        Serial.print(F("read error "));
        Serial.println(bytesIn);
      }
    }
    delay(1);
  }
  httpclient.stop();

  if (respLen >= sizeof(respBuf)) {
    Serial.print(F("respBuf overflow "));
    Serial.println(respLen);
    return;
  }
  // Terminate the C string
  respBuf[respLen++] = '\0';
  //Serial.print(F("respLen "));
  //Serial.println(respLen);

  jsonParser(respBuf);
}

void jsonConditionsParser(char *json)
{
  StaticJsonBuffer<3 * 1024> jsonBuffer;

  // Skip characters until first '{' found
  // Ignore chunked length, if present
  char *jsonstart = strchr(json, '{');
  //Serial.print(F("jsonstart ")); Serial.println(jsonstart);
  if (jsonstart == NULL) {
    Serial.println(F("JSON data missing"));
    updateInternalClock(0);
    return;
  }
  json = jsonstart;

  // Parse JSON
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
    updateInternalClock(0);
    return;
  }

  // Extract weather info from parsed JSON
  JsonObject& current = root["current_observation"];
  const float temp_f = current["temp_f"];
  conditions.currentTemp = (char)temp_f;
  Serial.print(temp_f, 1); Serial.print(F(" F, "));
  const float temp_c = current["temp_c"];
  Serial.print(temp_c, 1); Serial.print(F(" C, "));
  const char *humi = current[F("relative_humidity")];
  Serial.print(humi);   Serial.println(F(" RH"));
  const char *weather = current["weather"];
  Serial.println(weather);
  //const char *pressure_mb = current["pressure_mb"];
  //Serial.println(pressure_mb);
  const char *local_time = current["local_time_rfc822"];
  Serial.println(local_time);

  // Extract local timezone fields
  //const char *local_tz_short = current["local_tz_short"];
  //Serial.println(local_tz_short);
  //const char *local_tz_long = current["local_tz_long"];
  //Serial.println(local_tz_long);
  //const char *local_tz_offset = current["local_tz_offset"];
  //Serial.println(local_tz_offset);
  Serial.println(F("Percp within the hour: "));
  const float precip_1hr_metric = current["precip_1hr_metric"];
  Serial.println(precip_1hr_metric);
  Serial.println(F("Percp within today: "));
  const float precip_today_metric = current["precip_today_metric"];
  Serial.println(precip_today_metric);

  conditions.rainToday = precip_today_metric;
  conditions.rainHr =    precip_1hr_metric;

  updateInternalClock(local_time);
  return;
}

// The rest is new:
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

void handleWunderApi()
{
  connectToWunderground(WUNDERGROUND_REQ_CONDITIONS, &jsonConditionsParser);
  connectToWunderground(WUNDERGROUND_REQ_FORECAST,   &jsonForecastParser);
}

void jsonForecastParser(char * json )
{
  float in = 0.0;
  DynamicJsonBuffer jsonBufferF;
  // Skip characters until first '{' found
  // Ignore chunked length, if present
  char *jsonstart = strchr(json, '{');
  if (jsonstart == NULL) {
    Serial.println(F("JSON data missing"));
    return;
  }
  json = jsonstart;

  // Parse JSON
  JsonObject& root = jsonBufferF.parseObject(json);

  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
    return;
  }
  // Extract weather info from parsed JSON
  JsonArray& forecastday = root["forecast"]["simpleforecast"]["forecastday"];
  
  if ( !forecastday.success()) {
    Serial.println(F("Getting forecastday array failed"));
    return;
  }
  // Do the lookup for the forecast
  for ( char i = 0; i < forecastday.size(); i++) {
    // TODO instead of period check if the date matches
    if ( forecastday[i]["period"] == 1) {
      in = forecastday[i]["qpf_allday"]["in"];
      break;
    }
  }
  if ( in == 0 ) {
    return;
  }
  conditions.QPF = in;
  Serial.println("QPF for today is:");
  Serial.println( conditions.QPF );
  return;
}
