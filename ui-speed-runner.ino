/*
  Project Name:   ui_speed_runner
  Description:    Quickly review UI screens in development on target device

  See README.md for target information and revision history
*/

// hardware and internet configuration parameters
#include "config.h"
// private credentials for network, weather provider, etc.
#include "secrets.h"

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(4, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Generalized network handling
#include "aq_network.h"
AQ_Network aq_network;

// Variables for accumulating averaged sensor readings
int sampleCounter;
float averageTempF;
float averageHumidity;
uint16_t averageCO2;

// environment characteristics
typedef struct
{
  float internalTempF;
  float internalHumidity;
  uint16_t internalCO2;
  int extTemperature;
  int extHumidity;
  int extAQI;
} envData;

// global for air characteristics
envData sensorData;

bool batteryAvailable = false;
bool internetAvailable = false;

// Battery voltage sensor
#include <Adafruit_LC709203F.h>
Adafruit_LC709203F lc;

// screen support
// Adafruit MagTag
#include <Adafruit_ThinkInk.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#define SRAM_CS     -1  // can set to -1 to not use a pin (uses a lot of RAM!)
//#define EPD_BUSY    5   // can set to -1 to not use a pin (will wait a fixed delay)
#define EPD_BUSY    -1   // can set to -1 to not use a pin (will wait a fixed delay)
ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
// colors are EPD_WHITE, EPD_BLACK, EPD_RED, EPD_GRAY, EPD_LIGHT, EPD_DARK

#include "ArduinoJson.h"  // Needed by getWeather()

void setup()
{
  #if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
    // turn on the I2C power by setting pin to opposite of 'rest state'
    // Rev B board is LOW to enable
    // Rev C board is HIGH to enable
    pinMode(PIN_I2C_POWER, INPUT);
    delay(1);
    bool polarity = digitalRead(PIN_I2C_POWER);
    pinMode(PIN_I2C_POWER, OUTPUT);
    digitalWrite(PIN_I2C_POWER, !polarity);
  #endif

  #ifdef DEBUG
    Serial.begin(115200);
    // wait for serial port connection
    while (!Serial);

    // Confirm key site configuration parameters
    debugMessage("UI Speed Runner started");
    debugMessage("Client ID: " + String(CLIENT_ID));
    debugMessage("Site lat/long: " + String(OWM_LAT_LONG));
  #endif

  neopixels.begin();
  neopixels.setBrightness(50);
  neopixels.show(); // Initialize all pixels to 'off'
  // Neopixel power
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, LOW); // on

  // there is no way to query screen for status
  display.begin(THINKINK_GRAYSCALE4);
  debugMessage("Display ready");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);

  initBattery();

  // Setup whatever network connection is specified in config.h
  internetAvailable = aq_network.networkBegin();

  // Implement a variety of internet services, if networking hardware is present and the
  // network is connected.  Services supported include:
  //
  //  NTP to get date and time information (via Network subsystem)
  //  Open Weather Map (OWM) to get local weather and AQI info
  //  MQTT to publish data to an MQTT broker on specified topics
  //  DWEET to publish data to the DWEET service

  // Get local weather and air quality info from Open Weather Map
  // if (internetAvailable)
  // {
  //   getWeather();
  // }
}

