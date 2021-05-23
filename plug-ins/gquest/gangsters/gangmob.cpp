/* $Id: gangmob.cpp,v 1.1.2.5.4.9 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2003
 */
#include "clanreference.h"
#include "pcharacter.h"
#include "object.h"
#include "npcharacter.h"
#include "room.h"

#include "gangsters.h"
#include "gangstersinfo.h"
#include "gqchannel.h"
#include "gangmob.h"

#include "weapongenerator.h"
#include "exitsmovement.h"
#include "movetypes.h"
#include "handler.h"
#include "interp.h"
#include "fight.h"
#include "act.h"
#include "vnum.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

/*-------------------------------------------------------------------
 * GangMob
 *------------------------------------------------------------------*/
GangMob::GangMob( ) 
{
}

GangMob::~GangMob( ) 
{
}

void GangMob::config( int level ) 
{
    ch->setLevel( level );
    ch->hitroll = level * 2;
    ch->damroll = (short) ( level * 3 / 2 ); 
    ch->max_mana = ch->mana = level * 10;
    ch->max_move = ch->move = 1000;

    if (level < 30)
        ch->max_hit = level * 20 + 2 * number_fuzzy(level);
    else if (level < 60)
        ch->max_hit = level * 50 + 10 * number_fuzzy(level);
    else
        ch->max_hit = level * 100 + 20 * number_fuzzy(level);

    ch->hit = ch->max_hit;
    ch->armor[0] = -level * 5;
    ch->armor[1] = -number_fuzzy(level) * 6; 
    ch->armor[2] = -number_fuzzy(level) * 6; 
    ch->armor[3] = -number_fuzzy(level) * 6; 
    ch->saving_throw = -level / 2;

    if (chance(20)) {
        Object *weapon = create_object(get_obj_index(OBJ_VNUM_WEAPON_STUB), 0);
        weapon->level = min(level, LEVEL_MORTAL);
        obj_to_char(weapon, ch);

        if (chance(50))
            equip_char(ch, weapon, wear_wield);

        WeaponGenerator()
            .item(weapon)
//            .alignment(ch->alignment)
            .randomTier(3)
            .randomizeAll();
    }
} 

void GangMob::entry( ) 
{
    Character *mob, *ch_next;

    for (mob = ch->in_room->people; mob; mob = ch_next) {
        ch_next = mob->next_in_room;
        if (mob != ch)
            greet( mob );
    }
}

/*-------------------------------------------------------------------
 * GangMember
 *------------------------------------------------------------------*/
GangMember::GangMember( ) : confessed( false ), state( STAT_NORMAL )
{
}

bool GangMember::spec( ) 
{
    if (state == STAT_SLEEP && IS_AWAKE( ch ))
        state = STAT_NORMAL;
    
    if (ch->fighting) 
        state = STAT_FIGHTING;

    if (!IS_AWAKE( ch ))
        state = STAT_SLEEP;

    if (hasLastFought( )) {
        /* range-атака */
        if (state == STAT_NORMAL) {
            if (ch->hit < ch->max_hit / 4) {
                state = STAT_FLEE;
                fighting = lastFought;
                runaway( );
            }
            else        
                state = STAT_TRACKING;
        }
        if (state == STAT_FLEE) {
            clearLastFought( );
            memoryFought.clear( );
        }
    }

    if (state == STAT_FIGHTING) {
        return true;
    }

    if (state == STAT_NORMAL) {
        if (number_percent( ) < 20) {
            int door = number_door( );

            if (Wanderer::canWander( ch->in_room, door )) {
                switch (number_range( 1, 3 )) {
                case 1: move_char( ch, door, "normal" ); break;
                case 2: move_char( ch, door, "slink" ); break;
                case 3: move_char( ch, door, "crawl" ); break;
                }
                return true;
            }
            
            switch (number_range(1, 200)) {
            case 1: interpret(ch, "pound"); break;
            case 2: interpret(ch, "romeo"); break;
            case 3: interpret(ch, "buff"); break;
            case 4: interpret(ch, "camel self"); break;
            }
        }
        return true;
    }

    if (state == STAT_FLEE)
        runaway( );

    return true;
}

