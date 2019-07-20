//
// Created by MATSUNAGA Takuji on 2019-07-20.
//

#include <Arduino.h>
#include <string.h>
#include "gnss.h"

GNSS::GNSS(uint8_t ppsPin, void(*callback)(void)) {
    this->ppsPin = ppsPin;
    this->callback = callback;

    positioningMode.begin(gps, "GPGSA", 2);
    for (int i = 0; i < 12; i++) {
        userSatellite[i].begin(gps, "GPGSA", i + 3);
    }

    memset(&positioningData, 0, sizeof(positioningData_t));
    memset(&utc, 0, sizeof(struct tm));
};

void GNSS::begin(Stream& gnssRaw) {
    this->gnssRaw = &gnssRaw;
    attachInterrupt(ppsPin, this->callback, FALLING);
}

bool GNSS::loop() {
    bool isValid = false;
    if (gnssRaw->available()) {
        int ch = gnssRaw->read();
        isValid = gps.encode(ch);

        if (isValid) {
            noInterrupts();
            store_data();
            interrupts();
        }
    }

    return isValid;
};

void GNSS::store_data() {
    if (positioningMode.isUpdated()) {
        positioningData.mode = atoi(positioningMode.value());
        for (int i = 0; i < 12; i++) {
            positioningData.used[i] = atoi(userSatellite[i].value());
        }
    }

    if (gps.location.isUpdated()) {
        positioningData.latitude = gps.location.lat();
        positioningData.longitude = gps.location.lng();
    }
    if (gps.date.isUpdated() && gps.date.isValid()) {
        utc.tm_year = gps.date.year() - 1900;
        utc.tm_mon = gps.date.month() - 1;
        utc.tm_mday = gps.date.day();
    }
    if (gps.time.isUpdated() && gps.time.isValid()) {
        utc.tm_hour = gps.time.hour();
        utc.tm_min = gps.time.minute();
        utc.tm_sec = gps.time.second();

        if (utc.tm_year != 0) {
            positioningData.ti = mktime(&utc) + TIME_OFFSET;
        }
    }
    if (gps.speed.isUpdated()) {
        positioningData.speed = gps.speed.kmph();
    }
    if (gps.course.isUpdated()) {
        positioningData.course = gps.course.deg();
    }
    if (gps.altitude.isUpdated()) {
        positioningData.altitude = gps.altitude.meters();
    }
    if (gps.satellites.isUpdated()) {
        positioningData.satellites = gps.satellites.value();
    }
}

const char* GNSS::get_mode_str() {
    const PROGMEM char *mode_str[] = {"--", "NO", "2D", "3D"};

    uint8_t mode = positioningData.mode;
    if (mode > 3) {
        mode = 0;
    }

    return mode_str[mode];
}

