/* $Id$
 *
 * ruffina, 2004
 */
#ifndef BADNAMES_H
#define BADNAMES_H

#include "oneallocate.h"
#include "regexp.h"
#include "xmlconfigurableplugin.h"
#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmllist.h"

class BadNames : public OneAllocate, 
                   public XMLConfigurablePlugin, 
                   public virtual XMLVariableContainer 
{
XML_OBJECT    
public:
    typedef XMLListBase<XMLString> NameList;
    typedef std::list<RegExp> RegexpList;

    BadNames( );
    virtual ~BadNames( );
    
    bool checkName( const DLString &name ) const;
    bool checkRussianName( const DLString &name ) const;

    bool nameLength( const DLString &name ) const;
    bool nameEnglish( const DLString &name ) const;
    bool nameRussian( const DLString &name ) const;
    bool nameMobiles( const DLString &name ) const;
    bool nameReserved( const DLString &name ) const;
    bool nameReligion( const DLString &name, bool fRussian ) const;

protected:
    virtual void initialization( );

    XML_VARIABLE NameList names;
    XML_VARIABLE NameList patterns;
    RegexpList regexps;
};

extern BadNames *badNames;

#endif
