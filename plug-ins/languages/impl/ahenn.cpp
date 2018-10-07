/* $Id$
 *
 * ruffina, 2009
 */
#include "ahenn.h"
#include "ahenn_effects.h"
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "class.h"
#include "pcharacter.h"

#include "mercdb.h"
#include "def.h"

/*---------------------------------------------------------------------------
 * utilities
 *--------------------------------------------------------------------------*/
static bool is_vowel( char c )
{
    static const char vowels [] = 
	{ 'а', 'я', 'е', 'э', 'и', 'ы', 'о', 'у', 'ю', 0 };

    for (int i = 0; vowels[i]; i++)
	if (vowels[i] == c)
	    return true;
    
    return false;
}

static bool to_vowel( char c )
{
    return c == 'й';
}

static bool to_consonant( char c )
{
    return c == 'ь' || c == 'ъ';
}

static bool is_consonant( char c )
{
    return !is_vowel( c ) && !to_vowel( c ) && !to_consonant( c );
}

static bool is_cons_group( const char *arg, int n )
{
    if (n < 0 || !arg[n] || !arg[n+1])
	return false;

    return is_consonant( arg[n] ) && to_consonant( arg[n+1] );
}

static bool is_vowel_group( const char *arg, int n )
{
    if (n < 0 || !arg[n] || !arg[n+1])
	return false;
	
    return (to_vowel( arg[n] ) && is_vowel( arg[n+1] ) && n == 0)
	    || (is_vowel( arg[n] ) && to_vowel( arg[n+1] ));
}

static void copy_char( const char *src, char *&dst, int &i )
{
    *dst++ = src[i]; 
    i++;
}

static bool copy_vowels( const char *arg, char *&s, int &i )
{
    if (arg[i] == 0) {
	*s = 0;
	return false;
    }
    
    if (is_vowel_group( arg, i ))
    {
	copy_char( arg, s, i );
	copy_char( arg, s, i );
	return true;
    }

    if (is_vowel( arg[i] ) || to_vowel( arg[i] )) {
	copy_char( arg, s, i );
	return true;
    }
    
    return false;
}

static bool copy_consonants( const char *arg, char *&s, int &i )
{
    if (arg[i] == 0) {
	*s = 0;
	return false;
    }
    
    if (is_cons_group( arg, i ))
    {
	copy_char( arg, s, i );
	copy_char( arg, s, i );
	return true;
    }
    
    if (is_consonant( arg[i] ) || to_consonant( arg[i] )) {
	copy_char( arg, s, i );
	return true;
    }
	
    return false;
}

/*---------------------------------------------------------------------------
 * AhennLanguage
 *--------------------------------------------------------------------------*/
const DLString AhennLanguage::LANG_NAME = "ahenn";

AhennLanguage::AhennLanguage( ) : Language( LANG_NAME )
{
}

void AhennLanguage::initialization( )
{
    Class::regMoc<BadSpellWE>( );
    Class::regMoc<InspirationWE>( );
    Class::regMoc<AhennLanguage>( );
    Language::initialization( );
}

void AhennLanguage::destruction( )
{
    Language::destruction( );
    Class::unregMoc<AhennLanguage>( );
    Class::unregMoc<BadSpellWE>( );
    Class::unregMoc<InspirationWE>( );
}


DLString AhennLanguage::createDictum( ) const
{
    DLString dictum;
    vector<DLString> w;
    vector<vector<DLString> > syl;
    unsigned int cnt = 3, n, dsize, i;
    
    if (words.empty( ))
	throw LanguageException( *this, "empty words" );
    
    /*
     * make a word out of parts of three other words
     */
    for (i = 0; i < cnt; i++) {
	n = number_range( 0, words.size( ) - 1 );
	w.push_back( words[n].getValue( ) );
    }
    
    syl.resize( cnt );
    
    for (i = 0; i < cnt; i++) 
	parseSyllabes( w[i], syl[i] );
    
    dictum = mixSyllabes( syl );
    dsize = dictum.size( );
    
    if (dsize < 5) {
	dictum.erase( );
	return dictum;
    }

    /*
     * pretend this is a verb and add verb-endings
     */
    if (!verb_ends.empty( )
	&& dictum.at( dsize - 1 ) == 'о' 
	&& chance( 80 )) 
    {
	dictum.erase( dsize - 1, dsize );
	dictum += verb_ends[ number_range( 0, verb_ends.size( ) - 1 ) ].getValue( );
    }

    /*
     * dash and suffixes/prefixes
     */
    if (chance( 20 ) && !prefixes.empty( ))  {
	dictum = prefixes[ number_range( 0, prefixes.size( ) - 1 ) ].getValue( )
	         + "-" 
		 + dictum;
    }
    else if (chance( 20 ) && !suffixes.empty( )) {
	dictum = dictum 
	         + "-" 
	         + suffixes[ number_range( 0, suffixes.size( ) - 1 ) ].getValue( );
    }
    
    /*
     * final correction (totally XXX)
     */
    for (i = 0; i < dictum.size( ); i++) {
	DLString::size_type v_0;

	// eliminate @@@@@
	if (is_vowel( dictum[i] )) { 
	    for (v_0 = i; v_0 < dictum.size( ) && is_vowel( dictum[v_0] ); v_0++)
		;
		
	    if (v_0 - i > 2) 
		dictum.erase( i, v_0 - i - 1 );
	}
	// eliminate йй, $й
	else if (i != dictum.size( ) && dictum[i + 1] == 'й') { 
	    for (v_0 = i + 1; v_0 < dictum.size( ) && dictum[v_0] == 'й'; v_0++)
		;

	    dictum.erase( i + 1, v_0 - i - 1 );
	}
    }
    // льл, ллл

    return dictum;
}

