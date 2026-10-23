#include "logstream.h"
#include "calendar_utils.h"
#include "religionattribute.h"
#include "defaultreligion.h"
#include "pcharacter.h"
#include "date.h"
#include "dreamland.h"
#include "merc.h"
#include "idcontainer.h"

using namespace Scripting;

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

// attempts, successes and angers of sacrifices, for Fenia (attribute("religion")).
Register XMLAttributeReligion::toRegister() const
{
    Register attrReg = Register::handler<IdContainer>();
    IdContainer *c = attrReg.toHandler().getDynamicPointer<IdContainer>();

    c->setField(IdRef("attempts"), (int)attempts.getValue());
    c->setField(IdRef("successes"), (int)successes.getValue());
    c->setField(IdRef("angers"), (int)angers.getValue());
    return attrReg;
}
