/* $Id: fixremort.cpp,v 1.1.2.9.4.5 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2004
 */

#include "fixremort.h"

#include "class.h"
#include "so.h"
#include "logstream.h"
#include "xmlvector.h"

#include "room.h"
#include "object.h"
#include "pcharacter.h"

#include "merc.h"
#include "descriptor.h"
#include "mercdb.h"
#include "handler.h"
#include "vnum.h"
#include "def.h"

#define log(x) LogStream::sendNotice() << "FR: " << x << endl

void FixRemortListener::run( int oldState, int newState, Descriptor *d )
{
    PCharacter *ch;
    
    if (newState != CON_PLAYING)
	return;
    
    if (!d->character || !( ch = d->character->getPC( ) ))
	return;
    
    if (ch->getRemorts( ).size( ) == 0)
	return;

    if (ch->getAttributes( ).isAvailable( "fixremort1" )) {
	fixOldRemort( ch );
	return;
    }

    if (ch->getAttributes( ).isAvailable( "fixremort2" )) {
	fixNewRemort( ch );
	return;
    }
}

void FixRemortListener::fixNewRemort( PCharacter *ch )
{
    Object *corpse;
    
    adjustBonuses( ch );

    log( "Adjust objects for " << ch->getName( ) << ", corpse in room " << ch->in_room->vnum );
    corpse = makeCorpse( ch );
    adjustObjects( ch, ch->carrying, corpse );

    if (corpse->contains == 0) {
	extract_obj( corpse );
	corpse = 0;
    }
    
    ch->getAttributes( ).eraseAttribute( "fixremort2" );
    ch->save( );
}

void FixRemortListener::fixOldRemort( PCharacter *ch )
{
    Object *corpse;

    adjustExp( ch );

    log( "Adjust objects for " << ch->getName( ) << ", corpse in room " << ch->in_room->vnum );
    corpse = makeCorpse( ch );
    adjustObjects( ch, ch->carrying, corpse );

    if (corpse->contains == 0) {
	extract_obj( corpse );
	corpse = 0;
    }
   
    ch->getAttributes( ).eraseAttribute( "fixremort1" );
    ch->getRemorts( ).resetBonuses( );
    
    ch->send_to( "\r\n{RВнимание!{x\r\n"
                 "Концепция ремортов изменена. "
		 "Читайте changes, help remort, help oldremort.\r\n" );
		
    if (corpse)
	ch->send_to( "Обратите внимание на мешок, который лежит у вас под ногами.\r\n"
	             "Подробнее о нем тоже см. help oldremort.\r\n" );
	
    ch->save( );
}

Object * FixRemortListener::makeCorpse( PCharacter *ch )
{
    Object *corpse;

    corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );
    corpse->setOwner( ch->getNameP( ) );
    corpse->from = str_dup( ch->getNameP( '2' ).c_str() );
    corpse->killer = str_dup( "Ruffina" );
    corpse->value[3] = ch->getHometown( )->getPit( );
    corpse->timer = 24 * 60;

    corpse->setName( "мешок sack" );
    corpse->fmtShortDescr( "меш|ок|ка|ку|ок|ком|ке с вещами %s", ch->getNameP( '2' ).c_str( ) );
    corpse->fmtDescription( "Мешок (sack) с вещами %s лежит здесь.", ch->getNameP( '2' ).c_str( ) );
		   
    obj_to_room( corpse, ch->in_room );
    return corpse;
}

void FixRemortListener::adjustObjects( PCharacter *ch, Object *list, Object *corpse )
{
    Object *obj, *obj_next;

    for (obj = list; obj; obj = obj_next) {
	obj_next = obj->next_content;
	
	if (obj->contains)
	    adjustObjects( ch, obj->contains, corpse );

	if ((obj->getOwner( ) && is_name( obj->getOwner( ), ch->getNameP( '7' ).c_str( ) ))
	    || (obj->extra_descr && obj->extra_descr->description 
	        && strstr( obj->extra_descr->description, ch->getNameP( ) ) ))
	{
	    obj->level = std::min( (short)obj->level, ch->getModifyLevel( ) );
	    log( "fix level: vnum " << obj->pIndexData->vnum << ", id " << obj->getID( ) );
	}
	else if (obj->mustDisappear( ch )) {
	    if (obj->carried_by)
		obj_from_char( obj );
	    else
		obj_from_obj( obj );

	    obj_to_obj( obj, corpse );
	    log( "to corpse: vnum " << obj->pIndexData->vnum << ", id " << obj->getID( ) );
	}
    }
}

void FixRemortListener::adjustExp( PCharacter *ch )
{
    unsigned int i;
    int oldExp, base, newExp;
    DLString origRace;
    DLString origProf;
    Remorts &remorts = ch->getRemorts( );
    
    origRace = ch->getRace( )->getName( );
    origProf = ch->getProfession( )->getName( );
    oldExp = ch->exp;

    for (i = 0; i < remorts.size( ); i++) {
	LifeData &life = remorts[i];
	
	ch->setRace( life.race.getValue( ) );
	ch->setProfession( life.classCh.getValue( ) );

	base = ch->getBaseExp( );
	log( "#"<< i << " " << 100 * base << " ** " << ch->getExpPerLevel( 100, i ) );
	oldExp += 100 * base;
    }
    
    ch->setProfession( origProf );
    ch->setRace( origRace );
    newExp = ch->getExpPerLevel( );

    for (i = 0; i < remorts.size( ) && newExp <= oldExp; i++) {
	LifeData &life = remorts[i];
	
	ch->setRace( life.race.getValue( ) );
	ch->setProfession( life.classCh.getValue( ) );

	newExp += ch->getExpPerLevel( 100, i );
    }

    ch->setProfession( origProf );
    ch->setRace( origRace );

    log( "Adjust exp for " 
	<< ch->getName( ) << ": old exp " << oldExp << ", border exp " << newExp);

    if (i + 1 < remorts.size( )) {
	log ( "erase lives from " << i+1 << " to " << remorts.size( ) - 1 );
	remorts.erase( remorts.begin( ) + i + 1, remorts.end( ) );
    }
    
    ch->exp = URANGE( ch->getExpPerLevel( ),
		      ch->exp.getValue( ),
		      ch->getExpPerLevel( ch->getRealLevel( ) + 1 ) - 10 );
}

void FixRemortListener::adjustBonuses( PCharacter *ch )
{
    Remorts &r = ch->getRemorts( );
    
    r.points += 10 * r.level;
    r.level = 0;

    for (int i = ch->getRealLevel( ); i > 1; i--) {
	ch->max_hit -= r.getHitPerLevel( i );
	ch->perm_hit -= r.getHitPerLevel( i );
	ch->max_mana -= r.getManaPerLevel( i );
	ch->perm_mana -= r.getManaPerLevel( i );
	ch->max_skill_points -= r.getSkillPointsPerLevel( i );
    }

    r.points     += 5 * r.hp / 50;
    r.hp          = 0;
    r.points     += 5 * r.mana / 100;
    r.mana        = 0;
    r.points     += 5 * r.skillPoints / 500;
    r.skillPoints = 0;

    for (int i = 0; i < stat_table.size; i++) {
	if (i == STAT_CON)
	    continue;

	r.points += 10 * r.stats[i];
	r.stats[i] = 0;
    }
}
