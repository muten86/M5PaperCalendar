# M5PaperCalendar
Anzeige von Wetter und Kalender auf dem M5Paper.

Auf dem M5Paper werden auf 3 Seiten folgende Infos angezeigt:
1. Uhrzeit, Datum und ein paar Systeminfos
2. Kalender: Datum, laufender Monat und die nächsten Termine. Die Termine werden vom Google Kalender abgefragt. Dazu ist ein separates Script erforderlich (stelle ich gelegentich hier ein)(Comment Muten86: Script added)
4. Wetter: Die Wetterdaten für meinen Wohnort, die Daten kommen von OpenWeatherMap.


@updates in Muten86-Fork:
- Google Script added
- Adaptet Font-Size for M5Paper 1.1 (with global variable to control)
- Adapted Header and Footer
- optimized MyToday Page




Sämtliche Passwörter, APP-Ids und URLs sind in Credentials nach folgendem Muster hinterlegt, Einbindung als Library:

```
// My credentials

/* ------------------ WiFi --------------------- */
const char* ssid     = "xxx";
const char* password = "xxx";

/* ------------------ OpenWeatherMap --------------------- */
#define APIKEY 		     "xxx" 
#define LOCATION_ID      "xxx"      
#define LANGUAGE         "de"
#define LATITUDE         xx.xxxx                                     
#define LONGITUDE        xx.xxxx                                   
#define CITY_NAME        "xxx"
#define OPENWEATHER_SRV  "api.openweathermap.org"
#define OPENWEATHER_PORT  80

/* ------------------ Google Calendar --------------------- */
// Get calendar
char calendarServer[] = "script.google.com";
String calendarRequest = "https://script.google.com/macros/..."; 
```
