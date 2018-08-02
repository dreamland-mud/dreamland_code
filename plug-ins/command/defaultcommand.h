/* $Id: defaultcommand.h,v 1.1.2.8 2009/09/07 14:04:21 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef	DEFAULTCOMMAND_H
#define	DEFAULTCOMMAND_H

#include "xmlvariablecontainer.h"
#include "xmlshort.h"
#include "xmlboolean.h"
#include "xmlvector.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "xmlenumeration.h"
#include "xmlpointer.h"

#include "commandhelp.h"
#include "command.h"

class DefaultCommand : public virtual Command, public virtual XMLVariableContainer {
XML_OBJECT;
public:
	typedef ::Pointer<DefaultCommand> Pointer;

	DefaultCommand( );
	virtual ~DefaultCommand( );
	
	virtual const DLString& getName( ) const;
	virtual const DLString& getRussianName( ) const;
	virtual const XMLStringList &getAliases( ) const;
	virtual const XMLStringList &getRussianAliases( ) const;
	virtual const DLString & getHint( ) const;
	virtual ::Pointer<CommandHelp> getHelp( ) const;
	virtual short getLog( ) const;
	virtual const Flags & getExtra( ) const;
	virtual short getLevel( ) const;
	virtual const Enumeration & getPosition( ) const;
	virtual const Flags & getOrder( ) const;

	virtual bool matches( const DLString & ) const;
	virtual bool matchesExactly( const DLString & ) const;

	virtual void run( Character * ch, const DLString & );
	virtual void run( Character *, char * );

protected:	
	XML_VARIABLE XMLString name;
	XML_VARIABLE XMLStringList aliases, russian;
	XML_VARIABLE XMLFlagsNoEmpty extra;
	XML_VARIABLE XMLShortNoEmpty level;
	XML_VARIABLE XMLShortNoEmpty log;
	XML_VARIABLE XMLEnumeration position;
	XML_VARIABLE XMLFlagsNoEmpty order; 
	XML_VARIABLE XMLStringNoEmpty hint;
	XML_VARIABLE XMLPointerNoEmpty<CommandHelp> help;
};


inline const DLString& DefaultCommand::getName( ) const
{
    return name.getValue( );
}
inline const Flags & DefaultCommand::getExtra( ) const
{
    return extra;
}
inline short DefaultCommand::getLevel( ) const
{
    return level.getValue( );
}
inline short DefaultCommand::getLog( ) const
{
    return log.getValue( );
}
inline const Enumeration & DefaultCommand::getPosition( ) const
{
    return position;
}
inline const Flags & DefaultCommand::getOrder( ) const
{
    return order;
}
inline const DLString& DefaultCommand::getHint( ) const
{
    return hint.getValue( );
}

inline ::Pointer<CommandHelp> DefaultCommand::getHelp( ) const
{
    return help;
}

#endif
