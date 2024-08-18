
/* $Id: twitlist.h,v 1.1.2.1.8.1 2007/06/26 07:12:08 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __TWITLIST_H__ 
#define __TWITLIST_H__ 

#include "commandplugin.h"

class PCharacter;

class CTwit : public CommandPlugin {
public:
    typedef ::Pointer<CTwit> Pointer;

    CTwit( );

    virtual void run( Character *, const DLString & );
    
private:
    void doAdd( PCharacter *, DLString & );
    void doRemove( PCharacter *, DLString & );
    void doList( PCharacter * );
    void doUsage( PCharacter * );

    static const DLString COMMAND_NAME;
};


/** Returns true if player ch doesn't want to hear from this person. */
bool talker_is_ignored( PCharacter *ch, PCharacter *talker );

#endif

