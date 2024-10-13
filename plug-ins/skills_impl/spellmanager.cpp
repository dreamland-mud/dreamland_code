/* $Id: spellmanager.cpp,v 1.1.2.3.10.5 2008/05/27 21:30:05 rufina Exp $
 *
 * ruffina, 2004
 */

#include <fstream>

#include "spellmanager.h"
#include "spell.h"
#include "skill.h"

#include "logstream.h"
#include "stringlist.h"
#include "xmldocument.h"
#include "dbio.h"

#include "character.h"
#include "dreamland.h"
#include "def.h"

SpellManager* SpellManager::thisClass = 0;
SpellManager::SpellList SpellManager::spells;
SpellManager::Priorities SpellManager::priorities;
const DLString SpellManager::priorityFile = "spellpriority.xml";

SpellManager::SpellManager( ) 
{
    checkDuplicate( thisClass );
    thisClass = this;
}

SpellManager::~SpellManager( )
{
    thisClass = 0;
}

void SpellManager::initialization( )
{
    loadPriorities( );
}

void SpellManager::destruction( )
{
}

void SpellManager::loadPriorities( )
{
    DLFile pf( dreamland->getTableDir( ), priorityFile );
    
    try {
        std::ifstream ifstr( pf.getPath( ).c_str( ) );
        XMLDocument::Pointer root( NEW );
        
        root->load( ifstr );
        XMLNode::Pointer node = root->getFirstNode( );
        
        if (!node.isEmpty( ))
            priorities.fromXML( node );

    } catch( const Exception& ex ) {
        LogStream::sendError( ) << ex << endl;
    }

    LogStream::sendNotice( )
        << "Loaded " << priorities.size( ) << " spell priorities" << endl;
}

bool SpellManager::compare( Spell::Pointer a, Spell::Pointer b )
{
    int i_a, i_b;
    
    i_a = priorities.getIndexOf( a->getSkill( )->getName( ) );
    i_b = priorities.getIndexOf( b->getSkill( )->getName( ) );

    if (i_a != -1 && i_b != -1)
        return i_a < i_b;
        
    return (i_a != -1);
}

void SpellManager::registrate( Spell::Pointer spell )
{
    spells.push_back( spell );
    spells.sort( compare );
}

void SpellManager::unregistrate( Spell::Pointer spell )
{
    for (SpellList::iterator i = spells.begin( ); i != spells.end( ); i++)
        if (**i == spell.getPointer( )) {
            spells.erase( i );
            break;
        }
}

Spell::Pointer SpellManager::lookup( const DLString &name, Character *ch )
{
    // First do basic lookup by string prefix, so that 'c word' matches 'c word of recall' first,
    // and not 'holy word' (as per priority file).
    for (SpellList::iterator i = spells.begin( ); i != spells.end( ); i++) {
        Skill::Pointer skill = (*i)->getSkill( );
        
        if (!(*i)->isCasted( ))
            continue;
        if (!skill->matchesSubstring(name))
            continue;
        if (!skill->available(ch))
            continue;

        return *i;
    }

    // Do more complicated matching: 'c sup he' matches 'c superior heal'.
    for (SpellList::iterator i = spells.begin( ); i != spells.end( ); i++) {
        Skill::Pointer skill = (*i)->getSkill( );
        
        if (!(*i)->isCasted( ))
            continue;
        if (!skill->matchesUnstrict(name))
            continue;
        if (!skill->available(ch))
            continue;

        return *i;
    }

    return Spell::Pointer( );
}

