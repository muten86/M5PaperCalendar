// myCalendar.h

// the functions to draw the calendar

#pragma once

// calendar helper: leap year?
bool isLeapYear(int year) {
	if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
		return true;
	return false;
}

// calendar helper: days in month
int getDaysInMonth(int month, int year) {
	int daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (month != 2)
		return daysInMonth[(month - 1) % 12];
	if (isLeapYear(year))
		return 29;
	return 28;
}

// calendar helper: day of week 0 = Sunday, 1 = Monday, 2 = Tuesday, 3 = Wednesday, 4 = Thursday, 5 = Fryday, 6 = Saturday
int getDayOfWeek(int year, int month, int day) {
	uint16_t months[] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };   // days until 1st of month

	uint32_t days = year * 365;                                         // days until year 
	for (uint16_t i = 4; i < year; i += 4) if (isLeapYear(i)) days++;   // adjust leap years, test only multiple of 4 of course

	days += months[month - 1] + day;                                    // add the days of this year
	if ((month > 2) && isLeapYear(year)) days++;                        // adjust 1 if this year is a leap year, but only after febr

	// make Sunday 0
	days--;
	if (days < 0)
		days += 7;
	return days % 7;                                                    // remove all multiples of 7
}

// build and show the mini calendar
void miniCalendar() {

	uint8_t virtOldMM = 0;
	uint8_t virtIndex = 0;

	int yOffset = 220;

	// add the days of the week
	float col_width = (300 - 14 - 15) / 7;
	sideBar.setTextSize(fontSize);
	sideBar.setTextDatum(TC_DATUM);
	for (byte r = 1; r < 8; r++) {
		sideBar.drawString(dayShortNamesDD[r], 15 + (col_width * (r - 1)) + (col_width / 2), yOffset);
	}

	yOffset += 25;

	sideBar.drawFastHLine(15, yOffset, 270, 2, MYWHITE);

	yOffset += 24;

	int virtCurCount = getDaysInMonth(timevar.tm_mon + 1, timevar.tm_year + 1900);
	int virt1wd = getDayOfWeek(timevar.tm_year + 1900, timevar.tm_mon + 1, 1);

	if (virt1wd == 0) virt1wd = 7;                  // if first day of current month is sunday

	sideBar.setTextSize(fontSize);

	uint8_t virtMon[42];                            // create a virtual month with 42 days - 6 rows with 7 columns
	for (int8_t i = 0; i < 42; i++) {
		virtMon[i] = 0;
	}

	if (timevar.tm_mon + 1 == 1) virtOldMM = 12;        // it's because January = 0 and December = 11
	else virtOldMM = timevar.tm_mon +1 - 1;

	int virtOldCount = getDaysInMonth(virtOldMM, timevar.tm_year + 1900);

	// fill in the month before the current one
	if (virt1wd == 1) {                             // first day of current month is monday
		for (int8_t i = 6; i >= 0; i--) {
			virtMon[virtIndex] = virtOldCount - i;
			virtIndex++;
		}
	}
	else {                                          // first day of current month is not a monday
		for (int8_t i = virt1wd - 2; i >= 0; i--) {
			virtMon[virtIndex] = virtOldCount - i;
			virtIndex++;
		}
	}
	// fill in the current month
	for (int8_t i = 1; i <= virtCurCount; i++) {
		virtMon[virtIndex] = i;
		virtIndex++;
	}

	// fill in the following month
	uint8_t z = virtIndex;
	for (int8_t i = z; i <= 41; i++) {
		virtMon[virtIndex] = i - z + 1;
		virtIndex++;
	}

	uint8_t fg = MYGREY;
	uint8_t pbID = 0;
	for (byte row = 0; row < 6; row++) {       // 6 rows
		for (byte col = 1; col < 8; col++) {   // 7 coloumns

			uint8_t w = virtMon[(col - 1) + row * 7];

			if (w == 1) pbID++;

			if (pbID == 1) fg = MYWHITE;
			else fg = MYGREY;

			sideBar.setTextColor(fg, MYBLACK);

			if ((pbID == 1) && (w == timevar.tm_mday)) {
				fg = MYBLACK;
				sideBar.fillCircle((15 + (col_width * (col - 1)) + (col_width / 2)), (yOffset + (30 * row) + 8), 16, MYWHITE);
				sideBar.setTextColor(fg, MYWHITE);
			}

			sideBar.drawString(String(w).c_str(), (15 + (col_width * (col - 1)) + (col_width / 2)), yOffset + (30 * row));
		}
	}
}