void loop()
{
  
  String dateTimeString = "May 15 at 12:03pm";

  // Set environment test values needed for screen rendering
  sensorData.internalTempF = 78.23;
  sensorData.internalHumidity = 30.12;
  sensorData.internalCO2 = 567;
  //sensorData.internalCO2 = 10000;
  averageTempF =81.2;
  averageHumidity =27.4;
  averageCO2=671;
  sensorData.extTemperature=54.23;
  sensorData.extHumidity=67.5;
  sensorData.extAQI=1;

  if (! digitalRead(BUTTON_A)) {
    debugMessage("Button A pressed");
    neopixels.clear();
    neopixels.show();
    neopixels.setPixelColor(3,255,0,0);
    neopixels.show();
    screenAlert("Sensor not available");
  }
  else if (! digitalRead(BUTTON_B)) {
    debugMessage("Button B pressed");
        neopixels.clear();
    neopixels.show();
    neopixels.setPixelColor(2,0,255,0);
    neopixels.show();
    //screenInfo("Updated [+DIM] " + aq_network.dateTimeString());
    screenInfo("Updated [+DIM] " + dateTimeString);    
  }
  else if (! digitalRead(BUTTON_C)) {
    debugMessage("Button C pressed");
        neopixels.clear();
    neopixels.show();
    neopixels.setPixelColor(1,0,0,255);
    neopixels.show();
    screenInfo2("Updated [+DIM] " + dateTimeString);
  }
  else if (! digitalRead(BUTTON_D)) {
    debugMessage("Button D pressed");
        neopixels.clear();
    neopixels.show();
    neopixels.setPixelColor(0,255,255,255);
    neopixels.show();
    screenAlert2("Sensor not available");
  }
}

void debugMessage(String messageText)
// wraps Serial.println as #define conditional
{
#ifdef DEBUG
  Serial.println(messageText);
  Serial.flush();  // Make sure the message gets output (before any sleeping...)
#endif
}

void getWeather()
// retrieves weather from Open Weather Map APIs and stores data to environment global
{
  #if defined(WIFI) || defined(RJ45)
    // if there is a network interface (so it will compile)
    if (internetAvailable)
    // and internet is verified
    {
      String jsonBuffer;
  
      // Get local temp and humidity
      String serverPath = String(OWM_SERVER) + OWM_WEATHER_PATH + OWM_LAT_LONG + "&units=imperial" + "&APPID=" + OWM_KEY;
  
      jsonBuffer = aq_network.httpGETRequest(serverPath.c_str());
      //debugMessage(jsonBuffer);
  
      StaticJsonDocument<1024> doc;
  
      DeserializationError httpError = deserializeJson(doc, jsonBuffer);
  
      if (httpError)
      {
        debugMessage("Unable to parse weather JSON object");
        debugMessage(String(httpError.c_str()));
        sensorData.extTemperature = 10000;
        sensorData.extHumidity = 10000;
      }
  
      //JsonObject weather_0 = doc["weather"][0];
      // int weather_0_id = weather_0["id"]; // 804
      // const char* weather_0_main = weather_0["main"]; // "Clouds"
      // const char* weather_0_description = weather_0["description"]; // "overcast clouds"
      // const char* weather_0_icon = weather_0["icon"]; // "04n"
  
      JsonObject main = doc["main"];
      sensorData.extTemperature = main["temp"];
      // float main_feels_like = main["feels_like"]; // 21.31
      // float main_temp_min = main["temp_min"]; // 18.64
      // float main_temp_max = main["temp_max"]; // 23.79
      // int main_pressure = main["pressure"]; // 1010
      sensorData.extHumidity = main["humidity"]; // 81
  
      // int visibility = doc["visibility"]; // 10000
  
      // JsonObject wind = doc["wind"];
      // float wind_speed = wind["speed"]; // 1.99
      // int wind_deg = wind["deg"]; // 150
      // float wind_gust = wind["gust"]; // 5.99
  
      // int clouds_all = doc["clouds"]["all"]; // 90
  
      // long sys_sunrise = sys["sunrise"]; // 1640620588
      // long sys_sunset = sys["sunset"]; // 1640651017
  
      // int timezone = doc["timezone"]; // -28800
      // long id = doc["id"]; // 5803139
      // const char* name = doc["name"]; // "Mercer Island"
      // int cod = doc["cod"]; // 200
  
      // Get local AQI
      serverPath = String(OWM_SERVER) + OWM_AQM_PATH + OWM_LAT_LONG + "&APPID=" + OWM_KEY;
  
      jsonBuffer = aq_network.httpGETRequest(serverPath.c_str());
      //debugMessage(jsonBuffer);
  
      StaticJsonDocument<384> doc1;
  
      httpError = deserializeJson(doc1, jsonBuffer);
  
      if (httpError)
      {
        debugMessage("Unable to parse air quality JSON object");
        debugMessage(String(httpError.c_str()));
        sensorData.extAQI = 10000;
      }
  
      // double coord_lon = doc1["coord"]["lon"]; // -122.2221
      // float coord_lat = doc1["coord"]["lat"]; // 47.5707
  
      JsonObject list_0 = doc1["list"][0];
  
      sensorData.extAQI = list_0["main"]["aqi"]; // 2
  
      // JsonObject list_0_components = list_0["components"];
      // float list_0_components_co = list_0_components["co"]; // 453.95
      // float list_0_components_no = list_0_components["no"]; // 0.47
      // float list_0_components_no2 = list_0_components["no2"]; // 52.09
      // float list_0_components_o3 = list_0_components["o3"]; // 17.17
      // float list_0_components_so2 = list_0_components["so2"]; // 7.51
      // float list_0_components_pm2_5 = list_0_components["pm2_5"]; // 8.04
      // float list_0_components_pm10 = list_0_components["pm10"]; // 9.96
      // float list_0_components_nh3 = list_0_components["nh3"]; // 0.86
    }
  #else
    sensorData.extTemperature = 10000;
    sensorData.extHumidity = 10000;
    sensorData.extAQI = 10000;
  #endif
  debugMessage(String("Open Weather Map returned: ") + sensorData.extTemperature + "F, " + sensorData.extHumidity + "%, " + sensorData.extAQI + " AQI");
}

