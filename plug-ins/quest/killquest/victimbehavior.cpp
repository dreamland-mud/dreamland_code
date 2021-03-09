/* $Id: victimbehavior.cpp,v 1.1.2.11.6.2 2007/09/29 19:34:05 rufina Exp $
 *
 * ruffina, 2003
 */

#include "victimbehavior.h"

#include "pcharacter.h"

void VictimBehavior::deadFromHunter( PCMemoryInterface *pcm )
{
    pcm->getPlayer( )->pecho(
        "Твое задание {YВЫПОЛНЕНО{x!\n"
        "Вернись за вознаграждением к давшему тебе задание до того, как истечет время!" );
}

void VictimBehavior::deadFromSuicide( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( ))
        pcm->getPlayer( )->pecho( "{YЖертва умерла своей смертью.{x" );
}

void VictimBehavior::deadFromOther( PCMemoryInterface *pcm, Character *killer )
{
    killer->pecho("{YПоздравляю! Но это убийство было поручено другому.{x");

    if (pcm->isOnline( ))
        pcm->getPlayer( )->pecho( "{YКто-то другой выполнил порученное тебе задание.{x" );
}

void VictimBehavior::deadFromGroupMember( PCMemoryInterface *pcm, Character *killer )
{
    killer->pecho("{YПоздравляю! Ты выполнил%Gо||а задание своего согруппника.", killer);

    if (pcm->isOnline( ))
        pcm->getPlayer( )->pecho("{Y%1$^C1 выполнил%1$Gо||а твое задание.{x", killer);
}

void VictimBehavior::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ))
        buf << "{R[ЦЕЛЬ] {x";
}

