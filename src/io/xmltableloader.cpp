/* $Id: xmltableloader.cpp,v 1.1.2.7 2014-09-19 11:45:55 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#include <sstream>
#include <fstream>

#include "xmltableloader.h"

#include "logstream.h"
#include "exceptionskipvariable.h"
#include "xmlvariable.h"
#include "xmlstreamable.h"
#include "xmldocument.h"
#include "dbio.h"

/*-------------------------------------------------------------------------
 * XMLTableLoader
 *------------------------------------------------------------------------*/
XMLTableLoader::~XMLTableLoader( )
{
}

void XMLTableLoader::saveAll( bool fVerbose )
{
    DBIO dir( getTablePath( ), getTableName( ) );
    XMLStreamable<XMLTableElement> ptrElement( getNodeName( ) );
    
    LogStream::sendNotice( ) << "Saving " << getTableName( ) << "..." << endl;

    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++)
        try {
            ostringstream ostr;

            ptrElement.setPointer( e->getPointer( ) );
            ptrElement.toStream( ostr );

            dir.insert( (*e)->getName( ), ostr.str( ) );

            if (fVerbose)
                LogStream::sendNotice( ) << "save '" << (*e)->getName( ) << ".xml'" << endl;
        }
        catch (const Exception& ex) {
            LogStream::sendError( ) 
                << "Error while saving " << (*e)->getName( ) << ".xml: " << ex << endl;
        }
}

void XMLTableLoader::loadAll( )
{
    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++) {
        (*e)->loaded( );
        (*e)->setLoader(this);
    }
}

void XMLTableLoader::unloadAll( )
{
    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++) {
        (*e)->unloaded( );
        (*e)->setLoader(NULL);
    }

    elements.clear( );
}

void XMLTableLoader::readAll( bool fVerbose )
{
    DBIO::DBNode dbNode;
    DBIO dir( getTablePath( ), getTableName( ) );
    
    dir.open( );

    LogStream::sendNotice( ) << "Loading " << getTableName( ) << "..." << endl;

    try {
        while (true) {
            XMLStreamable<XMLTableElement> element( getNodeName( ) );
            
            dbNode = dir.nextXML( );

            basic_istringstream<char> xmlStream( dbNode.getXML( ) );
            element.fromStream( xmlStream );

            element->setName( dbNode.getKey( ) );
            elements.push_back( element );
            
            if (fVerbose)
                LogStream::sendNotice( ) << "read '" << element->getName( ) << ".xml'" << endl;
        }
    } 
    catch (const ExceptionDBIOEOF &e1) {
    } 
    catch (const Exception &e2) {
        LogStream::sendError( ) << e2.what( ) << endl;
        throw e2;
    }
}


void XMLTableLoader::saveElement(XMLTableElement::Pointer element)
{
    DBIO dir( getTablePath( ), getTableName( ) );    
    LogStream::sendNotice( ) << "Saving " << getTableName( ) << "/" << element->getName() << "..." << endl;

    try {
        ostringstream ostr;
        XMLStreamable<XMLTableElement> ptrElement( getNodeName( ) );

        ptrElement.setPointer( element.getPointer( ) );
        ptrElement.toStream( ostr );

        dir.insert( element->getName( ), ostr.str( ) );
    }
    catch (const Exception& ex) {
        LogStream::sendError( ) 
            << "Error while saving " << element->getName( ) << ": " << ex << endl;
    }
}

void XMLTableLoader::loadElement(XMLTableElement::Pointer element)
{
    elements.push_back(element);
    element->loaded();
    element->setLoader(this);
}
