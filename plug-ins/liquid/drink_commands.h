/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DRINK_COMMANDS_H__
#define __DRINK_COMMANDS_H__

#include "commandplugin.h"
#include "defaultcommand.h"

class CPour : public CommandPlugin, public DefaultCommand {
public:
    typedef ::Pointer<CPour> Pointer;

    CPour( );

    virtual void run( Character *, const DLString & );

private:
    void pourOut( Character *, Object * );
    void pourOut( Character *, Object *, Character * );
    void createPool( Character *, Object *, int );
    
    static const DLString COMMAND_NAME;
};

#endif
