/* $Id$
 *
 * ruffina, 2004
 */
#include "alignment.h"
#include "pcharacter.h"
#include "pcrace.h"
#include "mercdb.h"
#include "grammar_entities_impl.h"
#include "act.h"


const struct alignment_t alignment_table [] = {
  { -1000,  -950,  -900,   "дьявольск|ий|ого|ому|ий|им|ом" },
  {  -900,  -850,  -800,   "демоническ|ий|ого|ому|ий|им|ом" },
  {  -800,  -750,  -700,   "очень зл|ой|ого|ому|ой|ым|ом" },
  {  -700,  -500,  -300,   "зл|ой|ого|ому|ой|ым|ом" },
  {  -300,  -275,  -150,   "злобно-нейтральн|ый|ого|ому|ый|ым|ом" },
  {  -150,  -100,   -50,   "нейтрально-зл|ой|ого|ому|ой|ым|ом" },
  {   -50,     0,    50,   "абсолютно нейтральн|ый|ого|ому|ый|ым|ом" },
  {    50,   100,   150,   "нейтрально-добр|ый|ого|ому|ый|ым|ом" },
  {   150,   275,   300,   "добро-нейтральн|ый|ого|ому|ый|ым|ом" },
  {   300,   500,   700,   "добр|ый|ого|ому|ый|ым|ом" },
  {   700,   750,   800,   "очень добр|ый|ого|ому|ый|ым|ом" },
  {   800,   850,   900,   "свят|ой|ого|ому|ой|ым|ом" },
  {   900,   950,  1000,   "ангельск|ий|ого|ому|ий|им|ом" },

  { 0, 0, 0, NULL }
};

DLString align_name_for_range( int min, int max )
{
    if (min <= -500 && max >= 500)
        return "любой";

    if (min <= -500)
        return "злой";
    if (max >= 500)
        return "добрый";
    return "нейтральный";
}

int align_choose_range( int min, int max, int n )
{
    int cnt = 0;
    
    for (int i = 0; alignment_table[i].rname; i++)
        if (min <= alignment_table[i].minValue
            && max >= alignment_table[i].maxValue)
            if (++cnt == n)
                return alignment_table[i].aveValue;
    
    return ALIGN_ERROR;
}

int align_choose_range( int min, int max, const DLString &arg )
{
    if (arg.empty( ))
        return ALIGN_ERROR;

    for (int i = 0; alignment_table[i].rname; i++)
        if (min <= alignment_table[i].minValue
            && max >= alignment_table[i].maxValue)
            if (arg.strPrefix( russian_case( alignment_table[i].rname, '1' ) ))
                return alignment_table[i].aveValue;
    
    return ALIGN_ERROR;
}

void align_print_range( int min, int max, ostringstream &buf )
{
    int cnt = 0;
    
    for (int i = 0; alignment_table[i].rname; i++)
        if (min <= alignment_table[i].minValue
            && max >= alignment_table[i].maxValue)
        {
            buf << dlprintf( "%2d) %s\r\n", 
                             ++cnt, 
                             russian_case( alignment_table[i].rname, '1' ).c_str( ) );
        }
}

void align_get_ranges( PCharacter *ch, int &a_min, int &a_max )
{
    int p_min = ch->getProfession( )->getMinAlign( ),
        p_max = ch->getProfession( )->getMaxAlign( );
    int r_min = ch->getRace( )->getPC( )->getMinAlign( ),
        r_max = ch->getRace( )->getPC( )->getMaxAlign( );

    if (r_min <= p_min && p_min <= r_max) {
        a_min = p_min;
        a_max = min( r_max, p_max );
        return;
    }

    if (p_min <= r_min && r_min <= p_max) {
        a_min = r_min;
        a_max = min( r_max, p_max );
        return;
    }

    a_min = r_min;
    a_max = r_max;
    return;
}


int align_choose_allowed( PCharacter *ch, const DLString &arg )
{
    int a_min, a_max;

    align_get_ranges( ch, a_min, a_max );
    return align_choose_range( a_min, a_max, arg );
}

int align_choose_allowed( PCharacter *ch, int n )
{
    int a_min, a_max;

    align_get_ranges( ch, a_min, a_max );
    return align_choose_range( a_min, a_max, n );
}

void align_print_allowed( PCharacter *ch, ostringstream &buf )
{
    int a_min, a_max;

    align_get_ranges( ch, a_min, a_max );
    align_print_range( a_min, a_max, buf );
}

DLString align_name( int a )
{
    for (int i = 0; alignment_table[i].rname; i++)
        if (a >= alignment_table[i].minValue        
            && a <= alignment_table[i].maxValue)
        {
            return alignment_table[i].rname;
        }

    return DLString::emptyString;
}

DLString align_name( Character *ch )
{
    return align_name( ch->alignment );
}

DLString align_min( PCharacter *ch )
{
    int a_min, a_max;
    align_get_ranges( ch, a_min, a_max );
    return align_name( a_min + 1 );
}

DLString align_max( PCharacter *ch )
{
    int a_min, a_max;
    align_get_ranges( ch, a_min, a_max );
    return align_name( a_max );
}

static const char * evil [] = {
    "злое", "злой", "злая", "злые"
};
static const char * good [] = {
    "доброе", "добрый", "добрая", "добрые"
};
static const char * neutral [] = {
    "нейтральное", "нейтральный", "нейтральная", "нейтральные"
};
const char *align_name_short(Character *ch, const Grammar::MultiGender &mg)
{
    int a = ch->alignment;
    if (ALIGN_IS_EVIL(a))
        return evil[mg];
    if (ALIGN_IS_GOOD(a))
        return good[mg];
    return neutral[mg];    
}

