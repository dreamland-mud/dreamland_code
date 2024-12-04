#include "core/object.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "commandtemplate.h"
#include "itemutils.h"
#include "loadsave.h"
#include "skillreference.h"
#include "feniamanager.h"
#include "register-impl.h"
#include "wrappertarget.h"
#include "wrapperbase.h"
#include "interp.h"
#include "areaquestutils.h"
#include "act.h"
#include "merc.h"
#include "def.h"

GSN(curl);

/*--------------------------------------------------------------------
 *  stand / wake / sit / rest / sleep
 *-------------------------------------------------------------------*/
static bool mprog_wake(Character *ch, Character *waker)
{
    aquest_trigger(ch, waker, "Wake", "CC", ch, waker);
    FENIA_CALL(ch, "Wake", "C", waker);
    FENIA_NDX_CALL(ch->getNPC(), "Wake", "CC", ch, waker);
    return false;
}

static DLString oprog_msg(Object *obj, const char *tag)
{
    DLString msg;
    Scripting::IdRef ID_MSG(tag);
    Scripting::Register regObj, regObjIndex, reg;

    if (!FeniaManager::wrapperManager)
        return msg;

    regObj = FeniaManager::wrapperManager->getWrapper(obj);
    regObjIndex = FeniaManager::wrapperManager->getWrapper(obj->pIndexData);

    try
    {
        reg = *regObj[ID_MSG];
        if (reg.type != Scripting::Register::NONE)
            msg = reg.toString();

        if (msg.empty())
        {
            reg = *regObjIndex[ID_MSG];
            if (reg.type != Scripting::Register::NONE)
                msg = reg.toString();
        }
    }
    catch (const Exception &e)
    {
        LogStream::sendWarning() << e.what() << endl;
    }

    return msg;
}

static bool oprog_msg_furniture(Object *obj, Character *ch, const char *tagRoom, const char *tagChar)
{
    DLString msgRoom, msgChar;

    msgRoom = oprog_msg(obj, tagRoom);
    msgChar = oprog_msg(obj, tagChar);

    if (msgChar.empty() && msgRoom.empty())
        return false;

    if (!msgChar.empty())
        ch->pecho(msgChar.c_str(), obj, ch);

    if (!msgRoom.empty())
        ch->recho(POS_RESTING, msgRoom.c_str(), obj, ch);

    return true;
}