void GangMember::bribe( Character *briber, int gold, int silver ) 
{
    Gangsters *gquest = Gangsters::getThis( );
    int b = gquest->getMaxLevel( );
    int amount = gold * 100 + silver;

    if (!gquest->isLevelOK( briber ))
        return;

    if (state != STAT_NORMAL)
        return;
    
    if (confessed || amount / 100 < number_range( b, b + 50 ) || number_percent( ) < 50) {
        switch (number_range( 1, 3 )) {
        case 1: 
            oldact("$c1 громко вопит '{gПытаешься подкупить меня, щенок?!{x'", ch, 0, briber, TO_ROOM);
            interpret_raw(ch, "murder", briber->getNameC());
            break;
        case 2:
            oldact("$c1 произносит '{gПлакали твои денежки, $C1!{x'", ch, 0, briber, TO_ROOM);
            oldact("$c1 мерзко ухмыляется.", ch, 0, 0, TO_ROOM);
            break;
        case 3:
            for (int j = 0; j < 2; j++)
                for (int i = 0; i < DIR_SOMEWHERE; i++)
                    if (ch->in_room->exit[i]) {
                        if (!j)
                            oldact("$c1 потирает руки и удаляется в сторону ближайшей таверны.", ch, 0, 0, TO_ROOM);
                        move_char(ch, i, "slink");
                        break;
                    }
            
            break;
        }
    }
    else {
        oldact("$c1 тяжко вздыхает.", ch, 0, 0, TO_ROOM);
        oldact("$c1 произносит '{gУговорил, красноречивый.. Я открою тебе тайну!{x'", ch, 0, 0, TO_ROOM);
        oldact("$c1 что-то говорит на ухо $C3.", ch, 0, briber, TO_NOTVICT);        
        oldact_p("$c1 говорит тебе '{GВход в логово я видел около $t. Но больше мне ничего не известно.{x'",
               ch, gquest->lairHint( ).c_str( ), briber, TO_VICT, POS_RESTING);
        
        confessed = true;
    }
}

void GangMember::greet( Character *mob ) 
{
    if (Gangsters::getThis( )->getActor( mob ) != mob)
        return;
    
    if ((isLastFought( mob ) 
         || (!mob->is_npc( ) && mob->getName( ) == fighting.getValue( )))
        && ch->can_see( mob ) 
        && state == STAT_NORMAL) 
    {
        meetCnt = number_range( meetCnt, meetCnt + 1 );
        switch(number_range( 1, 3 )) {
        case 1:
            oldact("$c1 произносит '{g$C1, ну че ты за мной ходишь, влюбил$Gось|ся|ась?{x'", ch, 0, mob, TO_ROOM);
            break;
        case 2:
            oldact("$c1 произносит '{gО господи, это опять ты?{x'", ch, 0, 0, TO_ROOM);
            break;
        case 3:
            oldact("$c1 рычит '{g$C1, как же ты меня доста$Gло|л|ла!{x'", ch, 0, mob, TO_ROOM);
            interpret_raw(ch, "murder", mob->getNameC() );
            break;
        }
        return;
    }
    
    if (Gangsters::isPoliceman( mob ) 
            && canAttack( mob )
            && state != STAT_FLEE) 
    {
        if (ch->getRealLevel( ) > mob->getRealLevel( ) && ch->can_see( mob )) {
            switch (number_range(1, 2)) {
            case 1:
                oldact("$c1 произносит '{gЧертов фараон! Сейчас я тебе покажу!{x'", ch, 0, 0, TO_ROOM);
                break;
            case 2:
                oldact("$c1 произносит '{gМент поганый! Получай!{x'", ch, 0, 0, TO_ROOM);
                break;
            }
            multi_hit( ch, mob );
        } else {
            if (ch->can_see( mob )) {
                switch (number_range(1, 2)) {
                case 1:
                    oldact("$c1 бормочет '{gОппа.. менты.. менты{x'", ch, 0, 0, TO_ROOM);
                    break;
                case 2:
                    oldact("$c1 дрожит от страха перед $C5", ch, 0, mob, TO_ROOM);
                    break;
                }
            }
            if (mob->can_see(ch)) {
                yell( mob, "Смерть преступникам!");
                multi_hit( mob, ch );
            }
        }
        return;
    }
    
    if (state == STAT_NORMAL) {
               if (mob->is_npc( ) && number_percent( ) <= 10) {

            if (mob->getNPC()->pIndexData->area == ch->pIndexData->area) {
                if (ch->can_see( mob )) 
                    switch (number_range( 1, 5 )) {
                    case 1:
                        interpret_fmt(ch, "tip %s", mob->getNameC());
                        break;
                    case 2:
                        interpret_fmt(ch, "shake %s", mob->getNameC());
                        break;
                    }

                return;
            }
            
            if (mob->can_see( ch )) {
                yell( mob, "Это преступник! Хватайте его!" );

                if (number_percent( ) < 50 && canAttack( mob ) && ch->can_see( mob )) {
                    do_say( ch, "Да заткнись ты, зараза!" );
                    multi_hit( ch, mob );
                }
            }

            return;
        }
    
        if (!mob->is_npc( ) && mob->getSex( ) == SEX_FEMALE && ch->can_see( mob )) 
            switch (number_range( 1, 20 )) {
            case 1:
                interpret_fmt(ch, "pinch %s", mob->getNameC());
                interpret( ch, "smirk" );
                break;
            case 2:
                interpret_fmt(ch, "spank %s", mob->getNameC());
                break;
            case 3:
                interpret_fmt(ch, "leer %s", mob->getNameC());
                break;
            case 24:
                interpret_fmt(ch, "rose %s", mob->getNameC());
                break;
            case 25:
                interpret_fmt(ch, "bkiss %s", mob->getNameC());
                break;
            }
    }
}

