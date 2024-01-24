/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ADMIN_COMMAND_H
#define ADMIN_COMMAND_H

#include "commandtemplate.h"
#include "commandmanager.h"

class AdminCommand : public CommandPlugin {
public:
        virtual CommandLoader * getLoader( ) const;
};

CMDLOADER_EXTERN(admin)

#define CMDADM(x) \
CMD_DECL(x) \
template <> CommandLoader * CMD(x)::getLoader( ) const { \
    return CMDLOADER(admin)::getThis( ); \
} \
template <> void CMD(x)::run( Character* ch, const DLString &constArguments ) 

#endif