void AhennLanguage::parseSyllabes( const DLString &argument, 
                                   vector<DLString> &syllabes ) const
{
    int i;
    const char *arg = argument.c_str( );
    
    for (i = 0; arg[i]; ) {
	char syl[256];
	char *s = syl;
	bool two_vowels;
	
	while (copy_consonants( arg, s, i ))
	    ;
	
	if (!arg[i]) {
	    syllabes.push_back( syl );
	    break;
	}
	
	two_vowels = false;

	while (copy_vowels( arg, s, i )) {
	    if (!syllabes.empty( )) 
		if (is_vowel( arg[i] ) || to_vowel( arg[i] )) {
		    two_vowels = true;
		    break;
		}
	}
	
	if (two_vowels) {
	    *s = 0;
	    syllabes.push_back( syl );
	    continue;
	}
	
	if (!arg[i]) {
	    syllabes.push_back( syl );
	    break;
	}

	while (copy_consonants( arg, s, i ))
	    ;
	
	if (!arg[i]) {
	    syllabes.push_back( syl );
	    break;
	}

	i--;
	s--;
	
	if (is_cons_group( arg, i - 1 )) {
	    i--;
	    s--;
	}

	*s = 0;
	syllabes.push_back( syl );
    }
    
    if (syllabes.size( ) > 1) {
	unsigned int n = syllabes.size( ) - 1;

	if (syllabes[n].size( ) == 1 && is_vowel( syllabes[n].at( 0 ) )) {
	    syllabes[n-1] += syllabes[n];
	    syllabes.erase( syllabes.begin( ) + n );
	}
    }
}

DLString AhennLanguage::mixSyllabes( vector<vector<DLString> > &syllabes ) const
{
    vector<DLString> root, suf, pref;
    unsigned int size = syllabes.size( );
    unsigned int i, n[3];
	
    for (i = 0; i < size; i++) {
	vector<DLString> &syl = syllabes[i];
	unsigned int s = syl.size( );
	
	switch (s) {
	case 0:
	    break;
	    
	case 1:
	    suf.push_back( DLString::emptyString );
	    root.push_back( syl[0] );
	    pref.push_back( DLString::emptyString );
	    break;
	
	case 2:
	    suf.push_back( syl[0] );
	    root.push_back( DLString::emptyString );
	    pref.push_back( syl[1] );
	    break;
	    
	case 3:
	    suf.push_back( syl[0] );
	    root.push_back( syl[1] );
	    pref.push_back( syl[2] );
	    break;
	
	default:
	    suf.push_back( syl[0] );
	    root.push_back( syl[number_range( 1, s - 2 )] );
	    pref.push_back( syl[s - 1] );
	    break;
	}
    }
	
    for (i = 0; i < 3; i++) {
	unsigned int j;
	bool equal = true;
	
	while (equal) {
	    equal = false;
	    n[i] = number_range( 0, size - 1 );

	    for (j = 0; j < size; j++)
		if (n[j] == n[i] && i != j) {
		    equal = true;
		    break;
		}
	}
    }
	
    return suf[n[0]] + root[n[1]] + pref[n[2]];
}

void AhennLanguage::dream( const Word &word, PCharacter *ch ) const
{
    ch->printf( "Из звуков мелодии, древней, как сам мир, рождается слово {c%s{x.\r\n",
                word.toStr( ) );
}

