/* $Id: xmlloader.cpp,v 1.1.2.5.6.5 2010-09-01 08:21:11 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#include <sstream>
#include <fstream>

#include "xmlloader.h"

#include "logstream.h"
#include "exceptionskipvariable.h"
#include "xmlvariable.h"
#include "xmldocument.h"
#include "dbio.h"

/*-------------------------------------------------------------------------
 * XMLLoader
 *------------------------------------------------------------------------*/
XMLLoader::~XMLLoader( )
{
}

bool XMLLoader::loadXML( XMLVariable *var, const DLString &name, bool fIgnoreNotFound ) const 
{
    try {
        DBIO dbio( getTablePath( ), getTableName( ) );
        dbio.open( );
        
        if (fIgnoreNotFound && !dbio.getEntryAsFile( name ).exist( )) 
            return false;

        DBIO::DBNode dbNode = dbio.select( name );
        basic_istringstream<char> xmlStream( dbNode.getXML( ) );
        
        XMLDocument::Pointer root( NEW );
        root->load( xmlStream );

        XMLNode::Pointer node = root->getFirstNode( );
        if (!node.isEmpty( ))
            var->fromXML( node );
    }
    catch (const Exception& ex) {
        LogStream::sendError( ) 
            << "Error while loading " << name << ".xml: " << ex << endl;
        return false;
    }

    return true;
}

bool XMLLoader::saveXML( const XMLVariable *var, const DLString &name, bool fSafe ) const
{
    try {
        DBIO dbio( getTablePath( ), getTableName( ) );
        XMLNode::Pointer node( NEW, getNodeName( ) );

        if (var->toXML( node )) {
            XMLDocument::Pointer root( NEW );
            root->appendChild( node );

            basic_ostringstream<char> ostr;
            root->save( ostr );
            
            if (fSafe)
                dbio.safeInsert( name, ostr.str( ) );
            else
                dbio.insert( name, ostr.str( ) );
        }
    }
    catch (const Exception& ex) {
        LogStream::sendError( ) 
            << "Error while saving " << name << ".xml: " << ex << endl;
        return false;
    }
    
    return true;
}

