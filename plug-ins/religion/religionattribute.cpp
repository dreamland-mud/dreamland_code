#include "logstream.h"
#include "religionattribute.h"
#include "defaultreligion.h"
#include "pcharacter.h"
#include "bonusflags.h"
#include "date.h"
#include "dreamland.h"
#include "merc.h"

static long day_of_epoch(int year, int month, int day)
{
    return year * 35 * 17 + month * 35 + day;
}

static long day_of_epoch(const struct time_info_data &ti)
{
    return day_of_epoch(ti.year, ti.month, ti.day);
}

static long day_of_epoch(const XMLTimeInfo &ti)
{
    return day_of_epoch(ti.year, ti.month, ti.day);
}

void XMLTimeInfo::fromTime(const struct time_info_data &ti) 
{
    day = ti.day;
    month = ti.month;
    year = ti.year;
}

void XMLTimeInfo::fromDayOfEpoch(long lday) 
{
    long lmonth = lday / 35;
    this->day = lday % 35;
    this->month = lmonth % 17;
    this->year = lmonth / 17;
}

DLString XMLTimeInfo::toString() const
{
    ostringstream buf;
    buf << day << "/" << month << "/" << year;
    return buf.str();
}

XMLAttributeReligion::XMLAttributeReligion( )
                : bonuses(0, &bonus_flags), prevBonuses(0, &bonus_flags)
{
}

XMLAttributeReligion::~XMLAttributeReligion( )
{
}

bool XMLAttributeReligion::bonusUsedRecently(const Flags &bonus) const
{
    return prevBonuses.isSet(bonus);    
}

bool XMLAttributeReligion::handle( const ScoreArguments &args )
{
    if (hasBonus(time_info)) {
        args.lines.push_back("У тебя сегодня счастливый день!");
        return true;
    }
    return false;
}

bool XMLAttributeReligion::hasBonus(const bitstring_t &bonus) const
{
    if (!hasBonus(time_info))
        return false;

    return bonuses.isSet(bonus);
}

bool XMLAttributeReligion::hasBonus(const struct time_info_data &ti) const
{
    if (day_of_epoch(ti) < day_of_epoch(start))
        return false;

    if (day_of_epoch(ti) > day_of_epoch(end))
        return false;

    return true;
}

void XMLAttributeReligion::setLuckyWeek(const Flags &bonus)
{
    start.fromTime(time_info);
    long lday = day_of_epoch(time_info) + 8; 
    end.fromDayOfEpoch(lday);

    bonuses.setValue(bonus.getValue());
    prevBonuses.setValue(bonus.getValue());
}

