/* $Id: command.h,v 1.1.2.9.6.7 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef        COMMAND_H
#define        COMMAND_H

#include "xmlvariablecontainer.h"
#include "xmlshort.h"
#include "xmlboolean.h"
#include "xmlvector.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "xmlenumeration.h"
#include "xmlpointer.h"

#include "commandbase.h"
#include "commandhelp.h"

class CommandHelp;
class CommandLoader;

class Command : public CommandBase, public virtual XMLVariableContainer  {
XML_OBJECT
public:
        typedef ::Pointer<Command> Pointer;

        Command( );
        virtual ~Command( );

        // Main entry point for command interpreter
        virtual void entryPoint( Character *, const DLString & );
        // Legacy run methods overridden by child classes
        virtual void run( Character *, const DLString & ) = 0;
        virtual void run( Character *, char * );

        // Saves command XML profile (or enclosing XML profile) to disk
        virtual bool saveCommand() const = 0;

        virtual const DLString& getName( ) const;
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

        XML_VARIABLE XMLString name;
        XML_VARIABLE XMLStringList aliases, russian;
        XML_VARIABLE XMLFlagsNoEmpty extra;
        XML_VARIABLE XMLShortNoEmpty level;
        XML_VARIABLE XMLShortNoEmpty log;
        XML_VARIABLE XMLEnumeration position;
        XML_VARIABLE XMLFlagsNoEmpty order; 
        XML_VARIABLE XMLStringNoEmpty hint;
        XML_VARIABLE XMLPointerNoEmpty<CommandHelp> help;
        XML_VARIABLE XMLFlags cat;

protected:        
        void visualize( Character * );
        bool checkPosition( Character * );
};




#endif
