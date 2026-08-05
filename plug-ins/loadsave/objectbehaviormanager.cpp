/* $Id: objectbehaviormanager.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#include "objectbehaviormanager.h"
#include "objectbehavior.h"

#include "xmldocument.h"
#include "logstream.h"
#include "core/object.h"
#include "character.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "loadsave.h"
#include "fread_utils.h"
#include "act.h"

#include "merc.h"
#include "def.h"
#include "l10n.h"

void ObjectBehaviorManager::assign( Object *obj ) {
    if (!obj->pIndexData->behavior) 
        return;
    
    try {
        clear(obj);
        obj->behavior.fromXML( obj->pIndexData->behavior->getFirstNode( ) );
        obj->behavior->setObj( obj );

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void ObjectBehaviorManager::assign( Object *obj, const DLString &behaviorClassName ) 
{
    try {
        AllocateClass::Pointer p = Class::allocateClass(behaviorClassName);

        if (p) {
            clear(obj);
            obj->behavior.setPointer( p.getDynamicPointer<ObjectBehavior>( ) );
            obj->behavior->setObj( obj );
        }
    } catch (const ExceptionClassNotFound &e) {
        LogStream::sendError( ) << "Error allocating obj behavior " << behaviorClassName << ":" << e.what( ) << endl;
        return;
    }
}

void ObjectBehaviorManager::assignBasic( Object *obj ) 
{
    static const DLString basicName( "BasicObjectBehavior" );

    if (obj->behavior) {
        obj->behavior->unsetObj( );
        obj->behavior.clear( );
    }

    try {
        AllocateClass::Pointer pointer = Class::allocateClass( basicName );
        ObjectBehavior::Pointer behavior = pointer.getDynamicPointer<ObjectBehavior>( );
        
        if (!behavior)
            throw Exception( "BasicObjectBehavior is not derived from ObjectBehavior" );
        
        obj->behavior.setPointer( *behavior );
        obj->behavior->setObj( obj );
    }
    catch (const ExceptionClassNotFound &e ) {
    }
}


void ObjectBehaviorManager::clear( Object *obj ) 
{
    if (obj->behavior) {
        obj->behavior->unsetObj( );
        obj->behavior.clear( );
    }
}

void ObjectBehaviorManager::parse( OBJ_INDEX_DATA * pObjIndex, FILE *fp ) {
    char letter;
    DLString word;
    std::basic_istringstream<char> istr;
    XMLDocument::Pointer doc( NEW );
    
    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
        
    word = fread_dlstring( fp );

    try {
        istr.str( word );
        
        doc->load( istr );
        pObjIndex->behavior = new XMLDocument( **doc );

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
        
}

void ObjectBehaviorManager::parse( Object * obj, FILE *fp ) {
    char letter;
    DLString word;
    
    if (feof( fp ))
        return;
    
    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
    
    word = fread_dlstring( fp );
    
    try {
        std::basic_istringstream<char> istr( word );

        clear(obj);
        obj->behavior.fromStream( istr );
        obj->behavior->setObj( obj );

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void ObjectBehaviorManager::save( const OBJ_INDEX_DATA *pObjIndex, FILE *fp ) {
    std::basic_ostringstream<char> ostr;
     
    if (!pObjIndex->behavior)
        return;
    
    try {
        pObjIndex->behavior->save( ostr );
        fprintf( fp, "%s~\n", ostr.str( ).c_str( ) );

    } catch (const ExceptionXMLError &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void ObjectBehaviorManager::save( const Object *obj, FILE *fp ) {
    std::basic_ostringstream<char> ostr;
     
    if (!obj->behavior)
        return;
    
    try {
        obj->behavior.toStream( ostr );
        fprintf( fp, "%s~\n", ostr.str( ).c_str( ) );

    } catch (const ExceptionXMLError &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

/*-----------------------------------------------------------------
 * Ownership, keyed on the owner field alone
 *-----------------------------------------------------------------*/

/*
 * These rules used to live in BasicObjectBehavior, which meant they only ever
 * applied to items whose single behavior slot happened to hold one of its
 * descendants. Ownership is handed out from all over the place -- Fenia scripts
 * (obj.owner = ch.name), clan equipment, quest traders -- and most of those
 * leave the slot empty, so most named items in the game obeyed no rules at all.
 * Keying on the field instead covers every one of them, and survives the
 * eventual removal of C++ behaviors.
 */
bool obj_owner_allows( Object *obj, Character *ch )
{
    if (!obj || !ch)
        return true;

    if (obj->getOwner().empty())
        return true;

    // Corpses carry their owner's name but loot rules of their own, enforced by
    // the get and fetch commands: killer, group, full loot.
    if (obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC)
        return true;

    if (ch->is_immortal())
        return true;

    if (obj->hasOwner( ch ))
        return true;

    // Playing with full loot on waives ownership towards everyone else.
    PCMemoryInterface *pcm = PCharacterManager::find( obj->getOwner() );
    if (pcm && pcm->getAttributes( ).isAvailable( "fullloot" ))
        return true;

    return false;
}

/*
 * Someone is holding an item they may not own: put it back on the floor.
 * Mobs are turned away without a word -- they have no business carrying named
 * items, and a scavenger announcing it in every room would be noise.
 */
bool obj_owner_enforce( Object *obj, Character *ch )
{
    if (obj_owner_allows( obj, ch ))
        return true;

    // A trigger further up may already have taken the item off our hands; never
    // unlink an object from a character that is not holding it.
    if (obj->carried_by != ch || !ch->in_room)
        return false;

    if (!ch->is_npc()) {
        ch->pecho( _("Ты не можешь владеть %1$O5 и бросаешь %1$P2."), obj );
        ch->recho( _("%2$^C1 не может владеть %1$O5 и бросает %1$P2."), obj, ch );
    }

    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    return false;
}

/*-----------------------------------------------------------------
 * BasicObjectBehavior
 *-----------------------------------------------------------------*/

/** Owned items can't be confiscated by Rulers */
bool BasicObjectBehavior::canConfiscate() 
{
    if (!obj->getOwner().empty())
        return false;

    return true;
}

/*
 * The ownership rules that used to live here -- save, delete_, canSteal, get,
 * canEquip, checkOwnership -- are now obj_owner_allows/obj_owner_enforce above,
 * called from the generic get, wear and save paths so that every named item is
 * covered and not just the ones carrying a behavior. canSteal and delete_ had
 * no dispatch left at all: steal moved to Fenia and asks obj.owner directly.
 */

