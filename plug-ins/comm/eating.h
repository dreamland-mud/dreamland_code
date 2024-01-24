/* $Id$
 *
 * ruffina, 2004
 */
#ifndef EATING_H
#define EATING_H

#include "commandplugin.h"
#include "defaultcommand.h"

class NPCharacter;

class CEat : public CommandPlugin {
XML_OBJECT
public:
    typedef ::Pointer<CEat> Pointer;

    CEat( );

    virtual void run( Character *, const DLString & );
        
private:
    void eatFood( Character *, int, int, int );
    void eatCarnivoro( Character *, NPCharacter * );
    
    static const DLString COMMAND_NAME;
};

#endif


