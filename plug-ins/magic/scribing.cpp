/* $Id: scribing.cpp,v 1.1.2.14.6.8 2008/05/27 21:30:04 rufina Exp $
 *
 * ruffina, 2004
 */

#include "scribing.h"
#include "skillreference.h"
#include "commandtemplate.h"

#include "skill.h"
#include "spell.h"
#include "pcharacter.h"
#include "object.h"
#include "dreamland.h"
#include "act.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(scribing);

/* 
 * SpellBook behavior 
 */

SpellBook::SpellBook( ) 
{
}

DLString SpellBook::extraDescription( Character *ch, const DLString &args )
{
    if (!is_name( args.c_str( ), obj->getName( ) ))
        return DLString::emptyString;

     
    std::basic_ostringstream<char> buf;
    toString( buf );
    return buf.str( );
}

void SpellBook::toString( ostringstream &buf )
{
    if (obj->value[0] == 0) 
	buf << "В этой книге нет ни одной страницы." << endl;
    else if (obj->value[1] == 0) 
	buf << "Эта книга пуста." << endl;
    else {
	buf << "Листая " << obj->getShortDescr( '4' ) << ", "
	    << "ты видишь на страницах такие формулы: " << endl << endl;
	
	for (SpellList::iterator i = spells.begin( ); i != spells.end( ); i++) {
	    buf << "Формула заклинания {W" << i->first << "{x, "
		<< "записанная с качеством {W" << i->second << "%{x" << endl;
	}
	
	if (obj->value[1] >= obj->value[0])
	    buf << endl << "Использованы все страницы книги." << endl;
    }

    buf << endl << "Максимальное качество записываемых заклинаний: "
        << obj->value[2] << "%." << endl;
    
}

bool SpellBook::examine( Character *ch ) 
{ 
    std::basic_ostringstream<char> buf;
    toString( buf );
    ch->send_to( buf );
    return true;
}


bool SpellBook::hasTrigger( const DLString &t )
{
    return (t == "examine");
}


/*
 * 'scribe' command
 */

/*
 * ITEM_SPELLBOOK:
 * v0 total
 * v1 used
 * v2 max quality
 */
