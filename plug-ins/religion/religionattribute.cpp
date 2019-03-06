#include "logstream.h"
#include "calendar_utils.h"
#include "religionattribute.h"
#include "defaultreligion.h"
#include "pcharacter.h"
#include "date.h"
#include "dreamland.h"
#include "merc.h"

BONUS(none);

XMLAttributeReligion::XMLAttributeReligion( )
{
}

XMLAttributeReligion::~XMLAttributeReligion( )
{
}

bool XMLAttributeReligion::handle( const ScoreArguments &args )
{
    if (prevBonus == bonus_none)
        return false;
    if (day_of_epoch(time_info) > prevBonusEnds)
        return false;

    args.lines.push_back("У тебя сегодня счастливый день!");
    return true;
}

