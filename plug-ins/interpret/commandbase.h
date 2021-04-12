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

// Error codes for failed command dispatch attempt.
enum {
    RC_DISPATCH_OK = 0,
    RC_DISPATCH_AFK,
    RC_DISPATCH_SPELLOUT,
    RC_DISPATCH_STUN,
    RC_DISPATCH_GHOST,
    RC_DISPATCH_NOT_HERE,
    RC_DISPATCH_POSITION,
    RC_DISPATCH_PENALTY,
    RC_DISPATCH_CHARMED
};

// Error codes for failed order attempt.
enum {
    RC_ORDER_OK = 0,
    RC_ORDER_ERROR,
    RC_ORDER_NOT_FIGHTING,
    RC_ORDER_NOT_PLAYER,
    RC_ORDER_NOT_THIEF
};

class CommandBase: public virtual DLObject {
public:
        typedef ::Pointer<CommandBase> Pointer;

        virtual const DLString& getName( ) const = 0;
        virtual short getLog( ) const = 0;

        virtual bool matches( const DLString & ) const = 0;
        virtual int properOrder( Character * ) const = 0;
        virtual int dispatch( const InterpretArguments & ) = 0;
        virtual int dispatchOrder( const InterpretArguments & ) = 0;
        virtual void run( Character *, const DLString & ) = 0;
};


#endif
