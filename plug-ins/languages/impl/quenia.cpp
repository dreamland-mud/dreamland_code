/* $Id$
 *
 * ruffina, 2009
 */
#include "quenia.h"
#include "quenia_effects.h"
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "class.h"
#include "pcharacter.h"

#include "mercdb.h"
#include "def.h"

const DLString QueniaLanguage::LANG_NAME = "quenia";

QueniaLanguage::QueniaLanguage( ) : Language( LANG_NAME )
{
}

void QueniaLanguage::initialization( )
{
    Class::regMoc<GoodSpellWE>( );
    Class::regMoc<AccuracyWE>( );
    Class::regMoc<QueniaLanguage>( );
    Language::initialization( );
}

void QueniaLanguage::destruction( )
{
    Language::destruction( );
    Class::unregMoc<QueniaLanguage>( );
    Class::unregMoc<GoodSpellWE>( );
    Class::unregMoc<AccuracyWE>( );
}

static bool is_vowel( char c )
{
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

static DLString random_vowel( )
{
    switch (number_range( 0, 3 )) {
    case 0: return "a";
    case 1: return "i";
    case 2: return "o";
    case 3: return "u";
    default: return "e"; // very uncommon
    }
}

DLString QueniaLanguage::createDictum( ) const
{
    int n;
    DLString dictum;
    DLString pre, suf, root;
    
    if (suffixes.empty( ) && prefixes.empty( ) && roots.empty( ))
        throw LanguageException( *this, "totally empty" );
    
    if (!roots.empty( )) {
        n = number_range( 0, roots.size( ) - 1 );
        root = roots[n].getValue( );
    } 
    
    if (!suffixes.empty( )) {
        n = number_range( 0, suffixes.size( ) - 1 );
        suf = suffixes[n].getValue( );
    }
    
    dictum = concatenate( root, suf );

    /*
     * Начинать с корня, если слово длинное, либо со случайного суффикса,
     * либо с -i, -u (отрицание), -a
     */
    if (dictum.size( ) < 7 || chance(30)) {
        if (!prefixes.empty( )) {
            n = number_range( 0, prefixes.size( ) - 1 );
            pre = prefixes[n].getValue( );
        }
    }
    else if (chance(80)) {
        pre = random_vowel( );
    }
    
    dictum = concatenate( pre, dictum );
    correct( dictum );    
    
    if (dictum.size( ) < 7)
        dictum.erase( );

    return dictum;
}

DLString QueniaLanguage::concatenate( DLString &prefix, DLString &suffix ) const
{
    char p_last, s_first;
    DLString::size_type p_size, s_size, v_0, v_1;
    DLString dictum;
    
    dictum  = prefix + suffix;
    
    if (prefix.empty( ) || suffix.empty( ))
        return dictum;

    p_size  = prefix.size( );
    s_size  = suffix.size( );
    p_last  = prefix.at( p_size - 1 );
    s_first = suffix.at( 0 );

    /*
     * Вариант -y + y-
     * Допустимые сочетания гласных на стыке слов:
     * -ea-, -eo-, -ia-, -ie-, -io-, -iu-
     */
    if (is_vowel( p_last ) && is_vowel( s_first ))
    {
        static const char *diphs [] = { "ea", "eo", "ia", "ie", "io", "iu", };
        static const int diphs_size = sizeof( diphs ) / sizeof( *diphs );
        
        for (v_0 = p_size - 1;
             v_0 >= 0 && is_vowel( prefix[v_0] );
             v_0--)
            ;
        
        for (v_1 = 0;
             v_1 < s_size && is_vowel( suffix[v_1] );
             v_1++)
            ;
        
        v_1 += p_size - 1;
        
        ++v_0;
        dictum.replace( v_0, 
                        v_1 - v_0, 
                        diphs[ number_range( 0, diphs_size - 1 ) ] );
    } 
    
    /*
     * Вариант -x + x-
     * Сочетание согласных на стыке:
     * (1) контрактура
     * (2) замены nl~ll, nr~rr, ns~ss, nm~mm
     * (3) вставка гласной (чаще всего -e)
     */
    if (!is_vowel( p_last ) && !is_vowel( s_first ))
    {
        for (v_0 = p_size - 1, v_1 = 0;
             v_0 >= 0 && v_1 < s_size && prefix[v_0] == suffix[v_1];
             v_0--, v_1++)
            ;
            
        if (v_1) 
        { 
            /* (1) */
            dictum.erase( ++v_0, v_1 );
        }
        else if (p_last == 'n' && (   s_first == 'l' || s_first == 'r' 
                                   || s_first == 's' || s_first == 'm'))
        {
            /* (2) */
            dictum[ p_size - 1 ] = s_first;
        }
        else {     
            /* (3) */
            
            if (chance( 50 ))
                dictum.insert( p_size, 
                               chance( 80 ) ? "e" : chance( 50 ) ? "a" : "o" );    
        }
    }
    
    return dictum;
}

/*
     * квенийские слова никогда не содержат y, lh, gh, aw, ch, mh, ae,
       ew, rh, &#156; или iw.
     * никакие эльфийские языки вообще не содержат j, sh, zh. Нет также в
       Q и гласных с циркумфлексом [^].
     * eсли же начинается оно с mb, b, nd, d, ng, g, lh, mh, rh, dh, gh
       или какого-нибудь io -- это никак не Q.
     * а вот mb/b, nd, ld, rd, ng в Q могут встретиться, но только в
       середине слова.

    (c)
*/       
void QueniaLanguage::correct( DLString &d ) const
{
    static const char * replace [] = 
    { 
        "y",  "e",
        "lh", "l",
        "gh", "g",
        "aw", "a",
        "ch", "c",
        "mh", "m",
        "ae", "e",
        "ew", "e",
        "rh", "r",
        "iw", "i",
        "j",  "",
        "sh", "s",
        "zh", "s",
        "hh", "h",
        "uu", "u",
        0,
    };
    static const char * wrong_start [] =     
        { "mb", "b", "nd", "d", "ng", "g", "lh", "mh", "rh", "dh", 
          "gh", "io", "ld", "rd", "ee", "oo", "aa", "ii", "uu", "nn",
          0 };
          
    unsigned int i;
    DLString d0;
    bool fWrong = true;
    int repeat = 100;
    
    while (fWrong && repeat-- > 0 && !d.empty( )) {
        fWrong = false;
        
        /*
         * incorrect combinations
         */
        for (i = 0; replace[i]; i = i + 2) {
            d0 = d;
            
            if (d0 != d.replaces( replace[i], replace[i + 1] ))
                fWrong = true;
        }

        /*
         * incorrect first letters
         */
        for (i = 0; wrong_start[i]; i++)
            if (!str_prefix( wrong_start[i], d.c_str( ) )) {
                d.erase( 0, wrong_start[i][1] ? 2 : 1 );
                fWrong = true;
                break;
            }
        
        /*
         * remove 'triphtongs', 3 or more vowels in a row
         */
        for (i = 0; i < d.size( ); ) {
            DLString::size_type v_0;
            
            for (v_0 = i; v_0 < d.size( ) && is_vowel( d[v_0] ); v_0++)
                ;

            if (v_0 - i > 2) {
                d.replace( i, v_0 - i, random_vowel( ) );
                fWrong = true;
                break;
            }

            i = v_0 + 1;
        }
    }
}

void QueniaLanguage::dream( const Word &word, PCharacter *ch ) const
{
    ch->printf( "Сквозь зыбкую пелену сна тебе является слово {c%s.{x\r\n",
                word.toStr( ) ); 
}

