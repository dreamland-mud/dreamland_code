/* $Id: who.h,v 1.1.2.3.10.1 2007/06/26 07:11:42 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef WHO_H
#define WHO_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;

class Who : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
        typedef ::Pointer<Who> Pointer;
    
        Who( );

        virtual void run( Character*, const DLString& constArguments );
        
private:
        DLString formatChar( Character *, PCharacter * );
        DLString leftColumn( Character *, PCharacter * );
        DLString flags( PCharacter * );
        
        static const DLString COMMAND_NAME;
};

#endif

