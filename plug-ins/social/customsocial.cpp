/* $Id: customsocial.cpp,v 1.1.2.1.6.7 2008/05/27 21:30:05 rufina Exp $
 *
 * ruffina, 2005
 */

#include "customsocial.h"
#include "commandinterpreter.h"

#include "logstream.h"
#include "pcharacter.h"

#include "dreamland.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*----------------------------------------------------------------------
 * CustomSocialManager 
 *---------------------------------------------------------------------*/
CustomSocialManager::CustomSocialManager( )
{
    Class::regMoc<CustomSocial>( );
}

CustomSocialManager::~CustomSocialManager( )
{
    Class::unregMoc<CustomSocial>( );
}

void CustomSocialManager::putInto( )
{
    interp->put( this, CMDP_FIND, 30 );
}

bool CustomSocialManager::process( InterpretArguments &iargs )
{
    XMLAttributeCustomSocials::Pointer mysocials;

    if (iargs.ch->is_npc( )) {
	if (IS_AFFECTED(iargs.ch, AFF_CHARM) && iargs.ch->master) {
	    Character *orig = iargs.ch;

	    iargs.ch = iargs.ch->master;
	    process( iargs );
	    iargs.ch = orig;
	}

	return true;
    }

    mysocials = iargs.ch->getPC( )->getAttributes( ).findAttr<XMLAttributeCustomSocials>( "socials" );

    if (mysocials) {
	DLString cmd = iargs.cmdName;
	
	if (!cmd.empty( ) && cmd.at( 0 ) == '*') 
	    cmd.erase( 0, 1 );
	
	iargs.pCommand = mysocials->chooseSocial( cmd );

	if (iargs.pCommand) {
	    iargs.cmdName = cmd;
	    iargs.advance( );
	}
    }

    return true;
}

/*----------------------------------------------------------------------
 * CustomSocial 
 *---------------------------------------------------------------------*/
CustomSocial::CustomSocial( ) 
{
}

CustomSocial::~CustomSocial( )
{
}

int CustomSocial::getPosition( ) const
{
    return POS_RESTING;
}

void CustomSocial::reaction( Character *ch, Character *victim, const DLString &arg )
{
    if (!victim && !arg.empty( )) {
	if (!getErrorMsg( ).empty( ))
	    act_p( getErrorMsg( ).c_str( ), ch, 0, 0, TO_CHAR, getPosition( ) );
	else
	    ch->println("Нет этого здесь.");
    }
}

const DLString& CustomSocial::getName( ) const 
{
    return name.getValue( );
}
const DLString& CustomSocial::getRussianName( ) const 
{
    return rusName.getValue( );
}
const DLString & CustomSocial::getNoargOther( ) const
{
    return noargOther.getValue( );
}
const DLString & CustomSocial::getNoargMe( ) const
{
    return noargMe.getValue( );
}
const DLString & CustomSocial::getArgMe( ) const
{
    return argMe.getValue( );
}
const DLString & CustomSocial::getArgOther( ) const
{
    return argOther.getValue( );
}
const DLString & CustomSocial::getArgVictim( ) const
{
    return argVictim.getValue( );
}
const DLString & CustomSocial::getAutoMe( ) const
{
    return autoMe.getValue( );
}
const DLString & CustomSocial::getAutoOther( ) const
{
    return autoOther.getValue( );
}
const DLString & CustomSocial::getErrorMsg( ) const
{
    static const DLString emptyString;
    return emptyString;
}


/*----------------------------------------------------------------------
 * XMLAttributeCustomSocials 
 *---------------------------------------------------------------------*/
const DLString XMLAttributeCustomSocials::TYPE = "XMLAttributeCustomSocials";

XMLAttributeCustomSocials::XMLAttributeCustomSocials( ) 
{
}

CustomSocial::Pointer XMLAttributeCustomSocials::chooseSocial( const DLString &cmd )
{
    iterator i;
    
    for (i = begin( ); i != end( ); i++) 
	if (i->second->matches( cmd )) 
	    return i->second;
    
    return CustomSocial::Pointer( );
}

CustomSocial::Pointer XMLAttributeCustomSocials::getSocial( const DLString &name )
{
    iterator i = find( name );

    if (i == end( )) {
	CustomSocial::Pointer social( NEW );

	social->setName( name );
	insert( make_pair( name, XMLCustomSocial( social ) ) );
	return social;
    }
    else
	return i->second;
}
