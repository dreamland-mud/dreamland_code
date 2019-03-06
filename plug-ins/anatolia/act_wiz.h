/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ACT_WIZ_H
#define ACT_WIZ_H

#include "commandtemplate.h"
#include "commandmanager.h"

CMDLOADER_EXTERN(wizard)

#define CMDWIZ(x) \
CMD_DECL(x) \
template <> CommandLoader * CMD(x)::getLoader( ) const { \
    return CMDLOADER(wizard)::getThis( ); \
} \
template <> void CMD(x)::run( Character* ch, const DLString &constArguments ) 

#define CMDWIZP(x) \
CMD_DECL(x) \
template <> CommandLoader * CMD(x)::getLoader( ) const { \
    return CMDLOADER(wizard)::getThis( ); \
} \
template <> void CMD(x)::run( Character* ch, char *argument ) 

#endif
