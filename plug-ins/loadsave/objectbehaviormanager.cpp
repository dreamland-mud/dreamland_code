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
#include "loadsave.h"
#include "fread_utils.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

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
    char *word;
    std::basic_istringstream<char> istr;
    XMLDocument::Pointer doc( NEW );
    
    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
        
    word = fread_string( fp );

    try {
        istr.str( word );
        
        doc->load( istr );
        pObjIndex->behavior = new XMLDocument( **doc );

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
        
    free_string( word );
}

void ObjectBehaviorManager::parse( Object * obj, FILE *fp ) {
    char letter;
    char *word;
    
    if (feof( fp ))
        return;
    
    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
    
    word = fread_string( fp );
    
    try {
        std::basic_istringstream<char> istr( word );

        clear(obj);
        obj->behavior.fromStream( istr );
        obj->behavior->setObj( obj );

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
        
    free_string( word );
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
 * BasicObjectBehavior
 *-----------------------------------------------------------------*/

/** Owned items can't be confiscated by Rulers */
bool BasicObjectBehavior::canConfiscate() 
{
    if (obj->getOwner())
        return false;

    return true;
}

/** Someone else's items disappear on save. */
bool BasicObjectBehavior::save() 
{
    if (!obj->getOwner())
        return false;

    Character *ch = obj->getCarrier( );

    if (!ch || ch->is_immortal( ))
        return false;
    
    if (obj->hasOwner( ch )) 
        return false;
    
    oldact("$o1 исчезает!", ch, obj, 0, TO_CHAR);
    extract_obj(obj);
    return true;    
}

/** Owned items disappear when character is deleted. */
void BasicObjectBehavior::delete_(Character *ch) 
{
    if (obj->hasOwner( ch )) 
        extract_obj( obj );    
}

/** Owned items can't be stolen. */
bool BasicObjectBehavior::canSteal(Character *) 
{
    if (obj->getOwner())
        return true;
    
    return false;
}

/** Owned items can't be picked up from the floor or container */
void BasicObjectBehavior::get( Character *ch ) 
{ 
    canEquip(ch);
}

/** Owned items can't be equipped */
bool BasicObjectBehavior::canEquip(Character *ch) 
{
    if (!obj->getOwner())
        return true;

    if (ch->is_immortal())
        return true;
    
    if (!obj->hasOwner( ch )) {
        ch->pecho( "Ты не можешь владеть %1$O5 и бросаешь %1$P2.", obj );
        ch->recho("%2$^C1 не может владеть %1$O5 и бросает %1$P2.", obj, ch);
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }

    return true;
}

