//
// Created by MATSUNAGA Takuji on 2019-07-20.
//

#ifndef GNSS_H
#define GNSS_H

#include <time.h>
#include <Stream.h>
#include <TinyGPS++.h>

#define TIME_OFFSET 9 * 3600

class GNSS {
public:
    typedef struct {
        uint8_t     mode;
        double      latitude;
        double      longitude;
        time_t      ti;
        double      speed;
        double      course;
        double      altitude;
        uint8_t     satellites;
        uint8_t     used[12];
    } positioningData_t;

    GNSS(uint8_t, void(*)(void));
    void begin(Stream&);
    bool loop();

    const char* get_mode_str();
    void get_datetime_str(char* buf) {
        struct tm *jst;
        jst = localtime(&positioningData.ti);
        sprintf(buf, "%4d/%02d/%02d %02d:%02d:%02d",
                jst->tm_year + 1900, jst->tm_mon + 1, jst->tm_mday, jst->tm_hour, jst->tm_min, jst->sec);
    }
    void get_lat_str(char *buf) {
        sprintf(buf, "%9.5f", positioningData.latitude);
    }
    void get_long_str(char* buf) {
        sprintf(buf, "%9.5f", positioningData.longitude);
    }
    void get_course_str(char* buf) {
        sprintf(buf, "%5.1f Deg.", positioningData.course);
    }

private:
    void(*callback)(void);
    positioningData_t positioningData;
    struct tm utc;

    Stream *gnssRaw;

    TinyGPSPlus gps;
    TinyGPSCustom positioningMode;
    TinyGPSCustom userSatellite[12];

    uint8_t ppsPin;

    void store_data();
};

#endif //GNSS_H
