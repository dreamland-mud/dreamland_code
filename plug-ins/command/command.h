/* $Id: command.h,v 1.1.2.9.6.7 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef        COMMAND_H
#define        COMMAND_H

#include "commandbase.h"
#include "flags.h"
#include "enumeration.h"
#include "xmlstringlist.h"
#include "xmlpolymorphvariable.h"
#include "wrappertarget.h"

class CommandHelp;
class CommandLoader;

class Command : public CommandBase, public virtual WrapperTarget {
public:
        typedef ::Pointer<Command> Pointer;

        Command( );
        virtual ~Command( );

        virtual long long getID() const;

        // Main entry point for command interpreter
        virtual void entryPoint( Character *, const DLString & );
        // Legacy run methods overridden by child classes
        virtual void run( Character *, const DLString & ) = 0;
        
        virtual const DLString & getRussianName( ) const;
        virtual const XMLStringList &getAliases( ) const;
        virtual const XMLStringList &getRussianAliases( ) const;
        virtual const DLString & getHint( ) const;
        virtual ::Pointer<CommandHelp> getHelp( ) const;
        virtual short getLog( ) const;
        virtual const Flags & getExtra( ) const;
        virtual short getLevel( ) const;
        virtual const Enumeration & getPosition( ) const;
        virtual const Flags & getOrder( ) const;
        virtual const Flags & getCommandCategory( ) const;

        virtual bool matches( const DLString & ) const;
        virtual bool matchesAlias( const DLString & ) const;
        virtual bool matchesExactly( const DLString & ) const;
        virtual int properOrder( Character * ) const;
        virtual int dispatchOrder( const InterpretArguments & );
        virtual int dispatch( const InterpretArguments & );

        virtual bool available( Character * ) const;
        virtual bool visible( Character * ) const;

protected:        
        void visualize( Character * );
        bool checkPosition( Character * );
        bool feniaOverride(Character *, const DLString &);

        static const Flags defaultOrder;
        static const Enumeration defaultPosition;
        static const Flags defaultExtra;
        static const Flags defaultCategory;
};

class XMLCommand : public virtual Command, public virtual XMLPolymorphVariable {
public:
        typedef ::Pointer<XMLCommand> Pointer;

        XMLCommand( );
        virtual ~XMLCommand( );

        virtual CommandLoader * getLoader( ) const;
};

#endif
