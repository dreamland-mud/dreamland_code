/* $Id: cclantalk.h,v 1.1.6.1.10.3 2008/02/23 13:41:00 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CCLANTALK_H
#define CCLANTALK_H

#include "clan.h"
#include "commandplugin.h"

void clantalk( Clan &, const char *, ... );

class CClanTalk : public CommandPlugin {
public:
    typedef ::Pointer<CClanTalk> Pointer;

    CClanTalk( );
    virtual void run( Character *, const DLString & );
    virtual bool visible( Character * ) const;
private:
    static const DLString COMMAND_NAME;
};

#endif

