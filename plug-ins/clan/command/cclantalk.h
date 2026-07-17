/* $Id: cclantalk.h,v 1.1.6.1.10.3 2008/02/23 13:41:00 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CCLANTALK_H
#define CCLANTALK_H

#include "clan.h"
#include "commandplugin.h"

class MultiMessage;

void clantalk( Clan &, const DLString &message );
/* Trilinguality (Trello 2594): resolve the broadcast body per clan member, so a
 * _()-wrapped message renders in each recipient's display language. Trailing
 * printf args (e.g. a %s name) are applied per recipient via vfmt. The const
 * DLString& twin above stays byte-identical for unwrapped callers. */
void clantalk( Clan &, const MultiMessage &message, ... );

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

