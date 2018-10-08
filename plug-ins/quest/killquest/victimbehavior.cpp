/* $Id: victimbehavior.cpp,v 1.1.2.11.6.2 2007/09/29 19:34:05 rufina Exp $
 *
 * ruffina, 2003
 */

#include "victimbehavior.h"

#include "pcharacter.h"

void VictimBehavior::deadFromHunter( PCMemoryInterface *pcm )
{
    pcm->getPlayer( )->println(
	"Твое задание {YВЫПОЛНЕНО{x!\n"
	"Вернись за вознаграждением к давшему тебе задание до того, как истечет время!" );
}

void VictimBehavior::deadFromSuicide( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( ))
	pcm->getPlayer( )->println( "{YЖертва умерла своей смертью.{x" );
}

void VictimBehavior::deadFromOther( PCMemoryInterface *pcm, Character *killer )
{
    killer->println("{YПоздравляю! Но убить Его было поручено другому.{x");

    if (pcm->isOnline( ))
	pcm->getPlayer( )->println( "{YКто-то другой выполнил порученное тебе задание.{x" );
}

void VictimBehavior::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ))
	buf << "{R[ЦЕЛЬ] {x";
}

