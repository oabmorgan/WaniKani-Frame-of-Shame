#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <time.h>

/*  Basic Settings  */

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* apiKey = "WK-API-KEY"; //https://www.wanikani.com/settings/personal_access_tokens
const char* timezone = "TIMEZONECODE"; //https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

//Example time zones
//const char* timezone = "JST-9";                            // Japan
//const char* timezone = "GMT0BST,M3.5.0/01,M10.5.0/02";     // UK
//const char* timezone = "MET-2METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe
//const char* timezone = "EST-2METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//const char* timezone = "EST5EDT,M3.2.0,M11.1.0";           // EST USA  
//const char* timezone = "CST6CDT,M3.2.0,M11.1.0";           // CST USA
//const char* timezone = "MST7MDT,M4.1.0,M10.5.0";           // MST USA
//const char* timezone = "NZST-12NZDT,M9.5.0,M4.1.0/3";      // Auckland
//const char* timezone = "EET-2EEST,M3.5.5/0,M10.5.5/0";     // Asia
//const char* timezone = "ACST-9:30ACDT,M10.1.0,M4.1.0/3":   // Australia

/*  Advanced Settings  */

//How frequently to update, in minutes
unsigned long timerDelay = 10;

//What colors to use for the color gradient. https://uigradients.com/#Timber can give you some quick options.
#define colorA CRGB(0xff1493)
#define colorB CRGB(0x00bfff)

//Max power to draw, controls the brightness of the LEDs. If you're powering this with USB, keep this under 450.
const uint8_t maxPower = 450;

/*  End of Settings */




unsigned long scaledTime;
#define LED_PIN  4

#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

//hostnames
const char* serverName = "https://api.wanikani.com/v2/assignments?immediately_available_for_review=true&";
const char* timeHost      = "http://api.timezonedb.com/v2/get-time-zone?key=******&format=xml&fields=formatted&by=zone&zone=Asia/Tokyo";

// Params for width and height
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

int pos = 0;

void setup() {
  delay(3000);
  pinMode(LED_BUILTIN, OUTPUT);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxPower); //max 500 for usb
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println(WiFi.localIP());
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", timezone, 1);
  pos = 0;
}

int get_time() {
  time_t now;
  time(&now);
  char time_output[30];
  strftime(time_output, 30, "%H", localtime(&now));
  return String(time_output).toInt();
}

/*
int getUserLevel() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.useHTTP10(true);
    String urlString;
    urlString = "https://api.wanikani.com/v2/user";
    http.begin(client, urlString);
    http.addHeader("Authorization", "Bearer " + (String)apiKey);
    http.GET();

    StaticJsonDocument<64> filter;
    filter["data"]["level"] = true;
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return 60;
    }
    return doc["data"]["level"];
  }
  return 60;
}
*/

void setLED(int SRS) {
  if (pos < NUM_LEDS) {
    if (SRS < 10) {
      leds[pos] = CRGB::White;
      FastLED.show();
      delay(10);
      leds[pos] = blend(colorA, colorB, 30 * SRS);
    } else {
      leds[pos].fadeToBlackBy(128);
      delay(50);
    }
    pos ++;
    FastLED.show();
    delay(50);
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Update Running");
  pos = 0;
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
    Serial.println("Updating time");
    int currentHour = get_time();
    Serial.println(currentHour);
    switch (currentHour) {
      case 23:
      case 24:
      case 1:
      case 2:
        Serial.println("Med Power Mode");
        FastLED.setMaxPowerInVoltsAndMilliamps(5, maxPower*0.66); //max 500 for usb
        scaledTime = timerDelay * 3;
        break;
      break;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        Serial.println("Low Power Mode");
        FastLED.setMaxPowerInVoltsAndMilliamps(5, maxPower*0.33); //max 500 for usb
        scaledTime = timerDelay * 6;
        break;
      default:
        Serial.println("High Power Mode");
        FastLED.setMaxPowerInVoltsAndMilliamps(5, maxPower); //max 500 for usb
        scaledTime = timerDelay;
        break;
    }

    for (int srs = 0; srs < 9; srs++) {
      Serial.println("SRS: " + (String)srs);
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;
      http.useHTTP10(true);
      String urlString;
      urlString = serverName + (String)"srs_stages=" + (String)srs;
      http.begin(client, urlString);
      http.addHeader("Authorization", "Bearer " + (String)apiKey);
      http.GET();

      StaticJsonDocument<64> filter;
      filter["total_count"] = true;

      DynamicJsonDocument doc(2048);

      DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }

      int total_count = doc["total_count"];
      Serial.println("Reviews: " + (String)total_count);

      for (int i = 0; i < doc["total_count"]; i++) {
        setLED(srs);
      }
    }
  }
  Serial.println("You have " + (String)pos + " reviews");
  while (pos < NUM_LEDS) {
    setLED(10);
  }
  digitalWrite(LED_BUILTIN, HIGH);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(timerDelay * 60000);
  wifi_set_sleep_type(NONE_SLEEP_T);
}
