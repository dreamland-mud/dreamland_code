/* $Id: questgirth.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */

#include "questgirth.h"
#include "class.h"
#include "profflags.h"
#include "affect.h"
#include "pcharacter.h"
#include "object.h"
#include "act.h"
#include "merc.h"
#include "loadsave.h"
#include "def.h"

void QuestGirth::wear( Character *ch ) 
{
    ch->send_to( "{CТвой пояс ярко вспыхивает.{x\r\n" );
}


void QuestGirth::equip( Character *ch ) 
{
    obj->level = ch->getRealLevel( );
    
    if (!obj->affected.empty()) {
        // Updated existing affects to match player level.
        for (auto &paf: obj->affected)
            addAffect(ch, paf);
    }
    else {
        // Add affects for the first time.
        static const int applies [] = {
            APPLY_INT, APPLY_WIS, APPLY_CON, APPLY_DEX, APPLY_STR, 
            APPLY_AC, APPLY_HIT, APPLY_MANA, APPLY_MOVE,
            APPLY_HITROLL, APPLY_DAMROLL, -1
        };

        Affect af;

        af.where = TO_OBJECT;
        af.type  = -1;
        af.duration = -1;
        af.bitvector = 0;

        for (int i = 0; applies[i] != -1; i++) {
            af.location = applies[i];
            addAffect(ch, &af);
            affect_to_obj(obj, &af);
        }
    }
}

void QuestGirth::addAffect( Character *ch, Affect *paf ) 
{
    short level = ch->getModifyLevel();
    bool caster = ch->getProfession( )->getFlags( ).isSet(PROF_CASTER);
    bool fighter = !caster;
    bool evil = IS_EVIL(ch);
    bool good = IS_GOOD(ch);
    bool neutral = IS_NEUTRAL(ch);

    int mod = 0;

    switch( paf->location ) {
        case APPLY_DAMROLL:
            if (fighter) { // battle class
                if (evil)
                    mod = level / 10 + 3;
                else if (neutral)
                    mod = level / 10 + 2;
                else
                    mod = level / 10 + 1;
              
            } else { // caster class
                if (evil)
                    mod = level / 10;
                else if (neutral)
                    mod = level / 12;
                else
                    mod = level / 14;            
            }
            break;

        case APPLY_HITROLL:
            if (fighter) { // battle class
                if (evil)
                    mod = level / 10;
                else if (neutral)
                    mod = level / 10 + 2;
                else
                    mod = level / 10 + 3;
              
            } else { // caster class
                if (evil)
                    mod = level / 14;
                else if (neutral)
                    mod = level / 12;
                else
                    mod = level / 10;            
            }
            break;


        case APPLY_HIT:
            if (fighter) { // battle class
                if (evil)
                    mod = level / 2;
                else if (neutral)
                    mod = 3 * level / 4;
                else
                    mod = level;
              
            } else { // caster class
                if (evil)
                    mod = 3 * level / 2;
                else if (neutral)
                    mod = level * 2;
                else
                    mod = level * 3;            
            }
            break;

        case APPLY_MANA:
            if (fighter)  // battle class
                mod = level;
            else // caster class
                mod = level * 5;
            break;

        case APPLY_MOVE:
            mod = level;
            break;

        case APPLY_AC:
            // Old formula, doesn't make much difference.
            mod = IS_GOOD( ch ) ? -( level * 3 ) :
                              IS_EVIL( ch ) ? -( level * 3 ) / 2 : -( level * 2 );
            break;

        case APPLY_INT:
            if (caster) {
                if (evil)
                    mod = 2;
                else if (neutral)
                    mod = 1;
            }
            break;

        case APPLY_WIS:
            if (caster) {
                if (neutral)
                    mod = 1;
                else if (good)
                    mod = 2;
            }
            break;

        case APPLY_DEX:
            if (fighter) {
                if (neutral)
                    mod = 2;
            }
            break;

        case APPLY_STR:
            if (fighter) {
                if (evil)
                    mod = 2;
            }
            break;

        case APPLY_CON:
            if (fighter) { 
                if (evil)
                    mod = 1;
                else if (neutral)
                    mod = 1;
                else // good fighter
                    mod = 3;

            } else { // all casters
                mod = 1; 
            }
            break;

        default:
            return;
    }

    paf->level = level;
    paf->modifier = mod;
}

