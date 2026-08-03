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
    
    DLString checkName( const DLString &name ) const;
    DLString checkRussianName( const DLString &name ) const;

    DLString nameLength( const DLString &name ) const;
    bool nameEnglish( const DLString &name ) const;
    bool nameRussian( const DLString &name ) const;
    bool nameMobiles( const DLString &name ) const;
    bool nameReserved( const DLString &name ) const;
    bool nameLegendary( const DLString &name ) const;
    bool nameReligion( const DLString &name, bool fRussian ) const;

protected:
    virtual void initialization( );

    XML_VARIABLE NameList names;
    // Names carrying legendary status: founders, immortals, translators credited in
    // 'help credits', and figures from the written history. Refused with their own
    // error code so the nanny can explain why and point at the mailbox.
    XML_VARIABLE NameList legendary;
    XML_VARIABLE NameList patterns;
    RegexpList regexps;
};

extern BadNames *badNames;

#endif
