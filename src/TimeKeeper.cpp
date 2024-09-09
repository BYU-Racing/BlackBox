#include <BlackBox.h>
#include "TimeKeeper.h"

void TimeKeeper::init(
    const bool IS_RTC_ACTIVE, const bool IS_GPS_ACTIVE, const int TIME_SYNC_RATE=0
    )
{
    RTC_ACTIVE = IS_RTC_ACTIVE;
    GPS_ACTIVE = IS_GPS_ACTIVE;
    SYNC_RATE = TIME_SYNC_RATE;
}

bool TimeKeeper::isActive() const { return RTC_ACTIVE || GPS_ACTIVE; }

TimeKeeper::operator bool() const { return isActive(); }

time_t TimeKeeper::getRTCTime() { return Teensy3Clock.get(); }

// TODO: Implement GPS Time Sync
time_t TimeKeeper::getGPSTime() { return 0; }

bool TimeKeeper::setBestSyncProvider() const
{
    if (RTC_ACTIVE)
    {
        setSyncProvider(getRTCTime);
        return true;
    }
    if (GPS_ACTIVE)
    {
        setSyncProvider(getRTCTime);
        return true;
    }
    return false;
}
/** @return The current date and time in 'YYYY-MM-DD_HH:MM:SS' format. */
String TimeKeeper::getDateTime()
{
    const time_t time = now();
    char buffer[20]; // 19 chars + 1 \0
    sprintf(
        buffer, "%04d-%02d-%02d_%02d:%02d:%02d",
        year(time),
        month(time),
        day(time),
        hour(time),
        minute(time),
        second(time)
    );
    return {buffer};
}
