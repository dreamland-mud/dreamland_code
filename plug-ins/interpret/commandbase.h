/* $Id: commandbase.h,v 1.1.2.1.6.2 2008/02/24 17:24:27 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef        COMMANDBASE_H
#define        COMMANDBASE_H

#include "interpretlayer.h"

// Command logging types.
enum {
    LOG_NORMAL = 0,
    LOG_ALWAYS,
    LOG_NEVER
};

class CommandBase: public virtual DLObject {
public:
        typedef ::Pointer<CommandBase> Pointer;

        virtual const DLString& getName( ) const = 0;
        virtual short getLog( ) const = 0;

        virtual bool matches( const DLString & ) const = 0;
        virtual bool properOrder( Character * ) const = 0;
        virtual bool dispatch( const InterpretArguments & ) = 0;
        virtual bool dispatchOrder( const InterpretArguments & ) = 0;
        virtual void run( Character *, const DLString & ) = 0;
};


#endif
