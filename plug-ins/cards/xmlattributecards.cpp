/* $Id: xmlattributecards.cpp,v 1.1.2.7.6.1 2007/06/26 07:09:27 rufina Exp $
 *
 * ruffina, 2005
 */

#include "xmlattributecards.h"

#include "class.h"

#include "pcharacter.h"
#include "merc.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

/*--------------------------------------------------------------------------
 * XMLAttributeCards
 *--------------------------------------------------------------------------*/

const XMLAttributeCards::CardLevelFace XMLAttributeCards::levelFaces [] =
{
    { "шестерк|а|и|е|у|ой|е",        SEX_FEMALE },
    { "семерк|а|и|е|у|ой|е",         SEX_FEMALE },
    { "восьмерк|а|и|е|у|ой|е",       SEX_FEMALE },
    { "девятк|а|и|е|у|ой|е",         SEX_FEMALE },
    { "десятк|а|и|е|у|ой|е",         SEX_FEMALE },
    { "вал|ет|ьта|ьту|ьта|ьтом|ьте", SEX_MALE   },
    { "дам|а|ы|е|у|ой|е",            SEX_FEMALE },
    { "корол|ь|я|ю|я|ем|е",          SEX_MALE   },
    { "туз||а|у|а|ом|е",             SEX_MALE   },
};

const XMLAttributeCards::CardSuitFace XMLAttributeCards::suitFaces [] =
{
    { "черв|а|ей|ей|ей|ей|ей", "червов|ый|ого|ому|ого|ым|ом", "червов|ая|ой|ой|ую|ой|ой" },
    { "пик|а|||||",    "пиков|ый|ого|ому|ого|ым|ом",  "пиков|ая|ой|ой|ую|ой|ой"  },
    { "треф|а|||||",   "трефов|ый|ого|ому|ого|ым|ом", "трефов|ая|ой|ой|ую|ой|ой" },
    { "бубен",  "бубнов|ый|ого|ому|ого|ым|ом", "бубнов|ая|ой|ой|ую|ой|ой" },
};

int XMLAttributeCards::getMaxLevel( )
{
    return sizeof(levelFaces) / sizeof(*levelFaces) - 1;
}

int XMLAttributeCards::getRandomSuit( ) 
{
    return number_bits( 2 );
}

XMLAttributeCards::XMLAttributeCards( ) : level( -1 ), suit( -1 )
{
}

XMLAttributeCards::~XMLAttributeCards( ) 
{
}

bool XMLAttributeCards::isTrump( ) const
{
    return getSuit( ) == getTrump( );
}
    
DLString XMLAttributeCards::getFace( char needcase ) const
{
    DLString face, suit;
    
    if (getLevelFace( ).gender == SEX_MALE)
        suit = russian_case( getSuitFace( ).male, needcase );
    else
        suit = russian_case( getSuitFace( ).female, needcase );
    
    face = suit + " " + russian_case( getLevelFace( ).name, needcase );
    return face;
}

bool XMLAttributeCards::handle( const DeathArguments &args )
{
    PCharacter *pkiller;
    Pointer card;
    
    if (!args.killer || args.killer->is_npc( ) || args.killer == args.pch)
        return false;

    pkiller = args.killer->getPC( );

    card = pkiller->getAttributes( ).getAttr<XMLAttributeCards>( "cards" );
    
    if (card->level < level || isTrump( )) {
        if (card->level < getMaxLevel( )) {
            card->level++;

            if (card->suit < 0)
                card->suit = getRandomSuit( );

            act( "{cТы уби$gло|л|ла $t из Колоды.{x", pkiller, getFace( '4' ).c_str( ), 0, TO_CHAR );
            act( "{cТеперь ты $t!{x", pkiller, card->getFace( '1' ).c_str( ), 0, TO_CHAR );
        }
    
        level--;

        if (level >= 0)
            args.pch->printf( "{cТы становишься %s.{x\r\n", getFace( '5' ).c_str( ) );
        else {
            args.pch->send_to( "{cТы выбываешь из колоды!{x\r\n" );
            args.pch->getAttributes( ).eraseAttribute( "cards" );
            return false;
        }
    }

    return false;
}

int XMLAttributeCards::getTrump( ) 
{
    static const int PRIME1 = 37;
    static const int PRIME2 = 1048583;
    unsigned int h;

    h = time_info.day;
    h = h * PRIME1 ^ time_info.year;
    h = h * PRIME1 ^ time_info.month;
    h %= PRIME2;
    h %= 4;

    return h;
}

