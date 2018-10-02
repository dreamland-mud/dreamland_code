/* $Id$
 *
 * ruffina, 2004
 */
#ifndef COMMANDHELP_H
#define COMMANDHELP_H

#include "markuphelparticle.h"
#include "command.h"
#include "helpformatter.h"

class CommandHelp : public virtual XMLHelpArticle, public virtual MarkupHelpArticle {
public:
    typedef ::Pointer<CommandHelp> Pointer;
    typedef ::Pointer<Command> CommandPointer;

    virtual bool visible( Character * ) const;
    virtual bool toXML( XMLNode::Pointer& ) const;

    virtual void setCommand( CommandPointer );
    virtual void unsetCommand( );
    
    inline CommandPointer getCommand( ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    virtual void applyFormatter( Character *, ostringstream &, ostringstream & ) const;

    CommandPointer command;
};

inline Command::Pointer CommandHelp::getCommand( ) const
{
    return command;
}

inline const DLString & CommandHelp::getType( ) const
{
    return TYPE;
}

class CommandHelpFormatter : public HelpFormatter {
public:
    CommandHelpFormatter( const char *, Command::Pointer );
    virtual ~CommandHelpFormatter( );

protected:
    virtual void reset( );
    virtual void setup( Character * );
    virtual bool handleKeyword( const DLString &, ostringstream & );

    Command::Pointer cmd;
    bool fRusCmd;
};

#endif
