/* $Id: patientbehavior.cpp,v 1.1.2.13.6.5 2008/04/04 21:29:04 rufina Exp $
 *
 * ruffina, 2003
 */

#include "patientbehavior.h"
#include "healquest.h"

#include "pcharacter.h"
#include "npcharacter.h"

void PatientBehavior::deadFromIdiot( PCMemoryInterface *pcm )
{
    if (!getQuest( ) || !quest->isComplete( ))
	pcm->getPlayer( )->pecho( "{YТы вылечи%Gло|л|ла пациента сразу от всех болезней >8){x",  pcm->getPlayer( ) );
    else
	pcm->getPlayer( )->println("{YСперва лечим, затем калечим?{x");
}

void PatientBehavior::deadFromSuicide( PCMemoryInterface *pcm )
{
    deadFromKill( pcm, ch );
}

void PatientBehavior::deadFromKill( PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( )) 
	pcm->getPlayer( )->println( "{YПациент отправился на тот свет без твоей помощи.{x" );
}

bool PatientBehavior::spell( Character *caster, int sn, bool before ) 
{
    if (!getQuest( ) || quest->isComplete( ))
	return false;

    if (before) {
	if (quest->maladies.hasKey( sn ) && ourHero( caster ))
	    healInfect( caster->getPC( ) );
	
	quest->maladies.setAttempts( sn );
    }
    else {
	if (ourHero( caster )) {
	    if (quest->maladies.checkSuccess( sn, ch )) {
		if (quest->isComplete( )) 
		    healComplete( caster->getPC( ) );
		else
		    healSuccess( caster->getPC( ) );
	    }
	}
	else {
	    if (quest->maladies.checkSuccessOther( sn, ch )) {
		if (quest->isComplete( ))
		    healOtherComplete( getHeroMemory( ) );	
		else
		    healOtherSuccess( getHeroMemory( ) );
	    }
	}
    }

    return false;
}

void PatientBehavior::healInfect( PCharacter *pch )
{
    pch->println( "{YЧто ж ты творишь! Тебя лечить позвали.{x" );
}

void PatientBehavior::healSuccess( PCharacter *pch )
{
    pch->pecho( "{YТы излечи%Gло|л|ла пациента от одного из недугов.{x", pch );
}

void PatientBehavior::healComplete( PCharacter *pch )
{
    pch->println("Твое задание {YВЫПОЛНЕНО{x!");
    pch->println("Вернись за вознаграждением к давшему тебе задание до того, как истечет время!");
}

void PatientBehavior::healOtherSuccess( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( )) {
	pcm->getPlayer( )->println( "{YКому-то удалось избавить твоего пациента от одного из недугов.{x" );
	pcm->getPlayer( )->println( "Это отрицательно скажется на общем размере вознаграждения." );
    }
}

void PatientBehavior::healOtherComplete( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( )) {
	pcm->getPlayer( )->println( "{YКто-то другой полностью излечил твоего пациента.{x" );
	pcm->getPlayer( )->println( "Вернись к квестору за утешительным призом." );
    }
}

bool PatientBehavior::canCancel( Character *caster )
{
    return MobQuestBehavior::canCancel( caster );
}

