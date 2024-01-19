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
  { -1000,  -950,  -900,   "дьявольск|ая|ую|ой|ой|ой|ой" },
  {  -900,  -850,  -800,   "демоническ|ая|ую|ой|ой|ой|ой" },
  {  -800,  -750,  -700,   "очень зл|ая|ую|ой|ой|ой|ой" },
  {  -700,  -500,  -300,   "зл|ая|ую|ой|ой|ой|ой" },
  {  -300,  -275,  -150,   "злобно-нейтральн|ая|ую|ой|ой|ой|ой" },
  {  -150,  -100,   -50,   "нейтрально-зл|ая|ую|ой|ой|ой|ой" },
  {   -50,     0,    50,   "абсолютно нейтральн|ая|ую|ой|ой|ой|ой" },
  {    50,   100,   150,   "нейтрально-добр|ая|ую|ой|ой|ой|ой" },
  {   150,   275,   300,   "добро-нейтральн|ая|ую|ой|ой|ой|ой" },
  {   300,   500,   700,   "добр|ая|ую|ой|ой|ой|ой" },
  {   700,   750,   800,   "очень добр|ая|ую|ой|ой|ой|ой" },
  {   800,   850,   900,   "свят|ая|ую|ой|ой|ой|ой" },
  {   900,   950,  1000,   "ангельск|ая|ую|ой|ой|ой|ой" },

  { 0, 0, 0, NULL }
};

DLString align_name_for_range( int min, int max )
{
    if (max <= -300)
        return "злая";
    if (min >= 300)
        return "добрая";            
    if (min >  -300 && max < 300)
        return "нейтральная";
    if (min > -300 and max >= 500)
        return "нейтральная или добрая";
    if (min <= -300 && max <= 300)
        return "злая или нейтральная";
    return "любая";
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