// build and show the calendar sidebar
void showSideBar() {

	getLocalTime(&timevar);

	// Day, Weekday, Month and Year
	char day[3];
	sprintf(day, "%02d", timevar.tm_mday);
	const char* weekday = dayNames[timevar.tm_wday];
	const char* month = monthNames[timevar.tm_mon + 1];

	// black background white foreground
	sideBar.fillCanvas(MYBLACK);
	sideBar.setTextColor(MYWHITE, MYBLACK);

	// weekday
	sideBar.setTextSize(fontSize);
	sideBar.setTextDatum(TC_DATUM);
	sideBar.drawString(weekday, 150, 20);

	// day
	sideBar.setTextSize(fontSize);
	sideBar.setTextDatum(TC_DATUM);
	sideBar.drawString(day, 150, 75);

	// month and year
	sideBar.setTextSize(fontSize);
	sideBar.setTextDatum(TC_DATUM);
	char monthYear[20];
	sprintf(monthYear, "%s %04d", month, timevar.tm_year + 1900);
	sideBar.drawString(monthYear, 150, 170);

	miniCalendar();

	sideBar.pushCanvas(0, 0, UPDATE_MODE_GC16);

}

/* build and show a little clock*/
void showClock() {

	getLocalTime(&timevar);

	char timeString[16];
	sprintf(timeString, " %02d:%02d ", timevar.tm_hour, timevar.tm_min);

	myClock.fillCanvas(MYWHITE);
	myClock.setTextColor(MYBLACK, MYWHITE);
	myClock.setTextSize(fontSize);
	myClock.setTextDatum(TC_DATUM);
	myClock.drawString(timeString, 90, 18);
	myClock.setTextSize(fontSize);
	drawBattery(myClock, 250, 10);
	drawRSSI(myClock, 250, 35);
	myClock.drawString(calendarOK, 250, 60);
	myClock.pushCanvas(650, 445, UPDATE_MODE_GC16);
}

bool readCalendar() {
	//WiFiClient client;
	//HTTPClient http;

	// Getting calendar from google script
	Serial.println("Getting calendar");
	Serial.println(calendarRequest);

	if (!checkWiFi()) connectWiFi();

	//client.stop();
	http.end();
	http.setTimeout(20000);
	http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
	if (!http.begin(calendarRequest)) {
		Serial.println("Cannot connect to google script");
		//client.stop();
		http.end();
		return false;
	}

	Serial.println("Connected to google script");
	int returnCode = http.GET();
	Serial.print("Returncode: ");
	Serial.println(returnCode);
	//if (returnCode != HTTP_CODE_OK) return false;

	String response = http.getString();
	Serial.print("Response: ");
	Serial.println(response);

	int indexFrom = 0;
	int indexTo = 0;
	int cutTo = 0;

	String strBuffer = "";

	int count = 0;
	int line = 0;

	indexFrom = response.indexOf("#") + 1;

	Serial.println("Indexfrom: " + indexFrom);

	// Fill calendarEntries with entries from the get-request
	while (indexTo >= 0 && line < calEntryCount) {
		count++;
		indexTo = response.indexOf("#", indexFrom);
		cutTo = indexTo;

		if (indexTo != -1) {
			strBuffer = response.substring(indexFrom, cutTo);

			indexFrom = indexTo + 1;

			Serial.println(strBuffer);

			if (count == 1) {
				// Set start date
				calEnt[line].calStartDate = strBuffer;
			}
			else if (count == 2) {
				// Set start time
				calEnt[line].calStartTime = strBuffer;
			}
			else if (count == 3) {
				// Set end date
				calEnt[line].calEndDate = strBuffer;
			}
			else if (count == 4) {
				// Set end time
				calEnt[line].calEndTime = strBuffer;
			}
			else if (count == 5) {
				// Set title
				calEnt[line].calTitle = strBuffer;
			}
			else if (count == 6) {
				// Set location
				calEnt[line].calLocation = strBuffer;
			}
			else if (count == 7) {
				// Set allDay
				calEnt[line].calAllDay = strBuffer;
			}
			else if (count == 8) {
				// Set color
				calEnt[line].calColor = strBuffer;
				count = 0;
				line++;
			}
		}
	}
	//client.stop();
	http.end();
	Serial.println("Calendar readY!");
	return true;
}

