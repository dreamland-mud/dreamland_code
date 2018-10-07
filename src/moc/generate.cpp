/* $Id: generate.cpp,v 1.27.2.4.8.2 2007/09/29 19:22:21 rufina Exp $
 *
 * ruffina, 2004
 * rewritten... twice
 */
/***************************************************************************
	      generate.cpp  -  description
		 -------------------
    begin	: Wed Nov 22 2000
    copyright	: (C) 2000 by Igor S. Petrenko
    email	: nofate@europe.com
 ***************************************************************************/

#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <functional>
#include <iostream>

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include "system.h"

using namespace std;

ofstream	fmoc;

extern    char**	environ;

static void 
appendNodes( System::ClassType::MapNodeType::value_type value, 
	     System::ClassType::MapNodeType* nodes )
{
    System::NodeType& v = value.second;
    ( *nodes )[value.first] = v;
}

void error(const string &text);

static void 
ancestors( System::ListStringType::value_type value, 
	   System::ClassType::MapNodeType* nodes )
{
    System::MapClassType::iterator ca = System::getClassMap( ).find( value );
    if( ca != System::getClassMap( ).end( ) )
    {
	System::findAllNodes( ca->second, nodes );
    }
    else
    {
	printf(">>>>>> unknown ancestor class %s!\n", value.c_str());
//	error("unknown ancestor class " + value);
    }
}

void System::findAllNodes( ClassType c, ClassType::MapNodeType* nodes )
{
    // append nodes 
    for_each( c.nodes.begin( ), c.nodes.end( ), bind2nd( ptr_fun( appendNodes ), nodes ) );
    for_each( c.ancestors.begin( ), c.ancestors.end( ), bind2nd( ptr_fun( ancestors ), nodes ) );
}

void System::generateMoc( )
{
    basic_ostringstream<char>    output;
    VectorStringType ifiles;
    string outputFile = System::getOutputFile();

    fmoc.open( outputFile.c_str( ) ); // create output file
    if( !fmoc )
    {
	cerr << "moc: Cannot create " << outputFile << endl;
	exit( 1 );
    }

    fmoc	
	<< "/**" << endl
	<< " * " << outputFile << " generated automatically by moc." << endl
	<< " */" << endl << endl
	<< "#include <map>" << endl
	<< "#include \"moc.h\"" << endl
	<< "#include \"exceptionskipvariable.h\"" << endl
	<< "#include \"xmlnode.h\"" << endl;

    for( ListClassType::iterator mc = System::getMocClassList( ).begin( );
	mc != System::getMocClassList( ).end( );
	mc++ )
    {
	if( System::getInputFile( ).find( mc->getFile( ) ) != string::npos )
	    if( mc->isXML_OBJECT( ) )
		generateClass( output , *mc, ifiles );
    }

    for( vector<string>::size_type i = 0; i < ifiles.size( ); i++ )
    {
	fmoc << "#include \"" << ifiles[i] << "\"" << endl;
    }
  
    fmoc
	<< endl << endl
	<< output.str( );
    fmoc.close( );
}

void System::generateClass( ostream& output, const ClassType& mc, VectorStringType& ifiles )
{
    basic_ostringstream<char>    out_map;
    basic_ostringstream<char>    out_nodes;
    ClassType::MapNodeType  nodes;
    ClassType::MapNodeType::iterator  pos;
    
    string fname = mc.getFile( );
    vector<string>::iterator pfile = find( ifiles.begin( ), ifiles.end( ), fname );
    if( pfile == ifiles.end( ) )
    {
	ifiles.push_back( fname );
    }

    findAllNodes( mc, &nodes );

    for (pos = nodes.begin( ); pos != nodes.end( ); pos++) {
	string nodeName = pos->first;
	NodeType& node = pos->second;
	
	out_map << "        MOC_NODES_INIT_VAR( " << nodeName << ")" << endl;
	out_nodes << "    MOC_VARIABLE(" << nodeName << ")" << endl;

	if (node.hasVariable( )) 
	    generateHeader( ifiles, node.variable );
    }

    string cname = mc.getName( );
   
    /* программа должна иметь форму кирпича: >8) */
    output 
	<< "//------------------- " << cname << " ------------------" << endl << endl
	<< "class " << cname << "::__MetaInfo__  {            " << endl
	<< "public:                                           " << endl
        << "    typedef " << cname << " ContainerType;        " << endl 
	<< "                                                  " << endl
	<< "    MOC_CLASS_DECLS                               " << endl
	<< "                                                  " << endl
	<< "    __MetaInfo__( )                               " << endl
	<< "    {                                             " << endl
	<<           out_map.str( )                             
	<< "    }                                             " << endl
	<< "                                                  " << endl
	<<      out_nodes.str( )                                << endl
	<< "                                                  " << endl
	<< "    static __MetaInfo__ instance;                 " << endl
	<< "};                                                " << endl
	<<                                                         endl
	<< cname << "::__MetaInfo__ " << cname << "::__MetaInfo__::instance;" << endl
	<< "MOC_NODE_FROM_XML(" << cname << ")                " << endl
	<< "MOC_TO_XML(" << cname << ")                       " << endl
	<< "MOC_GET_TYPE(" << cname << ")                     " << endl
	<< "const DLString " << cname << "::MOC_TYPE = \"" << cname << "\";" << endl
	<< endl << endl;
}

void System::generateHeader( VectorStringType& ifiles, VariableXMLType & var )
{
    // Check types for headers
    string type = var.getType( );
    while( type[type.length( ) - 1] == '*' )
    {
	type.erase( type.length( ) - 1 );
    }

    MapClassType::iterator hpos = System::getClassMap( ).find( type );
    if( hpos != System::getClassMap( ).end( ) )
    {
	string file = hpos->second.getFile( );
	if( !file.empty( ) )
	{
	    vector<string>::iterator vpos = find( ifiles.begin( ), ifiles.end( ), file );
	    if( vpos == ifiles.end( ) ) 
		ifiles.push_back( file );
	}
    }
}
