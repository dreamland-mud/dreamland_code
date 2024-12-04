#ifndef CALENDAR_H
#define CALENDAR_H

/*
 * Time and weather stuff.
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3

struct time_info_data
{
    int hour;
    int day;
    int month;
    int year;
};

struct weather_data
{
    int mmhg;
    int avg_mmhg;
    int change;
    int change_;
    int sky;
    int sunlight;
};

typedef struct time_info_data TIME_INFO_DATA;
typedef struct weather_data WEATHER_DATA;

extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;

#endif