int eventItem(int y, const char* description, const char* location, const char* start, const char* end) {

	int y_start = y;

	// Events Description
	events.setTextSize(fontSize);
	textEllipsis(events, 95, y + 15, (800 - 365), description);

	// Event Start Time
	events.setTextSize(fontSize);
	events.setTextDatum(TR_DATUM);
	events.drawString(start, 650, y + 20 - 6);

	if (String(end).length() > 0) {
		events.setTextSize(fontSize);
		events.setTextDatum(TR_DATUM);
		events.drawString(end, 650, y + 20 + 15);
	}

	if (String(location).length() > 0) {
		// Event Location
		y += 20;
		events.setTextSize(fontSize);
		textEllipsis(events, 95, y + 25, (800 - 365), location);
	}

	// Bottom margin
	y += 20 + 30;

	// Side bar
	events.drawLine(85, y_start, 85, y, MYBLACK);

	return y;
}

int eventDate(int y, const char* day, const char* weekday) {

	// Day
	events.setTextColor(MYBLACK, MYWHITE);
	events.setTextSize(fontSize);
	events.setTextDatum(TR_DATUM);
	events.drawString(day, 75, y + 20);

	// Weekday
	y += 12;
	events.setTextColor(MYBLACK, MYWHITE);
	events.setTextSize(fontSize);
	events.setTextDatum(TR_DATUM);
	events.drawString(weekday, 75, y + 20 + 20);

	return y;
}


void eventList() {

	rC = readCalendar();

	Serial.println("Calendar OKX? " + String(rC));
	if (rC) calendarOK = "+";
	else calendarOK = "-";

	if (!rC) {
		// Google calendar could not be read!!
    	Serial.println("Error Calendar: " + String(rC));
		return;
	}

	Serial.println("Calendar midstep ");
	getLocalTime(&timevar);

	events.fillCanvas(MYWHITE);
	events.setTextSize(fontSize);
	events.setTextDatum(TL_DATUM);
	events.setTextColor(MYBLACK, MYWHITE);

	// Day and Weekday
	int start = 20; // Top padding
	int checkDay;
	int checkMonth;
	int checkYear;
	int checkOldDay;

	// Build List
	bool isEmpty = true;
	for (byte r = 0; r < 10; r++) {

		checkDay = calEnt[r].calStartDate.substring(9, 10).toInt();
		checkMonth = calEnt[r].calStartDate.substring(5, 6).toInt();
		checkYear = calEnt[r].calStartDate.substring(0, 3).toInt();

	Serial.println("Calendar date: " + String(checkDay) +String(checkMonth) +String(checkYear));
		// Add new day
		if (r == 0 || checkOldDay != checkDay) {

			if (calEnt[r].calTitle == "")
				break;

			if (r > 0)
				start += 20; // Add padding

			eventDate(start, String(checkDay).c_str(), dayShortNamesDDD[getDayOfWeek(checkYear, checkMonth, checkDay)]);
		}

		// Display event
		if (calEnt[r].calAllDay == "true") {
			start = eventItem(start, calEnt[r].calTitle.c_str(), calEnt[r].calLocation.c_str(), "ganztag", "");
		}
		else {
			start = eventItem(start, calEnt[r].calTitle.c_str(), calEnt[r].calLocation.c_str(), calEnt[r].calStartTime.c_str(), calEnt[r].calEndTime.c_str());
		}

		// Stop if no more room for a new event
		if (start > 350)
			break;

		// set day
		checkOldDay = checkDay;
	}

	events.pushCanvas(300, 0, UPDATE_MODE_GC16);
}
