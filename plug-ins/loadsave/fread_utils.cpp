/* $Id$
 *
 * ruffina, 2004
 */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "fileformatexception.h"
#include "logstream.h"
#include "fread_utils.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        c = getc( fp );
    }
    while ( dl_isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        c = getc( fp );
    }
    while ( dl_isspace(c) );

    number = 0;

    sign   = false;
    if ( c == '+' )
    {
        c = getc( fp );
    }
    else if ( c == '-' )
    {
        sign = true;
        c = getc( fp );
    }

    if ( !isdigit(c) )
    {
        bug("no number", 0);
        throw FileFormatException( "Fread_number: bad format (%c no number).", c );
    }

    while ( isdigit(c) )
    {
        number = number * 10 + c - '0';
        c      = getc( fp );
    }

    if ( sign )
        number = 0 - number;

    if ( c == '|' )
        number += fread_number( fp );
    else if ( c != ' ' )
        ungetc( c, fp );

    return number;
}

/*
 * Read a 'long long' from a file.
 */
long long fread_number64( FILE *fp )
{
    long long number;
    bool sign;
    char c;

    do
    {
        c = getc( fp );
    }
    while ( dl_isspace(c) );

    number = 0;

    sign   = false;
    if ( c == '+' )
    {
        c = getc( fp );
    }
    else if ( c == '-' )
    {
        sign = true;
        c = getc( fp );
    }

    if ( !isdigit(c) )
    {
        bug("no number", 0);
        throw FileFormatException( "Fread_number64: bad format (%c no number).", c );
    }

    while ( isdigit(c) )
    {
        number = number * 10 + c - '0';
        c      = getc( fp );
    }

    if ( sign )
        number = 0 - number;

    if ( c == '|' )
        number += fread_number64( fp );
    else if ( c != ' ' )
        ungetc( c, fp );

    return number;
}


long fread_flag( FILE *fp)
{
        int number;
        char c;
        bool negative = false;

        do
        {
                c = getc(fp);
        }
        while ( dl_isspace(c));

        if (c == '-')
        {
                negative = true;
                c = getc(fp);
        }

        number = 0;

        if (!isdigit(c))
        {
                while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
                {
                        number += flag_convert(c);
                        c = getc(fp);
                }
        }

        while (isdigit(c))
        {
                number = number * 10 + c - '0';
                c = getc(fp);
        }

        if (c == '|')
                number += fread_flag(fp);
        else if  ( c != ' ')
                ungetc(c,fp);

        if (negative)
                return -1 * number;

        return number;
}

long flag_convert(char letter )
{
    long bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z')
    {
        bitsum = 1;
        for (i = letter; i > 'A'; i--)
            bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z')
    {
        bitsum = 67108864; /* 2^26 */
        for (i = letter; i > 'a'; i --)
            bitsum *= 2;
    }

    return bitsum;
}


/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
    ostringstream buf;
    char c;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( dl_isspace(c) );

    if(c == '~')
        return str_empty;

    buf.put(c);

    for ( ;; )
    {
        int i = getc(fp);
        switch (i)
        {
        default:
            buf.put(i);
            break;

//        case EOF:
        /* temp fix */

// угу, энтот фикс не дает вставать миру если где-то в загрузочных файлах
// (бало обнаружено на нотах) есть "Ъ" == FF, равно как и EOF == FF
// в результате теряется кипа памяти
// ИМХО оно тут вобще не нужно.
//
//      Корвин.
//            bug( "Fread_string: EOF", 0 );
//            return 0;
            /* exit( 1 ); */
//            break;

        case '\n':
            buf.put('\n');
            buf.put('\r');
            break;

        case '\r':
            break;

        case '~':
            return str_dup(buf.str( ).c_str( ));
        }
    }
}

char *fread_string_eol( FILE *fp )
{
    static bool char_special[256-EOF];
    char space[MAX_STRING_LENGTH];
    char *plast = space;
    char c;

    if ( char_special[EOF-EOF] != true )
    {
        char_special[EOF -  EOF] = true;
        char_special['\n' - EOF] = true;
        char_special['\r' - EOF] = true;
    }

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( dl_isspace(c) );

    if ( ( *plast++ = c ) == '\n')
        return &str_empty[0];

    for ( ;; )
    {
        if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
            continue;

        switch ( plast[-1] )
        {
        default:
            break;

        case EOF:
            throw FileFormatException( "Fread_string_eol  EOF" );
            break;

        case '\n':  case '\r':
            plast[-1] = '\0';
            return str_dup(space);
        }
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
        c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}


DLString fread_dlstring( FILE *fp )
{
    ostringstream buf;
    char c[2];
    
    c[1] = 0;

    do
        c[0] = getc( fp );
    while ( dl_isspace(c[0]) );

    for ( ;; )
    {
        if (feof( fp ))
            break;

        switch (c[0]) {
        default:
            buf << c;
            break;

        case '\n':
            buf << endl;
            break;

        case '\r':
            break;

        case '~':
            return buf.str( );
        }

        c[0] = getc( fp );
    }
    
    return buf.str( );
}

DLString fread_dlstring_to_eol( FILE *fp )
{
    char c;
    DLString s;

    do
    {
        c = getc( fp );
        s.append( c );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return s;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
        static char word[MAX_INPUT_LENGTH];
        char *pword;
        char cEnd;

        do
        {
                cEnd = getc( fp );
        }
        while ( dl_isspace( cEnd ) );

        if ( cEnd == '\'' || cEnd == '"' )
        {
                pword   = word;
        }
        else
        {
                word[0] = cEnd;
                pword   = word+1;
                cEnd    = ' ';
        }

        for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
        {
                *pword = getc( fp );
                if ( cEnd == ' ' ? dl_isspace(*pword) : *pword == cEnd )
                {
                        if ( cEnd == ' ' )
                                ungetc( *pword, fp );
                        *pword = '\0';
                        return word;
                }
        }
        word[MAX_INPUT_LENGTH - 1] = 0;

        throw FileFormatException( "Fread_word: word too long: %s", word );
}


void fread_to_end_string( FILE *pfile ) 
{
  char c;

  if( feof( pfile ) ) return;
  do {
    c = getc( pfile );
  } while( c != '~' && !feof( pfile ) );

  return;
}