void screenAlert(String messageText)
// Display critical error message on screen
{
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  display.setFont(&FreeSans9pt7b);
  display.setCursor(5,(display.height()/2));
  display.print(messageText);

  //update display
  display.display();
}

void screenAlert2(String messageText)
// Display critical error message on screen
{
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  display.setFont();  // resets to system default monospace font
  display.setCursor(5,(display.height()/2));
  display.print(messageText);

  //update display
  display.display();
}

void screenInfo(String messageText)
// Display environmental information on screen
{
  String aqiLabels[5]={"Good", "Fair", "Moderate", "Poor", "Very Poor"};
  
  display.clearBuffer();

  // borders
  // ThinkInk 2.9" epd is 296x128 pixels
  // label border
  display.drawLine(0,(display.height()/8),display.width(),(display.height()/8),EPD_GRAY);
  // temperature area
  display.drawLine(0,(display.height()*3/8),display.width(),(display.height()*3/8),EPD_GRAY);
  // humidity area
  display.drawLine(0,(display.height()*5/8),display.width(),(display.height()*5/8),EPD_GRAY);
  // C02 area
  display.drawLine(0,(display.height()*7/8),display.width(),(display.height()*7/8),EPD_GRAY);
  // splitting sensor vs. outside values
  display.drawLine((display.width()/2),0,(display.width()/2),(display.height()*7/8),EPD_GRAY);

  // battery status
  screenBatteryStatus();

  display.setTextColor(EPD_BLACK);

  // indoor and outdoor labels
  display.setCursor(((display.width()/4)-10),((display.height()*1/8)-11));
  display.print("Here");
  display.setCursor(((display.width()*3/4-12)),((display.height()*1/8)-11));
  display.print("Outside");

  display.setTextSize(1);
  display.setFont(&FreeSans9pt7b);

  // indoor info
  int temperatureDelta = ((int)(sensorData.internalTempF +0.5)) - ((int) (averageTempF + 0.5));
  int humidityDelta = ((int)(sensorData.internalHumidity +0.5)) - ((int) (averageHumidity + 0.5));

  display.setCursor(5,((display.height()*3/8)-10));
  display.print(String("Temp ") + (int)(sensorData.internalTempF+0.5) + "F (" + temperatureDelta + ")");

  display.setCursor(5,((display.height()*5/8)-10));
  display.print(String("Humid ") + (int)(sensorData.internalHumidity+0.5) + "% (" + humidityDelta + ")");

  if (sensorData.internalCO2!=10000)
  {
    display.setCursor(5,((display.height()*7/8)-10));
    display.print(String("C02 ") + sensorData.internalCO2 + " (" + (sensorData.internalCO2 - averageCO2) + ")");
  }

  // outdoor info
  if (sensorData.extTemperature!=10000)
  {
    display.setCursor(((display.width()/2)+5),((display.height()*3/8)-10));
    display.print(String("Temp ") + sensorData.extTemperature + "F");
  }
  if (sensorData.extHumidity!=10000)
  {
    display.setCursor(((display.width()/2)+5),((display.height()*5/8)-10));
    display.print(String("Humidity ") + sensorData.extHumidity + "%");
  }
  // air quality index (AQI)
  if (sensorData.extAQI!=10000)
  {
    display.setCursor(((display.width()/2)+5),((display.height()*7/8)-10));
    display.print("AQI ");
    display.print(aqiLabels[(sensorData.extAQI-1)]);
  }

  // message
  display.setFont();  // resets to system default monospace font
  display.setCursor(5,(display.height()-10));
  display.print(messageText);

  //update display
  display.display();
  debugMessage("Screen updated");
}

