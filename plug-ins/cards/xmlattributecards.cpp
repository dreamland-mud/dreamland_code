/* $Id: xmlattributecards.cpp,v 1.1.2.7.6.1 2007/06/26 07:09:27 rufina Exp $
 *
 * ruffina, 2005
 */

#include "xmlattributecards.h"

#include "class.h"

#include "pcharacter.h"
#include "merc.h"
#include "act.h"

#include "def.h"
#include "l10n.h"

/*--------------------------------------------------------------------------
 * XMLAttributeCards
 *--------------------------------------------------------------------------*/

const XMLAttributeCards::CardLevelFace XMLAttributeCards::levelFaces [] =
{
    { "шестерк|а|и|е|у|ой|е",        SEX_FEMALE, "Six",   "шістк|а|и|і|у|ою|і"   },
    { "семерк|а|и|е|у|ой|е",         SEX_FEMALE, "Seven", "сімк|а|и|і|у|ою|і"    },
    { "восьмерк|а|и|е|у|ой|е",       SEX_FEMALE, "Eight", "вісімк|а|и|і|у|ою|і"  },
    { "девятк|а|и|е|у|ой|е",         SEX_FEMALE, "Nine",  "дев'ятк|а|и|і|у|ою|і" },
    { "десятк|а|и|е|у|ой|е",         SEX_FEMALE, "Ten",   "десятк|а|и|і|у|ою|і"  },
    { "вал|ет|ьта|ьту|ьта|ьтом|ьте", SEX_MALE,   "Jack",  "валет||а|у|а|ом|і"    },
    { "дам|а|ы|е|у|ой|е",            SEX_FEMALE, "Queen", "дам|а|и|і|у|ою|і"     },
    { "корол|ь|я|ю|я|ем|е",          SEX_MALE,   "King",  "корол|ь|я|ю|я|ем|і"   },
    { "туз||а|у|а|ом|е",             SEX_MALE,   "Ace",   "туз||а|у|а|ом|і"      },
};

const XMLAttributeCards::CardSuitFace XMLAttributeCards::suitFaces [] =
{
    { "черв|а|ей|ей|ей|ей|ей", "червов|ый|ого|ому|ого|ым|ом", "червов|ая|ой|ой|ую|ой|ой",
      "Hearts",   "черв|а|||||",          "червов|ий|ого|ому|ого|им|ому", "червов|а|ої|ій|у|ою|ій" },
    { "пик|а|||||",    "пиков|ый|ого|ому|ого|ым|ом",  "пиков|ая|ой|ой|ую|ой|ой",
      "Spades",   "пік|а|||||",           "піков|ий|ого|ому|ого|им|ому",  "піков|а|ої|ій|у|ою|ій" },
    { "треф|а|||||",   "трефов|ый|ого|ому|ого|ым|ом", "трефов|ая|ой|ой|ую|ой|ой",
      "Clubs",    "треф|а|||||",          "трефов|ий|ого|ому|ого|им|ому", "трефов|а|ої|ій|у|ою|ій" },
    { "бубен",  "бубнов|ый|ого|ому|ого|ым|ом", "бубнов|ая|ой|ой|ую|ой|ой",
      "Diamonds", "буб|на|он|он|он|он|он", "бубнов|ий|ого|ому|ого|им|ому", "бубнов|а|ої|ій|у|ою|ій" },
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
    return getFace( LANG_RU, needcase );
}

DLString XMLAttributeCards::getFace( lang_t lang, char needcase ) const
{
    const CardLevelFace &lf = getLevelFace( );
    const CardSuitFace &sf = getSuitFace( );
    bool male = (lf.gender == SEX_MALE);

    if (lang == LANG_EN)
        return DLString( lf.en ) + " of " + sf.en;

    if (lang == LANG_UA)
        return russian_case( male ? sf.maleUa : sf.femaleUa, needcase )
               + " " + russian_case( lf.ua, needcase );

    return russian_case( male ? sf.male : sf.female, needcase )
           + " " + russian_case( lf.name, needcase );
}

/* Suit as an undeclined noun pad ("треф|а|||||", "Clubs"), for $n/%N or russian_case. */
const char * XMLAttributeCards::getSuitNoun( int suit, lang_t lang )
{
    const CardSuitFace &sf = suitFaces[suit];

    if (lang == LANG_EN)
        return sf.en;

    return lang == LANG_UA ? sf.mltUa : sf.mlt;
}

bool XMLAttributeCards::handle( const DeathArguments &args )
{
    PCharacter *pkiller;
    Pointer card;
    
    if (!args.killer || args.killer->is_npc( ) || args.killer->is_immortal( ) || args.killer == args.pch)
        return false;

    pkiller = args.killer->getPC( );

    card = pkiller->getAttributes( ).getAttr<XMLAttributeCards>( "cards" );
    
    if (card->level < level || isTrump( )) {
        if (card->level < getMaxLevel( )) {
            card->level++;

            if (card->suit < 0)
                card->suit = getRandomSuit( );

            oldact(_("{cТы уби$gло|л|ла $t из Колоды.{x"), pkiller, getFace( viewerLang( pkiller ), '4' ).c_str( ), 0, TO_CHAR );
            oldact(_("{cТеперь ты $t!{x"), pkiller, card->getFace( viewerLang( pkiller ), '1' ).c_str( ), 0, TO_CHAR );
        }
    
        level--;

        if (level >= 0)
            args.pch->pecho( _("{cТы становишься %s.{x"), getFace( viewerLang( args.pch ), '5' ).c_str( ) );
        else {
            args.pch->pecho(_("{cТы выбываешь из колоды!{x"));
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

