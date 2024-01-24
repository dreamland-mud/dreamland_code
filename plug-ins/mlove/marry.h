/* $Id: marry.h,v 1.1.2.4.6.1 2007/06/26 07:18:03 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef MARRY_H
#define MARRY_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;

class Marry : public CommandPlugin {
public:
        typedef ::Pointer<Marry> Pointer;
    
        Marry( );

        virtual void run( Character*, const DLString& constArguments );
private:
        PCharacter *checkBride( Character *, DLString );

        static const DLString COMMAND_NAME;
};

#endif