CMDRUN( scribe )
{
    Object *book;
    SpellBook::Pointer behavior;
    Skill::Pointer skill;
    int chance, quality, sn;
    DLString arg1, arg2, arguments;
    
    chance = gsn_scribing->getEffective( ch );

    if (ch->is_npc( ) || chance <= 1) {
	ch->send_to( "Ты не владеешь искусством записи заклинаний.\r\n" );
	return;
    }

    if (ch->position != POS_RESTING && ch->position != POS_SITTING) {
	ch->send_to( "Сядь и сосредоточься.\r\n" );
	return;
    }
    
    /* parse args */
    arguments = constArguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments;
    arg2.stripWhiteSpace();
    
    if (arg1.empty( ) || arg2.empty( )) {
	ch->send_to( "Записать куда и что?\r\n" );
	return;
    }
    
    /* check book */
    if (!( book = get_obj_carry( ch, arg1 ) )) {
	ch->send_to( "У тебя нет такой книги.\r\n" );
	return;
    }
    if (book->item_type != ITEM_SPELLBOOK) {
	ch->send_to( "Это не книга заклинаний.\r\n" );
	return;
    }
    if (book->value[0] == 0) {
	ch->send_to( "Эта книга без страниц, даже в макулатуру не годится.\r\n" );
	return;
    }
    if (book->value[1] >= book->value[0]) {
	ch->send_to( "Все страницы книги уже заняты.\r\n" );
	return;
    }
    
    /* check spell */
    sn = SkillManager::getThis( )->unstrictLookup( arg2 );
    skill = SkillManager::getThis( )->find( sn );

    if (!skill) {
	ch->pecho( "Порывшись в голове, ты не смог%Gло|л|ла вспомнить такого заклинания.\r\n", ch );
	return;
    }
    if (!skill->getSpell( ) || !skill->getSpell( )->isCasted( )) {
	ch->send_to( "Это не заклинание. Используй мышечную память.:)\r\n" );
	return;
    }

    PCSkillData &data = ch->getPC( )->getSkillData( sn );
    if (data.learned <= 1) {
	ch->printf( "Ты не владеешь заклинанием '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
	return;
    }

    if (!skill->canForget( ch->getPC( ) )) {
	ch->printf( "Ты не сможешь выбросить из памяти знания о заклинании '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
	return;
    }
    
    if (data.learned > book->value[2]) {
        act("Качество этой книги не позволяет хранить полные знания о заклинании '$T'.", ch, 0, skill->getNameFor( ch ).c_str( ), TO_CHAR );
        return;
    }

    if (number_percent( ) > chance) {
        act("Ты пытаешься сконцентрировать свои знания в одну формулу, но она ускользает от тебя.", ch, 0, 0, TO_CHAR);
        act("$c1 о чем-то размышляет, уставившись в $o4.", ch, book, 0, TO_ROOM);
        gsn_scribing->improve( ch, false );
        ch->setWaitViolence( 1 );
        return;
    }

    /* calc quality */
    quality = data.learned;
    quality -= (number_percent( ) >= chance);
    quality = URANGE( 1, quality, book->value[2] );

    /* write */
    if (!book->behavior) {
	behavior = SpellBook::Pointer( NEW );
	behavior->setObj( book );
	book->behavior.setPointer( *behavior );
    }
    else if (!( behavior = book->behavior.getDynamicPointer<SpellBook>( ))) {
	ch->send_to( "Упс.. похоже, эта книга служит совсем другим целям.\r\n" );
	return;
    }

    book->value[1]++;
    behavior->spells[skill->getName( )] = quality;
    data.learned = 1;
    
    act( "Ты концентрируешь все свои знания о '$T' в одну формулу и наносишь ее на страницы $o2.", ch, book, skill->getNameFor( ch ).c_str( ), TO_CHAR );
    act( "$c1 что-то записывает в $o4.", ch, book, 0, TO_ROOM );
    act( "Ты чувствуешь пустоту в голове. Знания о '$t' стираются из твоей памяти.", ch, skill->getNameFor( ch ).c_str( ), 0, TO_CHAR ); 
    
    gsn_scribing->improve( ch, true );
    ch->setWaitViolence( 1 );
}

/*
 * 'memorize' command
 */
CMDRUN( memorize )
{
    std::basic_ostringstream<char> buf;
    Object *book;
    SpellBook::Pointer behavior;
    SpellBook::SpellList::iterator i;
    Skill::Pointer skill;
    int  chance, quality;
    DLString arg1, arg2, arguments;

    chance = gsn_scribing->getEffective( ch );

    if (ch->is_npc( ) || chance <= 1) {
	ch->send_to( "Ты не владеешь искусством чтения формулы заклинаний.\r\n" );
	return;
    }

    /* parse args */
    arguments = constArguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments;
    arg2.stripWhiteSpace( );
    
    if (arg1.empty( ) || arg2.empty( )) {
	ch->send_to( "Прочесть откуда и что?\r\n" );
	return;
    }

    /* check book */
    if (!( book = get_obj_carry( ch, arg1 ) )) {
	ch->send_to( "У тебя нет такой книги.\r\n" );
	return;
    }
    if (book->behavior) {
	behavior = book->behavior.getDynamicPointer<SpellBook>( );
    }
    if (!behavior || book->item_type != ITEM_SPELLBOOK) {
	ch->send_to( "Это не книга заклинаний.\r\n" );
	return;
    }

    /* find the spell */
    for (i = behavior->spells.begin( ); i != behavior->spells.end( ); i++)
	if (arg2.strPrefix( i->first )) {
	    skill = SkillManager::getThis( )->find( i->first );
	    
	    if (!skill) {
		ch->printf( "Похоже, что вместо формулы заклинания '%s' написан какой-то бред.", 
		            i->first.c_str( ) );
		return;
	    }
	    break;
	}

    if (!skill) {
	ch->send_to( "Ты не можешь найти формулу этого заклинания в книге.\r\n" );
	return;
    }

    /* can learn? */
    if (!skill->available( ch )) {
	ch->send_to( "Ты понятие не имеешь, как расшифровать эту формулу.\r\n" );
	return;
    }
    
    if (number_percent( ) > chance) {
        act("Ты пытаешься расшифровать формулу, но она ускользает от тебя.", ch, 0, 0, TO_CHAR);
        act("$c1 о чем-то размышляет, уставившись в $o4.", ch, book, 0, TO_ROOM);
        gsn_scribing->improve( ch, false );
        ch->setWaitViolence( 1 );
        return;
    }

//    if (!skill->canPractice( ch->getPC( ), buf )) {
//	ch->send_to( buf );
//	return;
//   }
    
    /* move from book to brains */
    quality = i->second;
    quality -= (number_percent( ) >= chance) * (number_percent( ) >= book->value[2]);
    ch->getPC( )->getSkillData( skill->getIndex( ) ).learned = std::max( 1, quality );
    behavior->spells.erase( i );
    book->value[1]--;
    
    act( "Ты расшифровываешь формулу '$t' и тихо шепчешь ее.", ch, skill->getNameFor( ch ).c_str( ), 0, TO_CHAR );
    act( "$c1 смотрит в $o4 и что-то шепчет.", ch, book, 0, TO_ROOM );
    act( "Формула слетает со страниц $o2 и прочно закрепляется в твоей памяти.", ch, book, 0, TO_CHAR );
    
    gsn_scribing->improve( ch, true );
    ch->setWaitViolence( 1 );
}


