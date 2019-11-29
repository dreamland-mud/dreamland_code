/* $Id: xmltableloader.h,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#ifndef XMLTABLELOADER_H
#define XMLTABLELOADER_H

#include <list>

#include "xmltableelement.h"

/*
 * XMLTableLoader
 */
class XMLTableLoader {
public:
    typedef list<XMLTableElement::Pointer> LoadedList;
    
    virtual ~XMLTableLoader( );

    virtual DLString getTablePath( ) const = 0;
    virtual DLString getTableName( ) const = 0;
    virtual DLString getNodeName( ) const = 0;

    void readAll( bool fVerbose = false );
    void saveAll( bool fVerbose = false );
    void loadAll( );
    void unloadAll( );
    void saveElement(XMLTableElement::Pointer element);
    void loadElement(XMLTableElement::Pointer element);

    inline const LoadedList & getElements( ) const
    {
        return elements;
    }

protected:
    LoadedList elements;
};

#endif
