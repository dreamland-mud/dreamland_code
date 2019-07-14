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
    std::basic_ostringstream<char> buf;    
    typedef map< int, vector<DLString> > GroupsMap;
    GroupsMap groups;
    NPCharacter *pet;
    OBJ_INDEX_DATA *pObjIndex;
    Object *obj;
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
        if (!pet || pet->in_room != victim->in_room)
            return;

        say_act( victim, ch, "Здравствуй, $c1." );
        say_act( victim, ch, "Если спросишь у меня 'что умеет мой питомец?', я расскажу, какими заклинаниями он владеет." );
        interpret_fmt( ch, "smile %s", victim->getNameP( ) );
        return;
    }
    
    if (!fWhat)
        return;

    if (!pet) {
        say_act( victim, ch, "$c1, у тебя нет домашнего животного." );
        return;
    }

    if (pet->in_room != victim->in_room) {
        say_act( victim, ch, "$c1, приводи его сюда - тогда и поговорим." );
        return;
    }
    
    if (!pet->getProfession( )->getFlags( pet ).isSet(PROF_CASTER)) {
        act("$C1 внимательно смотрит на $c4 и качает головой.", pet, 0, ch, TO_ALL);
        say_act( pet, ch, "Похоже, $c1 совершенно не способ$gно|ен|на к магии.." );
        return;
    }
    
    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill::Pointer skill = SkillManager::getThis( )->find( sn );
        Spell::Pointer spell = skill->getSpell( );

        if (spell 
            && spell->isCasted( ) 
            && skill->usable( pet, false )
            && spell->getSpellType( ) != SPELL_OFFENSIVE
            && skill->getGroup( )->available( pet ))
        {
            groups[skill->getGroup( )].push_back( skill->getNameFor( victim ) );
        }
    }
    
    if (groups.empty( )) {
        act("$C1 внимательно смотрит на $c4 и вздыхает.", pet, 0, ch, TO_ALL);
        say_act( pet, ch, "Похоже, что в голове у $x пусто.." );
        return;
    }
    
    act("$C1 тихо беседует с $c5 на странном языке.", pet, 0, ch, TO_ALL);
    buf << endl << "{G" << pet->getNameP( '1' ) << "{g "
        << "обладает познаниями в таких науках: " << endl << endl;
    
    for (GroupsMap::iterator i = groups.begin( ); i != groups.end( ); i++) {
        SkillGroup *group = skillGroupManager->find( i->first );
        DLString g = group->getShortDescr( );

        buf << "{G" << g.at( 0 ) << "{g" << g.substr( 1 ) << "{g:{x" << endl;

        for (vector<DLString>::iterator j = i->second.begin( ); j != i->second.end( ); ) {
            buf << *j;

            if (++j != i->second.end( ))
                buf << ", ";
        }

        buf << "{x" << endl << "\r\n";
    }
    
    pObjIndex = get_obj_index( 9606 );
    if (!pObjIndex) {
        say_act( victim, ch, "Извини, $c1, но у меня кончилась бумага." );
        interpret_fmt( ch, "snick" );
        return;
    }

    obj = create_object( pObjIndex, 0 );
    obj->addExtraDescr( obj->getName( ), buf.str( ) );
    obj_to_char( obj, victim );

    act( "$c1 делает запись на свитке пергамента и дает его $C3.", ch, 0, victim, TO_NOTVICT );
    act( "$c1 делает запись на свитке пергамента и дает его тебе.", ch, 0, victim, TO_VICT );
}

