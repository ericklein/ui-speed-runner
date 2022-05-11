// conditional compile flags
#define DEBUG 	// Output to serial port
//#define RJ45  	// use Ethernet
//#define WIFI    	// use WiFi

#define SCREEN_DISPLAY_PERIOD 5000

// set client ID; used by mqtt and wifi
#define CLIENT_ID "USR-test"

// Open Weather Map parameters
#define OWM_SERVER			"http://api.openweathermap.org/data/2.5/"
#define OWM_WEATHER_PATH	"weather?"
#define OWM_AQM_PATH		"air_pollution?"

// select time zone, used by NTPClient
//const int timeZone = 0;  	// UTC
//const int timeZone = -5;  // USA EST
//const int timeZone = -4;  // USA EDT
//const int timeZone = -8;  // USA PST
const int timeZone = -7;  // USA PDT

// Battery parameters
// based on a settings curve in the LC709203F datasheet
// #define BATTERY_APA 0x08 // 100mAH
// #define BATTERY_APA 0x0B // 200mAH
#define BATTERY_APA 0x10 // 500mAH
// #define BATTERY_APA 0x19 // 1000mAH
// #define BATTERY_APA 0x1D // 1200mAH
// #define BATTERY_APA 0x2D // 2000mAH
// #define BATTERY_APA 0x32 // 2500mAH
// #define BATTERY_APA 0x36 // 3000mAH

// The following parameters are defined in secrets.h.
// 	WiFi credentials (if WiFi enabled)
// 	#define WIFI_SSID
// 	#define WIFI_PASS       

// 	Open Weather Map
// 	#define OWM_KEY
//	#define OWM_LAT_LONG