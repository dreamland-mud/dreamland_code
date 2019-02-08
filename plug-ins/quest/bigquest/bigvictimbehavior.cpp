/* $Id: victimbehavior.cpp,v 1.1.2.11.6.2 2007/09/29 19:34:05 rufina Exp $
 *
 * ruffina, 2003
 */

#include "bigvictimbehavior.h"

#include "pcharacter.h"

void BigVictimBehavior::deadFromHunter( PCMemoryInterface *pcm )
{
    pcm->getPlayer( )->println(
        "Твое задание {YВЫПОЛНЕНО{x!\n"
        "Вернись за вознаграждением к давшему тебе задание до того, как истечет время!" );
}

void BigVictimBehavior::deadFromSuicide( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( ))
        pcm->getPlayer( )->println( "{YЖертва умерла своей смертью.{x" );
}

void BigVictimBehavior::deadFromOther( PCMemoryInterface *pcm, Character *killer )
{
    killer->println("{YПоздравляю! Но это убийство было поручено другому.{x");

    if (pcm->isOnline( ))
        pcm->getPlayer( )->println( "{YКто-то другой выполнил порученное тебе задание.{x" );
}

void BigVictimBehavior::deadFromGroupMember( PCMemoryInterface *pcm, Character *killer )
{
    killer->pecho("{YПоздравляю! Ты выполнил%Gо||а задание своего согруппника.", killer);

    if (pcm->isOnline( ))
        pcm->getPlayer( )->pecho("{Y%1$^C1 выполнил%1$Gо||а твое задание.{x", killer);
}

void BigVictimBehavior::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ))
        buf << "{R[ЦЕЛЬ] {x";
}

