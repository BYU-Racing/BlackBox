#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <Arduino.h>
#include <TimeLib.h>

class TimeKeeper {
public:
    TimeKeeper() : RTC_ACTIVE(false), GPS_ACTIVE(false), SYNC_RATE(-1) {  }
    void init(bool IS_RTC_ACTIVE, bool IS_GPS_ACTIVE, int TIME_SYNC_RATE);
    explicit operator bool() const;
    bool isActive() const;
    static String getDateTime();
private:
    bool RTC_ACTIVE;
    bool GPS_ACTIVE;
    int SYNC_RATE;
    static time_t getRTCTime();
    static time_t getGPSTime();
    bool setBestSyncProvider() const;
};

#endif //TIMEKEEPER_H
