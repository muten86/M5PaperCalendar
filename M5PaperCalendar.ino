/*********************************************************************

  Digital Calendar on M5Paper
  The M5PAPER is equipped with an e-ink screen with a resolution of 540*960 @ 4.7" and supports 16 levels of grayscale display.
							   Btn P/R/L
								   |
	 +-----------------------------�-------------------+
	 | 0/0   ePaper  960 x 540                         |
	 |                                                 |
	 |    16 levels grayscale                          |
	 |                                                 |
	 |                                                 |
	 |                                                 |
	 |                                        960/540  |
	 +-------------------------------------------------+

	 https://docs.m5stack.com/en/api/m5paper/system_api

  Time is synchronised with a NTP-Server
  NTP-Server: FritzBox  :-)

  In the SPIFFS must be stored the used TrueTypeFont!!

  Used Libraries:
  - M5EPD: M5Paper lib https://github.com/m5stack/M5EPD
  - ArduinoJson:       https://arduinojson.org/  V6.x or above
  - WiFi:              Arduino IDE
  - HTTPClient:        Arduino IDE
  - WiFiClientSecure:  Arduino IDE
  - TimeLib:           https://github.com/PaulStoffregen/Time

  Three pages / screens:
  1. Calendar
  2. Today
  3. Weather


  Inspired by this:
  Calendar:  https://github.com/SeBassTian23/ESP32-CalendarDisplay
  PureClock: https://github.com/azw413/M5-Clock
  Weather:   https://github.com/mzyy94/M5PaperWeather/tree/jp-model

  V0.1  20.11.2021: Initial release
  V0.2  22.11.2021: Added the pure clock page
  V0.3  27.11.2021: Added the weather page
  V0.4  05.12.2021: Small bug fixes, show "+" if read data was ok
  V0.5  18.12.2021: Tuned the mini calendar, renamed "pure clock" to "today"


 *********************************************************************/

#define FIRMWARE "0.5 - 2021-12-18"

 /**************************************************************************************
 **     Libraries
 ***************************************************************************************/
#include <M5EPD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <TimeLib.h>

#include <ArduinoJson.h>

#include <Credentials.h>

 /***************************************************************************************
 **                          Define the globals and class instances
 ***************************************************************************************/

 // Network
 // SSID and password in credentials.h
IPAddress ip(192, 168, 178, 235);                            // Static IP
IPAddress gateway(192, 168, 178, 1);
IPAddress dns(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);

#define HOSTNAME "M5PaperCalendarWeatherClock"

#define MYBLACK WHITE
#define MYWHITE BLACK
#define MYGREY M5EPD_Canvas::G9

// time and date vars
struct tm tm;
struct tm start;
rtc_time_t currentTime;
rtc_date_t currentDate;
byte oldMinute = 65;
byte oldDay = 0;
byte oldHour = 25;

