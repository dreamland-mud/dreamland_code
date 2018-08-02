/* $Id: cwizlist.cpp,v 1.1.2.3.6.3 2008/05/23 01:16:12 rufina Exp $
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
 *    Zadvinsky Alexandr   {Kiddy}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "cwizlist.h"
#include "class.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "comm.h"
#include "merc.h"
#include "def.h"

CWizlist::GodLevelName CWizlist::names[] =
{
  { "Implementors", 110, "{W" },
  { "Creators",     109, "{C" },
  { "Supremacies",  108, "{C" },
  { "Deities",      107, "{C" },
  { "Gods",         106, "{C" },
  { "Immortals",    105, "{G" },
  { "DemiGods",     104, "{G" },
  { "Angels",       103, "{G" },
  { "Avatars",      102, "{G" }
};

CWizlist::SwordLine CWizlist::swordLines[] = {
 { "       |XX::XXXX|        ",    12 },
 { "/<<>>\\/<<>>\\/<<>>\\/<<>>\\",  1 },
 { "\\<<>>/\\<<>>/\\<<>>/\\<<>>/" , 1 },
 { "      |  .// \\.  |       ",   -1 }
};

const int CWizlist::textLine = 28;

void CWizlist::initSwords( )
{
    cSwordLine = lineCounter = 0;
}

/* добавляет в буффер строку текста, окруженную с 2-х сторон мечами */
void CWizlist::writeSwordLine( std::ostream &buf, char *str, char * color )
{
    int clen, i;

    if ((int) strlen(str) > textLine)        /* Урезаем текст, чтобы он поместился */
	str[textLine] = '\0';

    clen = (textLine-strlen(str))/2; /* Считаем пробелы перед текстом*/

    buf << swordLines[cSwordLine].name.c_str( ); /* рисуем меч */

    if ( color && color[0] != '\0' )
	buf << color;

    for( i=0;i<clen;++i)             /* Рисуем пробелы и текст */
	buf << " ";
    
    buf << str;

    if ( color && color[0] != '\0' )
	buf << "{x";

    clen = textLine-(clen+strlen( str )); /* Считаем пробелы после текста */
    for( i=0;i<clen;++i)                  /* и рисуем их */
	buf << " ";

    buf << swordLines[cSwordLine].name.c_str( ); /* рисуем второй меч */
    buf << "\n\r";

    /* Проверяем, не пора ли переходить к следующему кусочку меча */
    if ( (++lineCounter >= swordLines[cSwordLine].count) && (swordLines[cSwordLine].count>0) ) {
	lineCounter = 0;
	cSwordLine++;
    }
}

/* Добавляет в буфер строку пикселов между мечей */
void CWizlist::writeSwordPixels( std::ostream &res, char pixel, int count ) 
{
    char buf[textLine+1];
    int i;

    if (count>textLine)
	count = textLine;

    for (i=0;i<count;++i)
	buf[i] = pixel;

    buf[count] = '\0';
    writeSwordLine( res, buf, 0 );
}

COMMAND(CWizlist, "wizlist")
{
    do_help( ch, "wizlist" );

#if 0
    int cLevel;
    char bbuf[128];
    std::basic_ostringstream<char> buf;

    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    GodList gods;
    GodList::iterator j;

    for (i = pcm.begin( ); i != pcm.end( ); i++) 
	if (i->second->getLevel( ) > LEVEL_HERO)
	    gods.push_back( i->second );

    gods.sort( CompareGods( ) );
    j = gods.begin( );

    initSwords();

    /* Рисуем заголовок */
    buf <<
    "        ________            **********************           ________\n\r"
    "      /+_+_+_+_+_\\       ** The gods of Dream Land **      /+_+_+_+_+_\\\n\r"
    "      \\__________/          **********************         \\__________/\n\r";


    /* Для каждого уровня богов */
    for ( cLevel = 0; cLevel < ( int )( sizeof(names)/sizeof(GodLevelName) ); cLevel ++ ) {
	writeSwordLine( buf, "", 0 );

	sprintf( bbuf, "%s [%d]", names[cLevel].name.c_str( ), names[cLevel].level );
	writeSwordLine( buf, bbuf, 0 );
	writeSwordPixels( buf, '*', names[cLevel].name.length( )+5 );
	writeSwordLine( buf, "", 0 );

	/* Выводим всех богов этого уровня */
	for ( ; j != gods.end( ) && (*j)->getLevel( ) >= names[cLevel].level; j++) {
	    sprintf( bbuf, "%s", (*j)->getName( ).c_str( ) );
	    writeSwordLine( buf, bbuf, names[cLevel].color );
	}
    }

    /* Рисуем кончики мечей */
    buf <<
    "       \\        /                                           \\        /\n\r"
    "        \\      /                                             \\      /\n\r"
    "         \\    /                                               \\    /\n\r"
    "          \\  /                                                 \\  /\n\r"
    "           \\/                                                   \\/\n\r" ;
  
    /* Бухаем все это на экран */
    page_to_char( buf.str( ).c_str( ), ch );
#endif    
}