void screenInfo2(String messageText)
// Display environmental information on screen
{
  String aqiLabels[5]={"Good", "Fair", "Moderate", "Poor", "Very Poor"};
  
  display.clearBuffer();
  display.setFont(&FreeSans12pt7b);
  //display.setTextSize(1);
  display.setTextColor(EPD_BLACK);

  // borders
  // ThinkInk 2.9" epd is 296x128 pixels
  // label border
  display.drawLine(0,(display.height()/8),display.width(),(display.height()/8),EPD_GRAY);
  // temperature area
  display.drawLine(0,(display.height()*3/8),display.width(),(display.height()*3/8),EPD_GRAY);
  // humidity area
  display.drawLine(0,(display.height()*5/8),display.width(),(display.height()*5/8),EPD_GRAY);
  // C02 area
  // // label border
  // display.drawLine(0,(display.height()/8),display.width(),(display.height()/8),EPD_GRAY);
  // // temperature area
  // display.drawLine(0,(display.height()*3/8),display.width(),(display.height()*3/8),EPD_GRAY);
  // // humidity area
  // display.drawLine(0,(display.height()*5/8),display.width(),(display.height()*5/8),EPD_GRAY);
  // message area
  display.drawLine(0,(display.height()*7/8),display.width(),(display.height()*7/8),EPD_GRAY);
  // splitting sensor vs. outside values
  display.drawLine((display.width()/2),0,(display.width()/2),(display.height()*7/8),EPD_GRAY);
  // battery status
  screenBatteryStatus();

  display.setTextColor(EPD_BLACK);

  // indoor and outdoor labels
  display.setCursor(((display.width()/4)-10),((display.height()*1/8)-11));
  display.print("Here");
  display.setCursor(((display.width()*3/4-12)),((display.height()*1/8)-11));
  display.print("Outside");

  display.setTextSize(1);
  display.setFont(&FreeSans9pt7b);

  // indoor info
  int temperatureDelta = ((int)(sensorData.internalTempF +0.5)) - ((int) (averageTempF + 0.5));
  int humidityDelta = ((int)(sensorData.internalHumidity +0.5)) - ((int) (averageHumidity + 0.5));

  display.setCursor(5,((display.height()*3/8)-10));
  display.print(String("Temp ") + (int)(sensorData.internalTempF+0.5) + "F (" + temperatureDelta + ")");
  display.print(String("Temp ") + (int)(sensorData.internalTempF+0.5) + "F");
  if (temperatureDelta!=0)
  {
    if(temperatureDelta>0)
    {
      display.fillTriangle(100,((display.height()*3/8)-10),90,(((display.height()*3/8)-10)+9),110,(((display.height()*3/8)-10)+9),EPD_BLACK);
    }
    else
    {
      display.fillTriangle(100,(((display.height()*3/8)-10)+9),90,((display.height()*3/8)-10),110,((display.height()*3/8)-10),EPD_BLACK);
    }
    display.setCursor(112,((display.height()*3/8)-10));
    display.print(temperatureDelta);
  }

  display.setCursor(5,((display.height()*5/8)-10));
  display.print(String("Humid ") + (int)(sensorData.internalHumidity+0.5) + "% (" + humidityDelta + ")");
  display.print(String("Humid ") + (int)(sensorData.internalHumidity+0.5) + "%");
  if (humidityDelta!=0)
  {
    if(humidityDelta>0)
    {
      display.fillTriangle(100,((display.height()*5/8)-10),90,(((display.height()*5/8)-10)+9),110,(((display.height()*5/8)-10)+9),EPD_BLACK);
    }
    else
    {
      display.fillTriangle(100,(((display.height()*5/8)-10)+9),90,((display.height()*5/8)-10),110,((display.height()*5/8)-10),EPD_BLACK);
    }
    display.setCursor(112,((display.height()*5/8)-10));
    display.print(humidityDelta);
  }

  if (sensorData.internalCO2!=10000)
  {
    display.setCursor(5,((display.height()*7/8)-10));
    display.print(String("C02 ") + sensorData.internalCO2 + " (" + (sensorData.internalCO2 - averageCO2) + ")");
    display.print(String("C02 ") + sensorData.internalCO2);
    if (humidityDelta!=0)
    {
      if(humidityDelta>0)
      {
        display.fillTriangle(100,((display.height()*7/8)-10),90,(((display.height()*7/8)-10)+9),110,(((display.height()*7/8)-10)+9),EPD_BLACK);
      }
      else
      {
        display.fillTriangle(100,(((display.height()*7/8)-10)+9),90,((display.height()*7/8)-10),110,((display.height()*7/8)-10),EPD_BLACK);
      }
      display.setCursor(112,((display.height()*7/8)-10));
      display.print(sensorData.internalCO2 - averageCO2);
    }
  }

  // outdoor info
  if (sensorData.extTemperature!=10000)
  {
    display.setCursor(((display.width()/2)+5),((display.height()*3/8)-10));
    display.print(String("Temp ") + sensorData.extTemperature + "F");
  }
  if (sensorData.extHumidity!=10000)
  {
    display.setCursor(((display.width()/2)+5),((display.height()*5/8)-10));
    display.print(String("Humidity ") + sensorData.extHumidity + "%");
  }
  // air quality index (AQI)
  if (sensorData.extAQI!=10000)
  {
    display.setCursor(((display.width()/2)+5),((display.height()*7/8)-10));
    display.print("AQI ");
    display.print(aqiLabels[(sensorData.extAQI-1)]);
  }

  // message
  display.setFont();  // resets to system default monospace font
  display.setCursor(5,(display.height()-10));
  display.print(messageText);

  //update display
  display.display();
  debugMessage("Screen updated");
}

void initBattery()
{
  if (lc.begin())
  // Check battery monitoring status
  {
    debugMessage("Battery monitor ready");
    lc.setPackAPA(BATTERY_APA);
    batteryAvailable = true;
  }
  else
  {
    debugMessage("Battery monitor not detected");
  }
}

void screenBatteryStatus()
// Displays remaining battery % as graphic in lower right of screen
// used in XXXScreen() routines 
{
  #ifdef SCREEN
    if (batteryAvailable)
    {
      // render battery percentage to screen

      int barHeight = 10;
      int barWidth = 28;
      // stored so we don't call the function twice in the routine
      float percent = lc.cellPercent();
      debugMessage("Battery is at " + String(percent) + " percent capacity");
      debugMessage("Battery voltage: " + String(lc.cellVoltage()) + " v");

      //calculate fill
      display.fillRect((display.width()-33),((display.height()*7/8)+4),(int((percent/100)*barWidth)),barHeight,EPD_GRAY);
      // border
      display.drawRect((display.width()-33),((display.height()*7/8)+4),barWidth,barHeight,EPD_BLACK);
    }
  #endif
}