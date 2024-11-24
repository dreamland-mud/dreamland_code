/* $Id: characterwrapper.h,v 1.1.4.11.6.2 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef _CHARACTERWRAPPER_H_
#define _CHARACTERWRAPPER_H_

#include "pluginwrapperimpl.h"
// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "xmlregister.h"
#include "exceptions.h"
// MOC_SKIP_END

class Character;

class CharacterWrapper : public PluginWrapperImpl<CharacterWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<CharacterWrapper> Pointer;

    CharacterWrapper( );

    virtual void setSelf( Scripting::Object * );
    virtual void extract( bool );
    virtual bool targetExists() const;

    void setTarget( Character * );
    void checkTarget( ) const ;
    Character *getTarget( ) const;
    
private:
    Character *target;
};

#endif 
