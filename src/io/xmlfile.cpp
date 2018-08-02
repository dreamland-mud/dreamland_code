/* $Id: xmlfile.cpp,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include <sstream>
#include <fstream>

#include "xmlfile.h"

#include "logstream.h"
#include "exceptionskipvariable.h"
#include "xmlvariable.h"
#include "xmldocument.h"

/*-------------------------------------------------------------------------
 * XMLFile
 *------------------------------------------------------------------------*/
XMLFile::XMLFile( )
{
}

XMLFile::XMLFile( const DLString &f, const DLString &n, XMLVariable *v )
                  : filename( f ), nodename( n ), variable( v )
{
}

XMLFile::XMLFile( const DLFile &file, const DLString &n, XMLVariable *v )
                  : filename( file.getPath( ) ),
		    nodename( n ),
		    variable( v )
{
}

XMLFile::~XMLFile( )
{
}

bool XMLFile::load( ) 
{
    ifstream istr( filename.c_str( ) );

    if (!istr) 
	return false;

    try {
	XMLDocument::Pointer root( NEW );
	root->load( istr );

	XMLNode::Pointer node = root->getFirstNode( );

	if (!node.isEmpty( ))
	    variable->fromXML( node );
    }
    catch( const Exception& ex )
    {
	LogStream::sendError( ) << "Error while loading " << filename << ": " 
	                        << ex << endl;
	return false;
    }

    return true;
}

bool XMLFile::save( ) const
{
    ofstream ostr( filename.c_str( ) );

    if (!ostr) 
	return false;

    try
    {
	XMLDocument::Pointer root( NEW );
	XMLNode::Pointer node( NEW, nodename );

	if (variable->toXML( node )) {
	    root->appendChild( node );
	    root->save( ostr );
	}
    }
    catch (const ExceptionSkipVariable &) {
	return true;
    }
    catch (const Exception& ex)
    {
        LogStream::sendError( ) 
	    << "Error while saving " << filename << ": " << ex << endl;
	return false;
    }

    return true;
}

