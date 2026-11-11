/* $Id: cplugin.h,v 1.4.2.5.18.2 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * based on CPlugin by NoFate, 2001
 */

#ifndef CPLUGIN_H
#define CPLUGIN_H

#include "admincommand.h"

class CPlugin : public AdminCommand 
{
XML_OBJECT
public:
    typedef ::Pointer<CPlugin> Pointer;

    CPlugin( );
    
    virtual void initialization( );
    virtual void destruction( );
    virtual void run( Character* character, const DLString& argument );

private:
    void usage( Character * );
    void doList( Character * );
    void doReload( Character *, DLString );

    static const DLString COMMAND_NAME;
};


#endif