const char* const PROGMEM ntpServer[] = { "fritz.box", "de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org" };
const char* const PROGMEM dayNames[] = { "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag" };
const char* const PROGMEM dayShortNamesDD[] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So" };
const char* const PROGMEM dayShortNamesDDD[] = { "Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam", "Son" };
const char* const PROGMEM monthNames[] = { "Error",  "Januar", "Februar", "M\u00e4rz", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember" }; //M\u00e4rz
const char* const PROGMEM monthShortNamesMMM[] = { "Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez" };

// internal temperature and humidity
int     sht30Temperatur;  // SHT30 temperature
int     sht30Humidity;    // SHT30 humidity

// the calendar vars
const int calEntryCount = 10;
const int calEntryMuellCount = 10;
bool rC = false;
String calendarOK = "x";

struct calendarEntries {
	String calStartDate;
	String calStartTime;
	String calEndDate;
	String calEndTime;
	String calTitle;
	String calLocation;
	String calAllDay;
	String calColor;
};
struct calendarEntries calEnt[calEntryCount];

// Muell calendar struct
struct calMuellEntries {
	String calStartDate;
	String calTitle;
	String calColor;
};

// the weather vars
#define MAX_HOURLY   24
#define MAX_FORECAST  8
#define MIN_RAIN      5

struct Weather {
	time_t currentTime;                     // Current timestamp
	int    currentTimeOffset;               // Current timezone

	time_t sunrise;                         // Sunrise timestamp
	time_t sunset;                          // Sunset timestamp
	float  winddir;                         // Wind direction
	float  windspeed;                       // Wind speed

	time_t hourlyTime[MAX_HOURLY];          // timestamp of the hourly forecast
	int    hourlyTempRange[2];              // min/max temp of the hourly forecast
	float  hourlyMaxTemp[MAX_HOURLY];       // max temperature forecast
	int    hourlyMaxRain;                   // maximum rain in mm of the hourly forecast
	float  hourlyRain[MAX_HOURLY];          // max rain in mm
	float  hourlyPop[MAX_HOURLY];           // pop of the hourly forecast
	float  hourlyPressure[MAX_HOURLY];      // air pressure
	String hourlyMain[MAX_HOURLY];          // description of the hourly forecast
	String hourlyIcon[MAX_HOURLY];          // openweathermap icon of the forecast weather

	time_t forecastTime[MAX_FORECAST];      // timestamp of the daily forecast
	int    forecastTempRange[2];            // min/max temp of the daily forecast
	float  forecastMaxTemp[MAX_FORECAST];   // max temperature
	float  forecastMinTemp[MAX_FORECAST];   // min temperature
	int    forecastMaxRain;                 // maximum rain in mm of the daily forecast
	float  forecastRain[MAX_FORECAST];      // max rain in mm
	float  forecastPop[MAX_FORECAST];       // pop of the dayly forecast
	float  forecastPressure[MAX_FORECAST];  // air pressure
	String forecastIcon[MAX_FORECAST];      // openweathermap icon of the forecast weather
};
Weather weather;
bool rW = false;
String weatherOK = "x";

// the page: 1 = Today, 2 = Calendar, 3 = weather
byte page = 1;

// the display 
int maxX = 960;
int maxY = 540;
// calendar page
M5EPD_Canvas sideBar(&M5.EPD);
M5EPD_Canvas myClock(&M5.EPD);
M5EPD_Canvas events(&M5.EPD);
M5EPD_Canvas nextEvent(&M5.EPD);
//pure clock page
M5EPD_Canvas myToday(&M5.EPD);
// weather page
M5EPD_Canvas myWeather(&M5.EPD);

// the clients for weather and google
HTTPClient http;
WiFiClient client;

// include all the helpers
#include "myWiFi.h"
#include "myCalendar.h"
#include "myUtils.h"
#include "myToday.h"
#include "myWeather.h"



/////////////////////////////// Setup /////////////////////////////////////////////////////////////////////////
void setup() {

	M5.begin(false, false, true, true, false);

	// Serial is initialised by M5.begin to 115200 baud
	Serial.print("\nM5PaperCalendarWeatherClock started.\n");

	if (!SPIFFS.begin(true)) {
		Serial.println("SPIFFS Mount Failed");
	}

	M5.SHT30.Begin();
	M5.RTC.begin();

	M5.EPD.SetRotation(0);
	M5.TP.SetRotation(0);

	if (!connectWiFi()) {
		Serial.println("Connection not possible! Please check! ESP now resetting!");
		ESP.restart();
	}

	setupTime();
	M5.RTC.getTime(&currentTime);

	// calendar page
	sideBar.createCanvas(300, 540);
	myClock.createCanvas(310, 95);
	events.createCanvas(660, 445);
	nextEvent.createCanvas(350, 95);

	// today page
	myToday.createCanvas(960, 540);

	// weather page
	myWeather.createCanvas(960, 540);

	// load the ArialRound ttf from SPIFFS
	sideBar.loadFont("/ARLRDBD.TTF", SPIFFS);
	// font sizes
	sideBar.createRender(92);
	sideBar.createRender(48);
	sideBar.createRender(42);
	sideBar.createRender(36);
	sideBar.createRender(24);
	sideBar.createRender(18);
	sideBar.createRender(14);

	// clear the screen
	myToday.fillCanvas(MYWHITE);
	myToday.pushCanvas(0, 0, UPDATE_MODE_GC16);
	myToday.deleteCanvas();

}
///////////////////// End Setup /////////////////////////////////////////////////////////////////////////

/////////////////////////// Loop ////////////////////////////////////////////////////////////////////////
void loop() {

	M5.RTC.getTime(&currentTime);
	M5.RTC.getDate(&currentDate);

	getLocalTime(&tm);

	M5.update();

	// check the button
	if (M5.BtnL.wasPressed()) {
		page = 1; // button to the left  --> Pure Clock
		myToday.createCanvas(960, 540);
		myToday.fillCanvas(MYWHITE);
		myToday.pushCanvas(0, 0, UPDATE_MODE_GC16);
		myToday.deleteCanvas();
		printTime();
		printDate();
		Serial.println(page);
	}
	if (M5.BtnP.wasPressed()) {
		page = 2; // button pressed      --> Calendar
		showSideBar();
		showClock();
		events.fillCanvas(MYWHITE);
		events.setTextSize(42);
		events.setTextDatum(TL_DATUM);
		events.drawString("Hole Kalender-Daten...", 100, 100);
		events.pushCanvas(300, 0, UPDATE_MODE_GC16);
		eventList();
		Serial.println(page);
	}
	if (M5.BtnR.wasPressed()) {
		page = 3; // button to the right --> weather
		myWeather.createCanvas(960, 540);
		myWeather.fillCanvas(MYWHITE);
		myWeather.pushCanvas(0, 0, UPDATE_MODE_GC16);
		getSHT30Values();
		myWeather.fillCanvas(MYWHITE);
		myWeather.setTextSize(42);
		myWeather.setTextDatum(TL_DATUM);
		myWeather.drawString("Hole Wetter-Daten...", 100, 100);
		myWeather.pushCanvas(300, 0, UPDATE_MODE_GC16);
		showWeather();
		Serial.println(page);
	}

	if (currentTime.min != oldMinute) {
		oldMinute = currentTime.min;
		if (page == 1) printTime();
		if (page == 2) showClock();
		if (page == 3) {
			getSHT30Values();
			showM5PaperInfo();
		}
	}

	if (currentTime.hour != oldHour) {
		oldHour = currentTime.hour;
		if (page == 2) eventList();
		if (page == 3) {
			getSHT30Values();
			showWeather();
		}
	}

	if (currentDate.day != oldDay) {
		oldDay = currentDate.day;
		if (page == 1) printDate();
		if (page == 2) showSideBar();
	}

}
//////////////////// End Loop /////////////////////////////////////////////////////////////////////////////

/*************************************( functions )****************************************/


