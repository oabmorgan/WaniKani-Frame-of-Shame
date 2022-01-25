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

//show lessons too?
bool showLessons = true;

//How frequently to update, in minutes
unsigned long timerDelay = 15;

//What colors to use for the color gradient. https://uigradients.com/#Timber can give you some quick options.
#define colorA CRGB(0xff1493)
#define colorB CRGB(0x00bfff)
#define colorLesson CRGB(0xffffff)

//Max power to draw, controls the brightness of the LEDs. If you're powering this with USB, keep this under 450.
const uint8_t maxPower = 400;

/*  End of Settings */


unsigned long scaledTime;
#define LED_PIN  4

#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

//hostnames
const char* serverName = "https://api.wanikani.com/v2/assignments?";
const char* timeHost      = "http://api.timezonedb.com/v2/get-time-zone?key=******&format=xml&fields=formatted&by=zone&zone=Asia/Tokyo";

// Params for width and height
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

int pos = 0;

void setup() {
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

void setLED(int SRS) {
  if (pos < NUM_LEDS) {
    if (SRS == 0) {
      leds[pos] = colorLesson;
      FastLED.show();
      delay(10);
    }
    else if (SRS < 10) {
      leds[pos] = CRGB::White;
      FastLED.show();
      delay(10);
      leds[pos] = blend(colorA, colorB, 30 * SRS);
      delay(10);
    } else {
      leds[pos].fadeToBlackBy(192);
      delay(10);
    }
    pos ++;
    FastLED.show();
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Update Running");
  pos = 0;
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    Serial.println("Connected");
    Serial.println("Updating time");
    int currentHour = get_time();
    Serial.println("Hour: " + (String)currentHour);
    switch (currentHour) {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        Serial.println("Low Power Mode");
        FastLED.setMaxPowerInVoltsAndMilliamps(5, maxPower * 0.5); //max 500 for usb
        scaledTime = timerDelay * 6;
        break;
      case 23:
      case 24:
      case 7:
      case 8:
      case 9:
        Serial.println("Med Power Mode");
        FastLED.setMaxPowerInVoltsAndMilliamps(5, maxPower * 0.75); //max 500 for usb
        scaledTime = timerDelay * 3;
        break;
        break;
      default:
        Serial.println("High Power Mode");
        FastLED.setMaxPowerInVoltsAndMilliamps(5, maxPower); //max 500 for usb
        scaledTime = timerDelay;
        break;
    }

    if (showLessons) {
      Serial.println("Lessons");
      HTTPClient http;
      http.useHTTP10(true);
      String urlString;
      urlString = serverName + (String)"immediately_available_for_lessons=true";
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
      Serial.println("Lessons: " + (String)total_count);

      for (int i = 0; i < doc["total_count"]; i++) {
        setLED(0);
      }
    }

    for (int srs = 0; srs < 9; srs++) {
      Serial.println("SRS: " + (String)srs);
      HTTPClient http;
      http.useHTTP10(true);
      String urlString;
      urlString = serverName + (String)"immediately_available_for_review=true&srs_stages=" + (String)srs;
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
  Serial.println("You have " + (String)pos + " reviews + lessons");
  while (pos < NUM_LEDS) {
    setLED(10);
  }

  Serial.println("Going to sleep. See you in " + (String)scaledTime + " minutes." );
  digitalWrite(LED_BUILTIN, HIGH);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(scaledTime * 60000);
  wifi_set_sleep_type(NONE_SLEEP_T);
}