CMDRUNP(stand)
{
    Object *obj = 0;
    int furniture_flag = 0;
    
    if (argument[0] != '\0')
    {
        if (ch->position == POS_FIGHTING)
        {
            ch->pecho("Во время сражения есть дела поважнее.");
            return;
        }

        obj = get_obj_list(ch, argument, ch->in_room->contents);

        if (obj == 0)
        {
            ch->pecho("Ты не видишь этого здесь.");
            return;
        }

        furniture_flag = Item::furnitureFlags(obj);

        if (furniture_flag == 0 || (!IS_SET(furniture_flag, STAND_AT) &&
                                !IS_SET(furniture_flag, STAND_ON) &&
                                !IS_SET(furniture_flag, STAND_IN)))
        {
            ch->pecho("Ты не можешь стоять на этом.");
            return;
        }

        if (ch->on != obj && Item::countUsers(obj) >= Item::furnitureMaxPeople(obj))
        {
            oldact_p("На $o6 нет свободного места.", ch, obj, 0, TO_ROOM, POS_DEAD);
            return;
        }

        ch->on = obj;
    }

    switch (ch->position.getValue())
    {
    case POS_SLEEPING:
        if (IS_AFFECTED(ch, AFF_SLEEP))
        {
            ch->pecho("Ты не можешь проснуться!");
            return;
        }

        if (obj == 0)
        {
            ch->pecho("Ты просыпаешься и встаешь.");
            oldact("$c1 просыпается и встает.", ch, 0, 0, TO_ROOM);
            ch->on = 0;
        }
        else if (!oprog_msg_furniture(obj, ch, "msgWakeStandRoom", "msgWakeStandChar"))
        {
            if (IS_SET(furniture_flag, STAND_AT))
            {
                oldact_p("Ты просыпаешься и становишься возле $o2.", ch, obj, 0, TO_CHAR, POS_DEAD);
                oldact("$c1 просыпается и становится возле $o2.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, STAND_ON))
            {
                oldact_p("Ты просыпаешься и становишься на $o4.", ch, obj, 0, TO_CHAR, POS_DEAD);
                oldact("$c1 просыпается и становится на $o4.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact_p("Ты просыпаешься и становишься в $o4.", ch, obj, 0, TO_CHAR, POS_DEAD);
                oldact("$c1 просыпается и становится в $o4.", ch, obj, 0, TO_ROOM);
            }
        }

        ch->position = POS_STANDING;
        interpret_raw(ch, "look", "auto");
        break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
        if (obj == 0)
        {
            if (ch->position == POS_STANDING)
            {
                ch->pecho("Ты уже стоишь.");
            }
            else
            {
                ch->pecho("Ты встаешь.");
                oldact("$c1 встает.", ch, 0, 0, TO_ROOM);
            }
            ch->on = 0;
        }
        else if (!oprog_msg_furniture(obj, ch, "msgStandRoom", "msgStandChar"))
        {
            if (IS_SET(furniture_flag, STAND_AT))
            {
                oldact("Ты становишься возле $o2.", ch, obj, 0, TO_CHAR);
                oldact("$c1 становится возле $o2.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, STAND_ON))
            {
                oldact("Ты становишься на $o4.", ch, obj, 0, TO_CHAR);
                oldact("$c1 становится на $o4.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact("Ты становишься в $o4.", ch, obj, 0, TO_CHAR);
                oldact("$c1 становится в $o4.", ch, obj, 0, TO_ROOM);
            }
        }

        ch->position = POS_STANDING;
        break;

    case POS_FIGHTING:
        ch->pecho("Ты уже сражаешься!");
        ch->on = 0;
        break;
    }

    if (IS_HARA_KIRI(ch))
    {
        ch->pecho("Ты чувствуешь, как рана от харакири затягивается, и твое тело заживает.");
        REMOVE_BIT(ch->act, PLR_HARA_KIRI);
    }

}

CMDRUNP(rest)
{
    Object *obj = 0;
    Object *obj_current = ch->on;
    int furniture_flag = 0;

    if (ch->position == POS_FIGHTING)
    {
        ch->pecho("Во время сражения есть дела поважнее.");
        return;
    }

    if (MOUNTED(ch))
    {
        ch->pecho("Ты не можешь отдыхать, когда ты в седле.");
        return;
    }

    if (RIDDEN(ch))
    {
        ch->pecho("Ты не можешь отдыхать, когда ты оседлан%Gо||а.", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP))
    {
        ch->pecho("Ты спишь и не можешь проснуться.");
        return;
    }

    if (ch->death_ground_delay > 0 && ch->trap.isSet(TF_NO_MOVE))
    {
        ch->pecho("Тебе некогда отдыхать.");
        return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
        obj = get_obj_list(ch, argument, ch->in_room->contents);

        if (obj == 0)
        {
            ch->pecho("Ты не видишь этого здесь.");
            return;
        }
    }
    else
        obj = ch->on;

    if (obj != 0)
    {
        furniture_flag = Item::furnitureFlags(obj);

        if (furniture_flag == 0 || (!IS_SET(furniture_flag, REST_AT) &&
                                !IS_SET(furniture_flag, REST_ON) &&
                                !IS_SET(furniture_flag, REST_IN)))
        {
            ch->pecho("Ты не можешь отдыхать на этом.");
            return;
        }

        if (ch->on != obj && Item::countUsers(obj) >= Item::furnitureMaxPeople(obj))
        {
            oldact_p("На $o6 нет свободного места.", ch, obj, 0, TO_CHAR, POS_DEAD);
            return;
        }

        ch->on = obj;
    }

    switch (ch->position.getValue())
    {
    case POS_SLEEPING:
        if (DIGGED(ch))
        {
            ch->pecho("Ты просыпаешься.");
        }
        else if (obj == 0)
        {
            ch->pecho("Ты просыпаешься и садишься отдыхать.");
            oldact("$c1 просыпается и садится отдыхать.", ch, 0, 0, TO_ROOM);
        }
        else if (!oprog_msg_furniture(obj, ch, "msgWakeRestRoom", "msgWakeRestChar"))
        {
            if (IS_SET(furniture_flag, REST_AT))
            {
                oldact_p("Ты просыпаешься и садишься отдыхать возле $o2.",
                         ch, obj, 0, TO_CHAR, POS_SLEEPING);
                oldact("$c1 просыпается и садится отдыхать возле $o2.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, REST_ON))
            {
                oldact_p("Ты просыпаешься и садишься отдыхать на $o4.",
                         ch, obj, 0, TO_CHAR, POS_SLEEPING);
                oldact("$c1 просыпается и садится отдыхать на $o4.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact_p("Ты просыпаешься и садишься отдыхать в $o4.",
                         ch, obj, 0, TO_CHAR, POS_SLEEPING);
                oldact("$c1 просыпается и садится отдыхать в $o4.", ch, obj, 0, TO_ROOM);
            }
        }
        ch->position = POS_RESTING;
        break;

    case POS_RESTING:
        if (obj != 0 && obj != obj_current)
        {
            if (!oprog_msg_furniture(obj, ch, "msgSitRestRoom", "msgSitRestChar"))
            {
                if (IS_SET(furniture_flag, REST_AT))
                {
                    oldact("Ты пересаживаешься возле $o2 и отдыхаешь.", ch, obj, 0, TO_CHAR);
                    oldact("$c1 пересаживается возле $o2 и отдыхает.", ch, obj, 0, TO_ROOM);
                }
                else if (IS_SET(furniture_flag, REST_ON))
                {
                    oldact("Ты пересаживаешься на $o4 и отдыхаешь.", ch, obj, 0, TO_CHAR);
                    oldact("$c1 пересаживается на $o4 и отдыхает.", ch, obj, 0, TO_ROOM);
                }
                else
                {
                    oldact("Ты пересаживаешься отдыхать в $o4.", ch, obj, 0, TO_CHAR);
                    oldact("$c1 пересаживается отдыхать в $o4.", ch, obj, 0, TO_ROOM);
                }
            }
        }
        else
        {
            ch->pecho("Ты уже отдыхаешь.");
        }
        break;

    case POS_STANDING:
        if (obj == 0)
        {
            ch->pecho("Ты садишься отдыхать.");
            oldact("$c1 садится отдыхать.", ch, 0, 0, TO_ROOM);
        }
        else if (!oprog_msg_furniture(obj, ch, "msgSitRestRoom", "msgSitRestChar"))
        {
            if (IS_SET(furniture_flag, REST_AT))
            {
                oldact("Ты садишься возле $o2 и отдыхаешь.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится возле $o2 и отдыхает.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, REST_ON))
            {
                oldact("Ты садишься на $o4 и отдыхаешь.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится на $o4 и отдыхает.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact("Ты садишься отдыхать в $o4.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится отдыхать в $o4.", ch, obj, 0, TO_ROOM);
            }
        }
        ch->position = POS_RESTING;
        break;

    case POS_SITTING:
        if (obj == 0)
        {
            ch->pecho("Ты отдыхаешь.");
            oldact("$c1 отдыхает.", ch, 0, 0, TO_ROOM);
        }
        else if (!oprog_msg_furniture(obj, ch, "msgRestRoom", "msgRestChar"))
        {
            if (IS_SET(furniture_flag, REST_AT))
            {
                oldact("Ты отдыхаешь возле $o2.", ch, obj, 0, TO_CHAR);
                oldact("$c1 отдыхает возле $o2.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, REST_ON))
            {
                oldact("Ты отдыхаешь на $o6.", ch, obj, 0, TO_CHAR);
                oldact("$c1 отдыхает на $o6.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact("Ты отдыхаешь в $o6.", ch, obj, 0, TO_CHAR);
                oldact("$c1 отдыхает в $o6.", ch, obj, 0, TO_ROOM);
            }
        }
        ch->position = POS_RESTING;

        break;
    }

    if (IS_HARA_KIRI(ch))
    {
        ch->pecho("Ты чувствуешь, как рана от харакири затягивается, и твое тело заживает.");
        REMOVE_BIT(ch->act, PLR_HARA_KIRI);
    }

    return;
}

CMDRUNP(sit)
{
    Object *obj = 0;
    Object *obj_current = ch->on;
    int furniture_flag = 0;

    if (ch->position == POS_FIGHTING)
    {
        ch->pecho("Во время сражения есть дела поважнее.");
        return;
    }

    if (MOUNTED(ch))
    {
        ch->pecho("Ты не можешь сесть, когда ты в седле.");
        return;
    }

    if (RIDDEN(ch))
    {
        ch->pecho("Ты не можешь сесть, когда ты оседлан%Gо||а.", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP))
    {
        ch->pecho("Ты спишь и не можешь проснуться.");
        return;
    }

    if (ch->death_ground_delay > 0 && ch->trap.isSet(TF_NO_MOVE))
    {
        ch->pecho("Тебе не до отдыха!");
        return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
        obj = get_obj_list(ch, argument, ch->in_room->contents);

        if (obj == 0)
        {
            if (IS_AFFECTED(ch, AFF_SLEEP))
            {
                ch->pecho("Ты спишь и не можешь проснуться.");
                return;
            }

            ch->pecho("Ты не видишь этого здесь.");
            return;
        }
    }
    else
        obj = ch->on;

    if (obj != 0)
    {
        furniture_flag = Item::furnitureFlags(obj);

        if (furniture_flag == 0 || (!IS_SET(furniture_flag, SIT_AT) &&
                                !IS_SET(furniture_flag, SIT_ON) &&
                                !IS_SET(furniture_flag, SIT_IN)))
        {
            ch->pecho("Ты не можешь сесть на это.");
            return;
        }

        if (ch->on != obj && Item::countUsers(obj) >= Item::furnitureMaxPeople(obj))
        {
            oldact_p("На $o6 нет больше свободного места.", ch, obj, 0, TO_CHAR, POS_DEAD);
            return;
        }

        ch->on = obj;
    }

    switch (ch->position.getValue())
    {
    case POS_SLEEPING:
        if (obj == 0)
        {
            ch->pecho("Ты просыпаешься и садишься.");
            oldact("$c1 просыпается и садится.", ch, 0, 0, TO_ROOM);
        }
        else if (!oprog_msg_furniture(obj, ch, "msgWakeSitRoom", "msgWakeSitChar"))
        {
            if (IS_SET(furniture_flag, SIT_AT))
            {
                oldact_p("Ты просыпаешься и садишься возле $o2.", ch, obj, 0, TO_CHAR, POS_DEAD);
                oldact("$c1 просыпается и садится возле $o2.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, SIT_ON))
            {
                oldact_p("Ты просыпаешься и садишься на $o4.", ch, obj, 0, TO_CHAR, POS_DEAD);
                oldact("$c1 просыпается и садится на $o4.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact_p("Ты просыпаешься и садишься в $o4.", ch, obj, 0, TO_CHAR, POS_DEAD);
                oldact("$c1 просыпается и садится в $o4.", ch, obj, 0, TO_ROOM);
            }
        }

        ch->position = POS_SITTING;
        break;

    case POS_RESTING:
        if (obj == 0)
            ch->pecho("Ты прекращаешь отдых.");
        else if (!oprog_msg_furniture(obj, ch, "msgSitRoom", "msgSitChar"))
        {
            if (IS_SET(furniture_flag, SIT_AT))
            {
                oldact("Ты садишься возле $o2.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится возле $o2.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, SIT_ON))
            {
                oldact("Ты садишься на $o4.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится на $o4.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact("Ты садишься в $o4.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится в $o4.", ch, obj, 0, TO_ROOM);
            }
        }

        ch->position = POS_SITTING;
        break;

    case POS_SITTING:
        if (obj != 0 && obj != obj_current)
        {
            if (!oprog_msg_furniture(obj, ch, "msgSitRoom", "msgSitChar"))
            {
                if (IS_SET(furniture_flag, SIT_AT))
                {
                    oldact("Ты пересаживаешься возле $o2.", ch, obj, 0, TO_CHAR);
                    oldact("$c1 пересаживается возле $o2.", ch, obj, 0, TO_ROOM);
                }
                else if (IS_SET(furniture_flag, SIT_ON))
                {
                    oldact("Ты пересаживаешься на $o4.", ch, obj, 0, TO_CHAR);
                    oldact("$c1 пересаживается на $o4.", ch, obj, 0, TO_ROOM);
                }
                else
                {
                    oldact("Ты пересаживаешься в $o4.", ch, obj, 0, TO_CHAR);
                    oldact("$c1 пересаживается в $o4.", ch, obj, 0, TO_ROOM);
                }
            }
        }
        else
        {
            ch->pecho("Ты уже сидишь.");
        }
        break;

    case POS_STANDING:
        if (obj == 0)
        {
            ch->pecho("Ты садишься.");
            oldact("$c1 садится на землю.", ch, 0, 0, TO_ROOM);
        }
        else if (!oprog_msg_furniture(obj, ch, "msgSitRoom", "msgSitChar"))
        {
            if (IS_SET(furniture_flag, SIT_AT))
            {
                oldact("Ты садишься возле $o2.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится возле $o2.", ch, obj, 0, TO_ROOM);
            }
            else if (IS_SET(furniture_flag, SIT_ON))
            {
                oldact("Ты садишься на $o4.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится на $o4.", ch, obj, 0, TO_ROOM);
            }
            else
            {
                oldact("Ты садишься в $o4.", ch, obj, 0, TO_CHAR);
                oldact("$c1 садится в $o4.", ch, obj, 0, TO_ROOM);
            }
        }
        ch->position = POS_SITTING;
        break;
    }

    if (IS_HARA_KIRI(ch))
    {
        ch->pecho("Ты чувствуешь, как рана от харакири затягивается, и твое тело заживает.");
        REMOVE_BIT(ch->act, PLR_HARA_KIRI);
    }

    return;
}

CMDRUNP(sleep)
{
    Object *obj = 0;
    ostringstream toMe, toRoom;
    int furniture_flag = 0;

    if (MOUNTED(ch))
    {
        ch->pecho("Ты не можешь спать, когда ты в седле.");
        return;
    }

    if (RIDDEN(ch))
    {
        ch->pecho("Ты не можешь спать, когда ты оседлан%Gо||а.", ch);
        return;
    }

    if (ch->death_ground_delay > 0 && ch->trap.isSet(TF_NO_MOVE))
    {
        ch->pecho("Тебе не до сна!");
        return;
    }

    switch (ch->position.getValue())
    {
    case POS_SLEEPING:
        ch->pecho("Ты уже спишь.");
        return;

    case POS_FIGHTING:
        ch->pecho("Но ты же сражаешься!");
        return;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
        if (argument[0] == '\0' && ch->on == 0)
        {
            ch->position = POS_SLEEPING;

            toMe << "Ты засыпаешь";
            toRoom << "%1$^C1 засыпает";

            if (gsn_curl->getEffective(ch) > 1)
            {
                toMe << ", свернувшись клубочком";
                toRoom << ", свернувшись клубочком";
            }
        }
        else /* find an object and sleep on it */
        {
            if (argument[0] == '\0')
                obj = ch->on;
            else
                obj = get_obj_list(ch, argument, ch->in_room->contents);

            if (obj == 0)
            {
                ch->pecho("Ты не видишь этого здесь.");
                return;
            }

            furniture_flag = Item::furnitureFlags(obj);

            if (furniture_flag == 0 || (!IS_SET(furniture_flag, SLEEP_AT) &&
                                    !IS_SET(furniture_flag, SLEEP_ON) &&
                                    !IS_SET(furniture_flag, SLEEP_IN)))
            {
                ch->pecho("Ты не можешь спать на этом!");
                return;
            }

            if (ch->on != obj && Item::countUsers(obj) >=  Item::furnitureMaxPeople(obj))
            {
                oldact_p("На $o6 не осталось свободного места для тебя.",
                         ch, obj, 0, TO_CHAR, POS_DEAD);
                return;
            }

            ch->on = obj;
            ch->position = POS_SLEEPING;

            if (oprog_msg_furniture(obj, ch, "msgSleepRoom", "msgSleepChar"))
                return;

            toMe << "Ты ложишься спать ";
            toRoom << "%1$^C1 ложится спать ";

            if (IS_SET(furniture_flag, SLEEP_AT))
            {
                toMe << "возле %2$O2";
                toRoom << "возле %2$O2";
            }
            else if (IS_SET(furniture_flag, SLEEP_ON))
            {
                toMe << "на %2$O4";
                toRoom << "на %2$O4";
            }
            else
            {
                toMe << "в %2$O4";
                toRoom << "в %2$O4";
            }

            if (gsn_curl->getEffective(ch) > 1)
            {
                toMe << ", свернувшись клубочком";
                toRoom << ", свернувшись клубочком";
            }
        }
        break;
    }

    toMe << ".";
    toRoom << ".";
    ch->pecho(toMe.str().c_str(), ch, obj);
    ch->recho(POS_RESTING, toRoom.str().c_str(), ch, obj);
}

CMDRUNP(wake)
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        if (DIGGED(ch) && ch->position <= POS_SLEEPING)
            interpret_raw(ch, "rest", argument);
        else
        {
            undig(ch);
            interpret_raw(ch, "stand", argument);
        }

        return;
    }

    if ((victim = get_char_room(ch, arg)) == 0)
    {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (ch == victim)
    {
        ch->pecho("Ты не можешь разбудить сам{Sfа{Sx себя!");
        return;
    }

    if (IS_AWAKE(victim))
    {
        oldact("$C1 уже не спит.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim, AFF_SLEEP))
    {
        oldact("Ты не можешь разбудить $S от нездорового сна!", ch, 0, victim, TO_CHAR);
        return;
    }

    oldact("Ты будишь $C4.", ch, 0, victim, TO_CHAR);
    oldact("$c1 будит $C4.", ch, 0, victim, TO_NOTVICT);
    oldact_p("$c1 будит тебя.", ch, 0, victim, TO_VICT, POS_SLEEPING);

    interpret_raw(victim, "stand", argument);    
    mprog_wake(victim, ch);
}


