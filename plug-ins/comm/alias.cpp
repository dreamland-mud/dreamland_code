/* $Id: alias.cpp,v 1.10.2.12.6.9 2009/09/05 18:30:47 rufina Exp $
 * 
 * ruffina, 2005
 */

#include <sstream>

#include "logstream.h"

#include "xmlmap.h"
#include "xmlstring.h"
#include "class.h"
#include "so.h"

#include "interpretlayer.h"
#include "commandinterpreter.h"
#include "commandplugin.h"
#include "defaultcommand.h"

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "pcharacter.h"
#include "playerattributes.h"

#include "merc.h"
#include "descriptor.h"
#include "loadsave.h"
#include "comm.h"
#include "mercdb.h"
#include "def.h"

/*-----------------------------------------------------------------------------
 * XMLAttributeAliases
 *----------------------------------------------------------------------------*/
class XMLAttributeAliases : public XMLMapBase<XMLString>, public RemortAttribute
{
public:        
        typedef ::Pointer<XMLAttributeAliases> Pointer;

public:
        static const DLString TYPE;

        virtual const DLString & getType( ) const
        {
            return TYPE;
        }
};

const DLString XMLAttributeAliases::TYPE = "XMLAttributeAliases";

/*-----------------------------------------------------------------------------
 * SkippingCommand
 *----------------------------------------------------------------------------*/
class SkippingCommand : public InterpretLayer, public CommandPlugin, public DefaultCommand 
{
public:

    virtual bool process( InterpretArguments &iargs )
    {
        if (getName( ).strPrefix( iargs.line ))
            iargs.advance( );

        return true;
    }


protected:

    virtual void initialization( )
    {
        CommandPlugin::initialization( );
        InterpretLayer::initialization( );
    }

    virtual void destruction( )
    {
        InterpretLayer::destruction( );
        CommandPlugin::destruction( );
    }
};

/*-----------------------------------------------------------------------------
 * 'alias' command 
 *----------------------------------------------------------------------------*/
class CAlias : public SkippingCommand {
public:
    typedef ::Pointer<CAlias> Pointer;

    CAlias( ) 
    {
        this->name = COMMAND_NAME;
    }

    virtual bool properOrder( Character *ch )
    {
        if (IS_CHARMED(ch))
            return false;
        else
            return true;
    }

    virtual void run( Character* ch, const DLString& cArgument )
    {
        PCharacter *pch;
        XMLAttributeAliases::Pointer aliases;
        XMLAttributeAliases::iterator i;
        DLString argument = cArgument, arg;
        
        if (ch->is_npc( )) 
            return;
        
        arg = argument.getOneArgument( );
        
        pch = ch->getPC( );
        aliases = pch->getAttributes( ).getAttr<XMLAttributeAliases>( "aliases" );
        
        if (arg.empty( )) {
            ostringstream buf;
            
            if (aliases->empty( )) {
                pch->send_to( "Не определен ни один синоним.\n\r" );
                return;
            }

            buf << "Определенные синонимы:" << endl;
            
            for (i = aliases->begin( ); i != aliases->end( ); i++)
                buf << "    " << i->first << ":  " << i->second << "{x" << endl;

            pch->send_to( buf );
            return;
        }
        
        if (arg == "flush" || arg == "очистить") {
            aliases->clear( );
            pch->send_to( "Все синонимы удалены.\r\n" );
            return;
        }
        
        if (argument.empty( )) {
            i = aliases->find( arg );
            
            if (i != aliases->end( ))
                pch->printf( "%s означает '%s{x'.\r\n", arg.c_str( ), i->second.getValue( ).c_str( ) );
            else
                pch->send_to( "Этот синоним не задан.\n\r" );

            return;
        }

        if (argument == "delete"
                || argument == "prefix"
                || argument == "alias")
        {
            pch->send_to( "Этим командам нельзя присвоить синоним!\n\r" );
            return;
        }
        
        i = aliases->find( arg );

        if (i != aliases->end( )) // redefine an alias
        {
            i->second.setValue( argument );
            pch->printf( "%s меняет свое значение на '%s{x'.\r\n", arg.c_str( ), argument.c_str( ) );
            return;
        }

        if (aliases->size( ) >= MAX_ALIASES)
        {
            pch->printf( "Извините, Вы превысили лимит синонимов (%d).\n\r", MAX_ALIASES );
            return;
        }

        // make a new alias
        (**aliases) [arg] = argument;

        pch->printf( "%s теперь будет означать '%s{x'.\r\n", arg.c_str( ), argument.c_str( ) );
    }

protected:
    virtual void putInto( )
    {
        interp->put( this, CMDP_SUBST_ALIAS, 1 );
    }

private:
    static const DLString COMMAND_NAME;
    static const unsigned int MAX_ALIASES;
};


const DLString CAlias::COMMAND_NAME = "alias";
const unsigned int CAlias::MAX_ALIASES = 200;

/*-----------------------------------------------------------------------------
 * 'unalias' command 
 *----------------------------------------------------------------------------*/
class CUnalias : public SkippingCommand {
public:
    typedef ::Pointer<CUnalias> Pointer;
    
    CUnalias( ) 
    {
        this->name = COMMAND_NAME;
    }

