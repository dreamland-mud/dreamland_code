/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLANTITLES_H__
#define __CLANTITLES_H__

#include "xmlmap.h"
#include "xmlvector.h"
#include "xmlstring.h"

class PCMemoryInterface;

/*
 * clan titles declaration
 */
class ClanTitles : public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<ClanTitles> Pointer;
    
    virtual ~ClanTitles( );
    virtual const DLString & build( PCMemoryInterface * ) const = 0;
    virtual void toStream( ostringstream & ) const = 0;
    virtual int size( ) const = 0;
};

/*
 * clan titles implementation
 */
/*
 * ClanLevelNames
 */
class ClanLevelNames: public XMLVariableContainer {
XML_OBJECT
public:
    
    virtual ~ClanLevelNames( );

    void toStream( ostringstream & ) const;

    XML_VARIABLE XMLString male;
    XML_VARIABLE XMLString female;
    XML_VARIABLE XMLStringNoEmpty abbr;
    XML_VARIABLE XMLStringNoEmpty english;
};

typedef XMLVectorBase<ClanLevelNames> ClanLevelNamesVector;

/*
 * ClanTitlesByClass 
 */
class ClanTitlesByClass : public ClanTitles, 
                          public XMLMapBase<ClanLevelNamesVector> 
{
public:
    typedef ::Pointer<ClanTitlesByClass> Pointer;
    
    virtual const DLString & build( PCMemoryInterface * ) const;
    virtual void toStream( ostringstream & ) const;
    virtual int size( ) const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};

/*
 * ClanTitlesByLevel
 */
class ClanTitlesByLevel : public ClanTitles, 
                          public ClanLevelNamesVector 
{
public:
    typedef ::Pointer<ClanTitlesByLevel> Pointer;
    
    virtual const DLString & build( PCMemoryInterface * ) const;
    virtual void toStream( ostringstream & ) const;
    virtual int size( ) const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};


#endif
