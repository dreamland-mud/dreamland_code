/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WRAP_UTILS_H
#define WRAP_UTILS_H

#include "wrapperbase.h"
#include "xmlregister.h"
#include "register-decl.h"

using Scripting::Register;
using Scripting::RegisterList;
class Object;
class Character;
class Room;
class Wearlocation;
struct obj_index_data;
struct mob_index_data;

Register wrap( Object * );
Register wrap( struct obj_index_data * );
Register wrap( Character * );
Register wrap( struct mob_index_data * );
Register wrap( Room * );

const Register & get_unique_arg( const RegisterList & );

int args2number( const RegisterList& );
DLString args2string( const RegisterList& );
void args2buf(const RegisterList &args, char *buf, size_t bufsize);

Wearlocation * arg2wearloc( const Register & );
Object * arg2item( const Register & );
Room * arg2room( const Register & );
Character * arg2character( const Register & );

#endif

