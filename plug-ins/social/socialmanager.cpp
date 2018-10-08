/* $Id: socialmanager.cpp,v 1.1.2.2.6.1 2007/06/26 07:21:13 rufina Exp $
 *
 * ruffina, 2004
 */
/*
 *
 * sturm, 2003
 */
#include "socialmanager.h"
#include "social.h"
#include "commandinterpreter.h"

#include "class.h"

const DLString SocialManager::NODE_NAME = "Social";
const DLString SocialManager::TABLE_NAME = "socials";
SocialManager * SocialManager::thisClass = 0;

SocialManager::SocialManager( )
{
    thisClass = this;
    Class::regMoc<Social>( );
}

SocialManager::~SocialManager( )
{
    Class::unregMoc<Social>( );
    thisClass = 0;
}

DLString SocialManager::getNodeName( ) const 
{
    return NODE_NAME;
}

DLString SocialManager::getTableName( ) const 
{
    return TABLE_NAME;
}


static bool compare( XMLTableElement::Pointer a, XMLTableElement::Pointer b )
{
    return a->getName( ) <= b->getName( );
}

void SocialManager::initialization( )
{
    readAll( );
    loadAll( );
    elements.sort( compare );
    InterpretLayer::initialization( );
}

void SocialManager::destruction( )
{
    InterpretLayer::destruction( );
//    saveAll( );
    unloadAll( );
}

void SocialManager::putInto( )
{
    interp->put( this, CMDP_FIND, 20 );        
}

bool SocialManager::process( InterpretArguments &iargs )
{
    LoadedList::iterator e;
    DLString cmd = iargs.cmdName;
    
    if (!cmd.empty( ) && cmd.at( 0 ) == '*')
        cmd.erase( 0, 1 );
        
    for (e = elements.begin( ); e != elements.end( ); e++) {
        Social *social = e->getStaticPointer<Social>( );

        if (social->matches( cmd )) {
            iargs.pCommand.setPointer( social );
            iargs.cmdName = cmd;
            iargs.advance( );
            break;
        }
    }

    return true;
}