void GangMember::fight( Character *victim ) 
{
    path.clear( );
    meetCnt = 0;
    state = STAT_FIGHTING;
    fighting = lastFought;

    BasicMobileDestiny::fight( victim );

    if (victim->hit < victim->max_hit / 4) {
        switch (number_range(1, 3)) {
        case 1: interpret(ch, "powertip"); break;
        }
    }
    
    if (victim->hit < victim->max_hit / 10) {
        switch (number_range(1, 3)) {
        case 1: interpret(ch, "anticipate"); break;
        }
    }
    
    if (ch->hit < ch->max_hit / 4) {
        switch (number_range(1, 7)) {
        case 1: 
            oldact("$c1 вопит '{gMamma mia!{x'", ch, 0, 0, TO_ROOM); 
            break;
        case 2:
            interpret(ch, "curse"); 
            break;
            case 3: 
            interpret_fmt(ch, "fatality %s", victim->getNameC());
            break;
        case 4: 
            interpret_fmt(ch, "cramp %s", victim->getNameC());
            break;
        case 5:
            oldact("$c1 вопит '{gПомогите! Хулиганы зрения лишают!{x'", ch, 0, 0, TO_ROOM); 
            break;
        }

        state = STAT_FLEE;
        clearLastFought( );
        memoryFought.clear( );
        runaway( );
    }
}

bool GangMember::death( Character *killer ) 
{
    std::basic_ostringstream<char> buf;
    Character * newmob;
    Gangsters *gquest = Gangsters::getThis( );
   
    if (!killer) {
        return false;
    }

    killer = gquest->getActor( killer );

    log("GangMember: killed by " << killer->getNameC());
    
    if (gquest->isLevelOK( killer )) {
        if (killer->in_room == ch->in_room && number_percent( ) < 10) {
            oldact_p("$c1 предсмертной хваткой цепляется за твою одежду.", 
                    ch, 0, killer, TO_VICT, POS_RESTING );

            if (IS_GOOD( killer ))
                oldact("$c1 хрипит '{gТолько сейчас я понял, как неправильно жил.. Я раскаиваюсь и перед смертью открою тебе тайну.{x'", ch, 0, killer, TO_VICT);
            else if (IS_EVIL( killer ))
                oldact("$c1 хрипит '{gБратан, передай от меня привет шефу!{x'", ch, 0, killer, TO_VICT);
            else 
                oldact("$c1 хрипит '{gЭх, все равно помирать.. Так слушай же..{x'", ch, 0, killer, TO_VICT);

            oldact_p("$c1 хрипит '{gВход в логово найдешь неподалеку от $t..{x'",
                   ch, gquest->lairHint( ).c_str( ), killer, TO_VICT, POS_RESTING );
        }
        
        gquest->rewardMobKiller( killer->getPC( ), ch );
        return false;
    }

            
    if (killer == ch) {
        if (ch->fighting) {
            oldact_p("$c1 хрипит '{gЛучше сдохнуть своей смертью, чем от руки такой собаки, как ты, $C2.{x'",
                  ch, 0, ch->fighting, TO_ROOM, POS_RESTING );
        }
        else {
            switch (number_range( 1, 3 )) {
            case 1: oldact("$c1 хрипит '{gМне крышка...{x'", ch, 0, 0, TO_ROOM); break;
            case 2: oldact("$c1 хрипит '{gКонец моим мучениям...{x'", ch, 0, 0, TO_ROOM); break;
            case 3: oldact("$c1 хрипит '{gЭта напасть доконала меня..{x'", ch, 0, 0, TO_ROOM); break;
            }
        }                
    } else {
        switch(number_range(1, 2)) {
        case 1: oldact("$c1 хрипит '{g$C1, тебе незнакомо понятие чести...{x'", ch, 0, killer, TO_ROOM); break;
        case 2: oldact("$c1 хрипит '{gЯ не должен был умереть от твоей руки, $C1..{x'", ch, 0, killer, TO_ROOM); break;    
        }
    }
    
    newmob = gquest->createMob( );
    char_to_room( newmob, gquest->pickRandomRoom( ) );
    
    if (eatKey( ))
        obj_to_char( gquest->createKey( ), newmob );
    
    return false;
}

