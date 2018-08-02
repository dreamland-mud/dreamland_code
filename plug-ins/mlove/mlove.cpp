/* $Id: mlove.cpp,v 1.1.2.14.6.5 2009/11/04 03:24:33 rufina Exp $
 * ruffina, 2003
 * ideas and beta-testing with Filths >8) 
 */

#include "logstream.h"
#include "commandtemplate.h"

#include "xmllovers.h"
#include "xmlattributelovers.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

#define MLOVE_DAZE(ch)	(ch)->wait = std::max((ch)->wait, 25);

static bool mprog_makelove( Character *ch, Character *victim )
{
    FENIA_CALL( ch, "MakeLove", "C", victim );
    FENIA_NDX_CALL( ch->getNPC( ), "MakeLove", "CC", ch, victim );
    return false;
}

CMDRUN( mlove )
{
	DLString arguments = constArguments;
	DLString arg;
	std::basic_ostringstream<char> str;
	Character *victim;

	if (IS_AFFECTED(ch,AFF_CHARM)) {
	    act_p("... но сердцу не прикажешь.", ch, 0, 0, TO_CHAR, POS_RESTING);  
	    act_p("$c1 ухмыляется - сердцу не прикажешь.", ch, 0, ch->master, TO_VICT, POS_RESTING);
	    return;
	}

	if (arguments.empty( )) {
	    if (ch->getSex( ) == SEX_MALE)
		act_p("Ты никак не можешь определиться: куда совать жетон?", ch, 0, 0, TO_CHAR, POS_RESTING);
	    else 
		act_p("Куда пойти, куда податься.. кого найти, кому отдаться?", ch, 0, 0, TO_CHAR, POS_RESTING);

	    act_p("$c1 гоняется с похотливым видом за всеми в комнате..берегись!", ch, 0, 0, TO_ROOM, POS_RESTING);
	    return;
	}
	
	arg = arguments.getOneArgument( );

	if ( (victim = get_char_room(ch, arg.c_str())) == 0 ) {
	    ch->send_to("Объект твоей страсти куда-то подевался.\n\r");
	    return;
	}

	if (ch == victim) {
	    ch->move -= ch->move / 4;
	    ch->mana -= ch->mana / 4;

	    ch->send_to("Да! Ты любишь себя! Еще, еще..!\n\r");
	    act_p("Страсть $c1 к само$gму|му|й себе пользуется полной взаимностью.", ch, 0, 0, TO_ROOM, POS_RESTING);
	    MLOVE_DAZE(ch);
	    return;
	}

	if (ch->position == POS_FIGHTING) {
	    if (ch->getSex( ) == SEX_MALE)
		act_p("Быстро спрячь, пока не отрезали!", ch, 0, 0, TO_CHAR, POS_RESTING);
	    else 
		act_p("Эй, не отвлекайся!", ch, 0, 0, TO_CHAR, POS_RESTING);
	    
	    act_p("$c1 торжественно произносит: '{gMake love, not war!{x'", ch, 0, 0, TO_ROOM, POS_RESTING);
	    return;
	}

	if (victim->position <= POS_STUNNED) {
	    act_p("$M сейчас 'как-то так'.. извини.", ch, 0, victim, TO_CHAR, POS_RESTING);
	    return;
	}
	else if (victim->position == POS_SLEEPING) {
	    act_p("Может, стоит $S для начала разбудить?", ch, 0, victim, TO_CHAR, POS_RESTING);
	    act_p("$c1 вертится вокруг $C2 и так, и эдак, но что-то $s смущает. Наверное, $S храп?", ch, 0, victim, TO_NOTVICT, POS_RESTING);
	    return;
	}
	else if (victim->position == POS_FIGHTING) {
	    act_p("$M сейчас совсем не до тебя.", ch, 0, victim, TO_CHAR, POS_RESTING);
	    return;
	}
	
	if (mprog_makelove( ch, victim ))
	    return;

	if (ch->is_npc()) {
	    ch->send_to("Тебе нельзя.\n\r");
	    return;
	}
	
	if (!victim->is_npc()) {
	    XMLAttributeLovers::Pointer pointer;
	    
	    XMLAttributes* attributes = &victim->getPC( )->getAttributes( );
	    XMLAttributes::iterator ipos = attributes->find( "lovers" );	
	
	    if (ipos != attributes->end( ) &&
		!(pointer = ipos->second.getDynamicPointer<XMLAttributeLovers>( ))->lovers.empty( ))
	    {
		if (pointer->lovers.isPresent( ch->getName() ) ) {
		    ch->mana -= ch->mana / 4;
		    victim->mana -= victim->mana / 4;
		    ch->move -= ch->move / 4;
		    victim->move -= victim->move / 4;
 
		    act_p("Ты срываешь с $C2 одежду и страстно занимаешься с $Y любовью.", ch, 0, victim, TO_CHAR, POS_RESTING);
		    act_p("$c1 срывает с тебя одежду и страстно занимается с тобой любовью. Ах, да! Еще, еще!", ch, 0, victim, TO_VICT, POS_RESTING);
		    act_p("$c1 срывает с $C2 одежду и страстно занимается с $Y любовью.", ch, 0, victim, TO_NOTVICT, POS_RESTING);
		    
		    MLOVE_DAZE(victim);
		    MLOVE_DAZE(ch);
		    
		    return;
		}
	    }
	}
	
	act_p("О$Gно|н|на тебя не хочет.", ch, 0, victim, TO_CHAR, POS_RESTING);
	act_p("$c1 пытается добиться от тебя взаимности, но ты отвергаешь $s.", ch, 0, victim, TO_VICT, POS_RESTING);
	act_p("$c1 пытается добиться от $C2 взаимности, но $C1 отвергает $s.", ch, 0, victim, TO_NOTVICT, POS_RESTING);
}


