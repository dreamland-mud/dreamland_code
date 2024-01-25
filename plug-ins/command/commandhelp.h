/* $Id$
 *
 * ruffina, 2004
 */
#ifndef COMMANDHELP_H
#define COMMANDHELP_H

#include "markuphelparticle.h"
#include "helpformatter.h"

class Command;

class CommandHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<CommandHelp> Pointer;
    typedef ::Pointer<Command> CommandPointer;

    virtual bool visible( Character * ) const;
    virtual void setCommand( CommandPointer );
    virtual void unsetCommand( );
    virtual void save() const;
    
    virtual DLString getTitle(const DLString &label) const;
    inline CommandPointer getCommand( ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

    // Find help article referenced by 'refby' field
    CommandHelp::Pointer getReferencedBy();
    
protected:
    virtual void applyFormatter( Character *, ostringstream &, ostringstream & ) const;

    CommandPointer command;
};

inline ::Pointer<Command> CommandHelp::getCommand( ) const
{
    return command;
}

inline const DLString & CommandHelp::getType( ) const
{
    return TYPE;
}

class CommandHelpFormatter : public HelpFormatter {
public:
    CommandHelpFormatter( const char *, ::Pointer<Command> );
    virtual ~CommandHelpFormatter( );

protected:
    virtual void reset( );
    virtual void setup( Character * );
    virtual bool handleKeyword( const DLString &, ostringstream & );

    ::Pointer<Command> cmd;
    bool fRusCmd;
};

#endif