    virtual void run( Character* ch, const DLString& cArgument ) 
    {
        PCharacter *pch;
        XMLAttributeAliases::Pointer aliases;
        XMLAttributeAliases::iterator i;

        DLString argument = cArgument;
        DLString arg = argument.getOneArgument( );
        
        if (ch->is_npc( )) 
            return;
        
        pch = ch->getPC( );
        aliases = pch->getAttributes( ).getAttr<XMLAttributeAliases>( "aliases" );
        
        if (arg.empty( )) {
            pch->send_to("Какой синоним удалить?\n\r");
            return;
        }
        
        i = aliases->find( arg );
        
        if (i != aliases->end( ))
        {
            pch->send_to( "Синоним удален.\n\r" );
            aliases->erase( i );
        }
        else
        {
            pch->send_to( "Синоним с таким именем не задан.\n\r" );
        }
    }

protected:

    virtual void putInto( )
    {
        interp->put( this, CMDP_SUBST_ALIAS, 2 );
    }
        
private:
    static const DLString COMMAND_NAME;
};

const DLString CUnalias::COMMAND_NAME = "unalias";

/*-----------------------------------------------------------------------------
 * SubstituteAliasInterpretLayer 
 *----------------------------------------------------------------------------*/
class SubstituteAliasInterpretLayer : public InterpretLayer {
public:
    virtual void putInto( )
    {
        interp->put( this, CMDP_SUBST_ALIAS, 10 );
    }

    virtual bool process( InterpretArguments &iargs )
    {
        char buffer[MAX_STRING_LENGTH];
        XMLAttributeAliases::iterator i;
        XMLAttributeAliases::Pointer aliases;
        Character *ch;
        DLString line;

        Descriptor *d = iargs.d;
        ch = d->character;

        aliases = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeAliases>( "aliases" );

        if (!aliases)
            return true;
    
        if (iargs.line.empty( ))
            return true;

        if (iargs.line.at( 0 ) == '\'' || iargs.line.at( 0 ) == '\"')
            return true;

        line = iargs.line;
        i = aliases->find( line.getOneArgument( ) );

        if (i == aliases->end( ))
            return true;
                
        iargs.line = i->second.getValue( );
        if (!line.empty( ))
            iargs.line += " " + line;
        strcpy( buffer, iargs.line.c_str( ) );
        iargs.line = get_multi_command( d, buffer );

        if (iargs.line.size( ) >  MAX_INPUT_LENGTH) {
            iargs.ch->send_to( "Подстановка синонимов слишком удлинила строку, строка будет обрезана.\r\n" );
            iargs.line.erase( MAX_INPUT_LENGTH - 1, iargs.line.size( ) );
        }

        return true;
    }
};


/*-----------------------------------------------------------------------------
 * 'prefix' command 
 *----------------------------------------------------------------------------*/
class CPrefix : public SkippingCommand {
public:
    typedef ::Pointer<CPrefix> Pointer;
    
    CPrefix( ) 
    {
        this->name = COMMAND_NAME;
    }

    virtual void run( Character *ch, const DLString &argument )
    {
        char buf[MAX_INPUT_LENGTH];
        
        if (argument.empty( ))
        {
            if (ch->prefix[0] == '\0')
            {
                ch->send_to("You have no prefix to clear.\r\n");
                return;
            }

            ch->send_to("Prefix removed.\r\n");
            free_string(ch->prefix);
            ch->prefix = str_dup("");
            return;
        }

        if (ch->prefix[0] != '\0')
        {
            sprintf(buf,"Prefix changed to %s.\r\n",argument.c_str( ));
            free_string(ch->prefix);
        }
        else
        {
            sprintf(buf,"Prefix set to %s.\r\n",argument.c_str( ));
        }

        ch->prefix = str_dup(argument.c_str( ));
        ch->send_to( buf );
    }

protected:
    virtual void putInto( )
    {
        interp->put( this, CMDP_PREFIX, 1 );
        interp->put( this, CMDP_SUBST_ALIAS, 3 );
    }
        
private:
    static const DLString COMMAND_NAME;
};

const DLString CPrefix::COMMAND_NAME = "prefix";

/*-----------------------------------------------------------------------------
 * PrefixInterpretLayer 
 *----------------------------------------------------------------------------*/
class PrefixInterpretLayer : public InterpretLayer {
public:

    virtual void putInto( )
    {
        interp->put( this, CMDP_PREFIX, 10 );
    }

    virtual bool process( InterpretArguments &iargs )
    {
        Character *ch = iargs.d->character;
        
        if (ch->prefix[0] != 0) {
            if (strlen( ch->prefix ) + iargs.line.size( ) > MAX_INPUT_LENGTH)
                iargs.ch->send_to("Line to long, prefix not processed.\r\n");
            else
                iargs.line = DLString( ch->prefix ) + " " + iargs.line;
        }

        return true;
    }
};

/*-----------------------------------------------------------------------------
 * libalias initialization 
 *----------------------------------------------------------------------------*/
extern "C"
{
    SO::PluginList initialize_alias( )
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<PrefixInterpretLayer>( ppl );
        Plugin::registerPlugin<SubstituteAliasInterpretLayer>( ppl );
        
        Plugin::registerPlugin<CAlias>( ppl );
        Plugin::registerPlugin<CUnalias>( ppl );
        Plugin::registerPlugin<CPrefix>( ppl );
        
        Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeAliases> >( ppl );
                
        return ppl;
    }
}