void GangMember::yell( Character *mob, const DLString &msg ) 
{
    std::basic_ostringstream<char> buf;
    DLString name = mob->getNameP( '1' );
    name.upperFirstCharacter( );
        
    buf << name << " пронзительно кричит '{Y" << msg << "{x'" << endl;
    GQChannel::zecho( Gangsters::getThis( ), mob->in_room->area, buf.str( ) );
}

bool GangMember::eatKey( ) 
{
    Object *obj;

    for (obj = ch->carrying; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == GangstersInfo::getThis( )->vnumKey) {
            extract_obj( obj );
            oldact("$c1 выхватывает из кармана ключ и съедает его!", ch, 0, 0, TO_ROOM);
            return true;
        }
    
    return false;
}

bool GangMember::canAttack( Character *mob ) 
{
    if (mob->getRealLevel( ) <= Gangsters::getThis( )->getMaxLevel( ) + 3)
        return true;

    return false;
}

PCharacter * GangMember::getFightingRoom( Room *const pRoomIndex )
{
    for (Character *vch = pRoomIndex->people; vch; vch = vch->next_in_room) 
        if (!vch->is_npc( ) && vch->getName( ) == fighting)
            return vch->getPC( );

    return NULL;
}

PCharacter * GangMember::getFightingRoom( )
{
    return getFightingRoom( ch->in_room );
}

/*-------------------------------------------------------------------
 * GangMember: movement
 *------------------------------------------------------------------*/
bool GangMember::canEnter( Room *const pRoomIndex ) 
{
    if (!Gangsters::checkRoom( pRoomIndex ))
        return false;
    
    if (getFightingRoom( pRoomIndex ))
        return false;

    return true;
}

bool GangMember::canWander( Room *const room, EXTRA_EXIT_DATA *eexit )
{
    return false;
}

bool GangMember::canWander( Room *const room, Object *portal )
{
    return false;
}

bool GangMember::makeSomeSteps( int count )
{
    while (count-- > 0) {
        Room *old_room = ch->in_room;

        makeOneStep( );

        if (ch->in_room != old_room)  
            return !path.empty( );

        if (path.empty( )) 
            return false;
    }
    
    return true;
}

bool GangMember::runaway( )
{
    if (path.empty( ) && state == STAT_FLEE)
        pathWithDepth( ch->in_room, 4, 1000 );

    if (makeSomeSteps( 2 ))
        return true;

    if (getFightingRoom( )) {
        pathWithDepth( ch->in_room, 3, 500 );
        return true;
    }

    /* добегался */
    state = STAT_NORMAL;
    return false;
}

class GangFleeMovement : public ExitsMovement {
public:
    GangFleeMovement( GangMember::Pointer gang, int door )
                : ExitsMovement( gang->getChar( ), door, MOVETYPE_RUNNING )
    {
    }

    virtual bool checkPositionWalkman( )
    {
        return ch->position > POS_RESTING;
    }
};

int GangMember::moveOneStep( int door )
{
    return GangFleeMovement( this, door ).move( );
}

