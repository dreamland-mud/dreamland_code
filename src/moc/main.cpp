/* $Id: main.cpp,v 1.18.2.4.24.1 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, 2004
 */
/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Wed Nov 22 2000
    copyright            : (C) 2000 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <fstream>
#include <algorithm>
#include <iostream>
#include <cstdio>

#include "system.h"
#include "mocfunctional.h"

using std::cerr;
using std::endl;

extern        FILE*        yyin;
extern        FILE*        yyout;

int yyparse( );

void yydbg(int i);

int main( int argc, char* argv[] )
{

        char* error = 0;

        yydbg(1);

        string include( "./" );

        for( int n = 1; n < argc && error == 0; n++ )
        {
                string arg = argv[n];

                if( arg[0] == '-' )
                {
                        if( arg[1] == 'i' )
                        {
                                if( !( n < argc - 1 ) )
                                {
                                        error = "Missing input-files name";
                                        break;
                                }
                                System::setInputFile( argv[++n] );
                        } else if( arg[1] == 'o' )
                        {
                                if( !( n < argc - 1 ) )
                                {
                                        error = "Missing output-file name";
                                        break;
                                }
                                System::setOutputFile( argv[++n] );
                        }
                        else if( arg[1] == 'I' )
                        {
                                include += ':'; include += arg.substr( 2 );
                        }
                        else if( arg[1] == 'r' )
                        {
                                System::setRoot( );
                        }
                        else
                        {
                                error = "Invalid argument";
                                break;
                        }
                }
        }

        if( error )
        {
                cerr << error << endl;
                return 1;
        }

        System::appendIncludePath( include );

        yyout = fopen( "/dev/null", "w" );

        if( System::isInputFile( ) )
        {
                System::getHeadersVector( ) = toList<System::VectorStringType>( System::getInputFile( ) );
                System::VectorStringType::iterator pos = System::getHeadersVector( ).begin( );
                for( pos = System::getHeadersVector( ).begin( );
                        pos != System::getHeadersVector( ).end( );
                        ++pos )
                {
                        System::setCurrentFile( *pos );
                        yyin = fopen( pos->c_str( ), "r" );
                        if( !yyin )
                        {
                                cerr << "moc: " << *pos << ": No such file" << endl;
                                return 1;
                        }
                        System::setST_INITIAL( );
                        yyparse( );
                        fclose( yyin );
                        System::getUsingNamespaceList().clear();
                }
        }

        fclose( yyout );
        System::generateMoc( );
        
        return 0;
}
