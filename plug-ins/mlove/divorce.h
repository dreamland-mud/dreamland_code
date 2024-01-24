/* $Id: divorce.h,v 1.1.2.5.6.1 2007/06/26 07:18:03 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef DIVORCE_H
#define DIVORCE_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;

class Divorce : public CommandPlugin {
public:
        typedef ::Pointer<Divorce> Pointer;
    
        Divorce( );

        virtual void run( Character*, const DLString& constArguments );
        
private:
        void divorce( PCharacter *, DLString );
        void divorceWidow( PCharacter * );
        PCharacter * checkBride( Character *, DLString );
        
        static const DLString COMMAND_NAME;
};

#endif

