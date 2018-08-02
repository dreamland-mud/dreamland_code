/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultracelanguage.h"
#include "character.h"
#include "race.h"
#include "merc.h"
#include "def.h"

/*-------------------------------------------------------------------
 * DefaultRaceLanguage
 *------------------------------------------------------------------*/
DefaultRaceLanguage::DefaultRaceLanguage( )
                : races( raceManager ),
		  common( false )
{
}


void DefaultRaceLanguage::loaded( )
{
    raceLanguageManager->registrate( Pointer( this ) );
}

void DefaultRaceLanguage::unloaded( )
{
    raceLanguageManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultRaceLanguage::getShortDescr( ) const
{
    return shortDescr.getValue( );
}

bool DefaultRaceLanguage::available( Character *ch ) const
{
    if (common.getValue( ))
	return true;
    
    if (ch->is_immortal( ))
	return true;

    if (races.isSet( ch->getRace( ) ))
	return true;

    return false;
}

struct translation_type
{
    char common;
    const char * language;
};

const struct translation_type translation_table [] =
{
  {' ',     " "},
  {'a',     "e"}, {'A',     "E"}, {'b',     "c"}, {'B',     "C"},
  {'c',     "d"}, {'C',     "D"}, {'d',     "f"}, {'D',     "F"},
  {'e',     "i"}, {'E',     "I"}, {'f',     "g"}, {'F',     "G"},
  {'g',     "h"}, {'G',     "H"}, {'h',     "j"}, {'H',     "J"},
  {'i',     "o"}, {'I',     "O"}, {'j',     "k"}, {'J',     "K"},
  {'k',     "l"}, {'K',     "L"}, {'l',     "m"}, {'L',     "M"},
  {'m',     "n"}, {'M',     "N"}, {'n',     "p"}, {'N',     "P"},
  {'o',     "u"}, {'O',     "U"}, {'p',     "q"}, {'P',     "Q"},
  {'q',     "r"}, {'Q',     "R"}, {'r',     "s"}, {'R',     "S"},
  {'s',     "t"}, {'S',     "T"}, {'t',     "v"}, {'T',     "V"},
  {'u',     "y"}, {'U',     "Y"}, {'v',     "w"}, {'V',     "W"},
  {'w',     "x"}, {'W',     "X"}, {'x',     "z"}, {'X',     "Z"},
  {'y',     "a"}, {'Y',     "A"}, {'z',     "b"}, {'Z',     "B"},

  {'Á',     "Å"}, {'á',     "å"}, {'Â',     "×"}, {'â',     "÷"},
  {'×',     "Ç"}, {'÷',     "ç"}, {'Ç',     "Ä"}, {'ç',     "ä"},
  {'Ä',     "Ö"}, {'ä',     "ö"}, {'Å',     "É"}, {'å',     "é"},
  {'Ö',     "Ú"}, {'ö',     "ú"}, {'Ú',     "Ê"}, {'ú',     "ê"},
  {'É',     "Ï"}, {'é',     "ï"}, {'Ê',     "Ë"}, {'ê',     "ë"},
  {'Ë',     "Ì"}, {'ë',     "ì"}, {'Ì',     "Í"}, {'ì',     "í"},
  {'Í',     "Î"}, {'í',     "î"}, {'Î',     "Ð"}, {'î',     "ð"},
  {'Ï',     "Õ"}, {'ï',     "õ"}, {'Ð',     "Ò"}, {'ð',     "ò"},
  {'Ò',     "Ó"}, {'ò',     "ó"}, {'Ó',     "Ô"}, {'ó',     "ô"},
  {'Ô',     "Æ"}, {'ô',     "æ"}, {'Õ',     "Ü"}, {'õ',     "ü"},
  {'Æ',     "È"}, {'æ',     "è"}, {'È',     "Ã"}, {'è',     "ã"},
  {'Ã',     "Þ"}, {'ã',     "þ"}, {'Þ',     "Û"}, {'þ',     "û"},
  {'Û',     "Ý"}, {'û',     "ý"}, {'Ý',     "ß"}, {'ý',     "ÿ"},
  {'ß',     "Â"}, {'ÿ',     "â"}, {'Ü',     "À"}, {'ü',     "à"},
  {'À',     "Ñ"}, {'à',     "ñ"}, {'Ñ',     "Á"}, {'ñ',     "á"},
  { 0, 	0 } 
};

DLString DefaultRaceLanguage::translate( const DLString &arg, Character *ch, Character *victim ) const
{
    ostringstream buf;

    if (victim && available( victim ))
	return arg;
    
    for (unsigned int i = 0; i < arg.size( ); i++) {
	char c = arg.at( i );

	for (int j = 0; translation_table[j].common != '\0'; j++)
	    if (translation_table[j].common == c) {
		buf << translation_table[j].language;
		break;
	    }
    }
    
    return buf.str( );
}


