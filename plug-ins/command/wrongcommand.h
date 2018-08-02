/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __WRONGCOMMAND_H__
#define __WRONGCOMMAND_H__

#include "defaultcommand.h"
#include "commandplugin.h"

class WrongCommand : public DefaultCommand, public CommandPlugin {
XML_OBJECT
public:
    typedef ::Pointer<WrongCommand> Pointer;

    WrongCommand( const DLString & );

    virtual void run( Character *, const DLString & );

protected:
    XML_VARIABLE XMLString msgWrong;
};

#endif

