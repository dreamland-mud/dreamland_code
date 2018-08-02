/* $Id: cremort.cpp,v 1.1.2.18.4.8 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "commandtemplate.h"
#include "commonattributes.h"
#include "playerattributes.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "race.h"
#include "room.h"
#include "remortdata.h"

#include "dreamland.h"
#include "merc.h"
#include "descriptor.h"
#include "nannyhandler.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

CLAN(flowers);
bool password_check( PCMemoryInterface *pci, const DLString &plainText );

const	short	CONVERT_PRACTICE_QP = 3;

CMDRUN( remort )
{
    Descriptor *d;
    PCharacter *pch, *new_ch;
    Remorts remorts;
    LifeData thisLife;
    XMLAttributes newAttributes;
    DLString argument = constArguments;

    pch = ch->getPC( );

    if (!pch || !( d = pch->desc )) 
	return;
    
    /*
     * can remort?
     */
    if (pch->age.getTrueYears( ) < 25) {
	pch->pecho("Ты слишком моло%1$Gдо|д|да, чтобы умирать.", pch);
	return;
    }
    
    if (IS_SET(pch->comm, COMM_NOTELL|COMM_NOEMOTE) 
	|| pch->in_room->vnum == ROOM_VNUM_JAIL 
	|| pch->in_room->vnum == 10 
	|| pch->curse < 100)
    {
	pch->pecho("Ты еще не искупи%1$Gло|л|ла провинности ЭТОЙ жизни.", pch);
	return;
    }
    
    if (argument.empty( )) {
	pch->println("Если хочешь начать новую жизнь, набери remort <password>");
	return;
    }
    
    if (!password_check( pch, argument )) {
	pch->println("Неверный пароль.");
	return;
    }

    if (auction->item && ((pch == auction->buyer) || (pch == auction->seller))) {
	pch->println("Подожди пока вещь, выставленная на аукцион, будет продана или возвращена.");
	return;
    }

    if (!PCharacterManager::pfRemort(pch)) {
	pch->println("Не удалось сохранить твой профайл: сообщи Богам!");
	return;
    }
    
    new_ch = PCharacterManager::getPCharacter( );
    
    /* setup inherited fields */
    new_ch->setName( pch->getName( ) );
    new_ch->setPassword( pch->getPassword( ) );
    new_ch->setRussianName( pch->getRussianName( ).getFullForm( ) );
    new_ch->setSex( pch->getSex( ) );
    new_ch->prompt = pch->prompt;
    new_ch->batle_prompt = pch->batle_prompt;
    new_ch->add_comm = COMM_NOTELNET | COMM_NOIAC; /* XXX save some options from last remort */
    new_ch->act = PLR_COLOR;

    if (pch->getClan( ) != clan_flowers) {
	new_ch->questpoints = pch->questpoints +
		              pch->practice * CONVERT_PRACTICE_QP +
		              pch->train * CONVERT_PRACTICE_QP * 10;
	new_ch->bank_s = pch->bank_s;
	new_ch->bank_g = pch->bank_g;
    }
    
    /* remember this life and reset remort bonuses */
    thisLife.classCh = pch->getProfession( )->getName( );
    thisLife.race = pch->getRace( )->getName( );
    thisLife.time = pch->age.getTrueHours( );
    thisLife.bonus = (pch->getClan( ) != clan_flowers);

    remorts = pch->getRemorts( );
    remorts.push_back( thisLife );
    remorts.resetBonuses( );
    new_ch->setRemorts( remorts );
    
    /* notify attributes */
    pch->getAttributes( ).handleEvent( RemortArguments( pch, &newAttributes ) );
    newAttributes.getAttr<XMLEmptyAttribute>( "remorting" );
    new_ch->setAttributes( newAttributes );
    
    /* good bye ... */
    PCharacterManager::remove( pch->getName( ) );
    extract_char( pch );

    /* ...and hello */
    PCharacterManager::update( new_ch );
    new_ch->save( );

    /* as always */
    d->associate( new_ch );
    NannyHandler::initRemort( d );
}

