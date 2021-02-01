/* $Id: hierophant.cpp,v 1.1.2.11.6.13 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sstream>
#include <map>
#include <vector>

#include "hierophant.h"
#include "profflags.h"

#include "class.h"
#include "regexp.h"

#include "skill.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "spell.h"
#include "damageflags.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"

#include "arg_utils.h"
#include "handler.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

using namespace std;

Hierophant::Hierophant( ) 
{
}

void Hierophant::speech( Character *victim, const char *msg ) 
{
    tell( victim, msg );
}

void Hierophant::tell( Character *victim, const char *speech ) 
{
    NPCharacter *pet;
    bool fHello = false, fWhat = false;
    
    if (victim->is_npc( ))
        return;
    
    pet = victim->getPC( )->pet;

    if (arg_has_oneof( speech, "hello", "hi", "привет", "здравствуй" )) 
        fHello = true;
    else if ((arg_contains_someof( speech, "pet питомец животное любимец пет" )
                || (pet && arg_contains_someof( speech, pet->getName( ).c_str( ) )))
             && arg_contains_someof( speech, "заклинаниями владеет колдовать умеет" )) 
        fWhat = true;
    else 
        return;

    if (fHello) {
        say_act( victim, ch, "Здравствуй, $c1." );
        say_act( victim, ch, "Если вдруг ты при%1$Gшло|шел|шла поговорить о питомцах, то знай: отныне я на пенсии! Лучше почитай {y{hh1005{lR? рапорт{lE? report{x и {y{hh1091{lR? приказать{lE? order{x" );
        interpret_fmt( ch, "smile %s", victim->getNameP( ) );
        return;
    }
    
    if (!fWhat)
        return;

        say_act( victim, ch, "Мил%1$Gое|ой|ая мо%1$Gе|й|я, я же на пенсии! Лучше почитай {y{hh1005{lR? рапорт{lE? report{x и {y{hh1091{lR? приказать{lE? order{x" );
        return;    
  
}

