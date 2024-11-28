/* $Id: alias.cpp,v 1.10.2.12.6.9 2009/09/05 18:30:47 rufina Exp $
 * 
 * ruffina, 2005
 */

#include <sstream>
#include <string.h>

#include "logstream.h"

#include "xmlmap.h"
#include "xmlstring.h"
#include "class.h"
#include "so.h"
#include "commandtemplate.h"
#include "interpretlayer.h"
#include "commandinterpreter.h"
#include "commandplugin.h"

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "pcharacter.h"
#include "playerattributes.h"
#include "commonattributes.h"
#include "merc.h"
#include "descriptor.h"
#include "loadsave.h"
#include "comm.h"

#include "def.h"

const unsigned int MAX_ALIASES = 1000;

/*-----------------------------------------------------------------------------
 * XMLAttributeAliases
 *----------------------------------------------------------------------------*/
class XMLAttributeAliases : public XMLStringMapAttribute, public RemortAttribute
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
 * 'alias' command 
 *----------------------------------------------------------------------------*/
CMDRUN(alias) 
{
    PCharacter *pch;
    XMLAttributeAliases::Pointer aliases;
    XMLAttributeAliases::iterator i;
    DLString argument = constArguments, arg;
    
    if (ch->is_npc( )) 
        return;
    
    arg = argument.getOneArgument( );
    
    pch = ch->getPC( );
    aliases = pch->getAttributes( ).getAttr<XMLAttributeAliases>( "aliases" );
    
    if (arg.empty( )) {
        ostringstream buf;
        
        if (aliases->empty( )) {
            pch->pecho("Не определен ни один синоним.");
            return;
        }

        buf << "Определенные синонимы:" << endl;
        
        for (i = aliases->begin( ); i != aliases->end( ); i++)
            buf << "    " << i->first << ":  " << i->second << "{x" << endl;

        pch->send_to( buf );
        return;
    }
    
    if (arg_is_strict(arg, "flush")) {
        aliases->clear( );
        pch->pecho("Все синонимы удалены.");
        return;
    }
    
    if (argument.empty( )) {
        i = aliases->find( arg );
        
        if (i != aliases->end( ))
            pch->pecho( "%s означает '%s{x'.", arg.c_str( ), i->second.getValue( ).c_str( ) );
        else
            pch->pecho("Этот синоним не задан.");

        return;
    }

    i = aliases->find( arg );

    if (i != aliases->end( )) // redefine an alias
    {
        i->second.setValue( argument );
        pch->pecho( "%s меняет свое значение на '%s{x'.", arg.c_str( ), argument.c_str( ) );
        return;
    }

    if (aliases->size( ) >= MAX_ALIASES)
    {
        pch->pecho( "Лимит синонимов (%d) уже превышен.", MAX_ALIASES );
        return;
    }

    // make a new alias
    (**aliases) [arg] = argument;

    pch->pecho( "%s теперь будет означать '%s{x'.", arg.c_str( ), argument.c_str( ) );
}



/*-----------------------------------------------------------------------------
 * 'unalias' command 
 *----------------------------------------------------------------------------*/
CMDRUN(unalias)
{
    PCharacter *pch;
    XMLAttributeAliases::Pointer aliases;
    XMLAttributeAliases::iterator i;

    DLString argument = constArguments;
    DLString arg = argument.getOneArgument( );
    
    if (ch->is_npc( )) 
        return;
    
    pch = ch->getPC( );
    aliases = pch->getAttributes( ).getAttr<XMLAttributeAliases>( "aliases" );
    
    if (arg.empty( )) {
        pch->pecho("Какой синоним удалить?");
        return;
    }
    
    i = aliases->find( arg );
    
    if (i != aliases->end( ))
    {
        pch->pecho("Синоним удален.");
        aliases->erase( i );
    }
    else
    {
        pch->pecho("Синоним с таким именем не задан.");
    }
}


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
            iargs.ch->pecho("Подстановка синонимов слишком удлинила строку, строка будет обрезана.");
            iargs.line.erase( MAX_INPUT_LENGTH - 1, iargs.line.size( ) );
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

        Plugin::registerPlugin<SubstituteAliasInterpretLayer>( ppl );
        
        Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeAliases> >( ppl );
                
        return ppl;
    }
}
