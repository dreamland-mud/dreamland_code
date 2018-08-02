/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MYSOCIAL_H__
#define __MYSOCIAL_H__

#include "commandplugin.h"
#include "defaultcommand.h"
#include "customsocial.h"

class MySocial : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
    typedef ::Pointer<MySocial> Pointer;

    MySocial( );

    virtual void run( Character *, const DLString & );
    
private:
    static const DLString COMMAND_NAME;

    bool hasMeOnly( Character *,  const DLString & );
    bool hasNoVariables( Character *, const DLString & );
    bool hasVictOnly( Character *, const DLString & );
    bool hasBoth( Character *, const DLString & );
    void doList( Character *, XMLAttributeCustomSocials::Pointer );
    void doDelete( Character *, XMLAttributeCustomSocials::Pointer, const DLString & );
    void doShow( Character *, XMLAttributeCustomSocials::Pointer, const DLString & );
    void usage( Character * );
		
};

#endif
