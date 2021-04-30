/* $Id$
 *
 * ruffina, 2004
 */
#include "loadsave.h"
#include "logstream.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "behavior_utils.h"
#include "npcharacter.h"
#include "npcharactermanager.h"
#include "core/object.h"
#include "objectmanager.h"
#include "room.h"

#include "merc.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(float);

/*
 * Extract an object consider limit
 */
void extract_obj( Object *obj, const char *message )
{
  extract_obj_1(obj,true, message);
}

/*
 * Extract an object consider limit
 */
void extract_obj_nocount( Object *obj )
{
  extract_obj_1(obj,false);
}

bool oprog_extract( Object *obj, bool count ) 
{
    FENIA_CALL( obj, "Extract", "i", count )
    FENIA_NDX_CALL( obj, "Extract", "Oi", obj, count )
    obj->extractWrapper(count);
    BEHAVIOR_CALL( obj, extract, count )
    return false;
}

/*
 * Extract an obj from the world.
 */
void extract_obj_1( Object *obj, bool count, const char *message )
{
        Object *obj_content;
        Object *obj_next;
        char buf[MAX_STRING_LENGTH];

        if (obj->extracted)  /* if the object has already been extracted once */
        {
                sprintf(buf, "Warning! Extraction of %s, vnum %d.", obj->getName( ),
                        obj->pIndexData->vnum);
                bug(buf, 0);
                return; /* if it's already been extracted, something bad is going on */
        }
        else
                obj->extracted = true;  /* if it hasn't been extracted yet, now
                                                                                                                 * it's being extracted. */
        
        if (IS_SET(obj->pIndexData->area->area_flag, AREA_NOSAVEDROP)
            || IS_SET(obj->extra_flags, ITEM_NOSAVEDROP)) 
        {
            count = true;
        }

        // Remove obj from all locations, showing an optional message.
        if (obj->in_room != 0) {
            if (message)
                obj->in_room->echo(POS_RESTING, message, obj);
            obj_from_room(obj);

        } else if (obj->carried_by != 0) {
            Character *carrier = obj->carried_by;
            bool echoAround = (obj->wear_loc == wear_float);
            
            obj_from_char(obj);

            if (message) {
                carrier->pecho(message, obj);
                if (echoAround)
                    carrier->recho(message, obj);
            }

        } else if (obj->in_obj != 0) {
            obj_from_obj(obj);
        }

        for (obj_content = obj->contains; obj_content; obj_content = obj_next) {
            obj_next = obj_content->next_content;
            extract_obj_1(obj_content, count);
        }

        obj_from_list( obj );

        if (count)
                --obj->pIndexData->count;

        oprog_extract( obj, count );
        obj->pIndexData->instances.remove(obj);
        ObjectManager::extract( obj );

        return;
}

/*--------------------------------------------------------------
 * character extraction
 *--------------------------------------------------------------*/
/*
 * Extract плоходропнутого моба >8)
 */
void extract_mob_baddrop( NPCharacter *mob ) 
{
    --mob->pIndexData->count;

    char_from_list( mob, &char_list );

    NPCharacterManager::extract( mob );
}

bool mprog_extract( Character *ch, bool count ) 
{
    FENIA_CALL( ch, "Extract", "i", count );
    FENIA_NDX_CALL( ch->getNPC( ), "Extract", "Ci", ch, count );
    ch->extractWrapper(count);
    BEHAVIOR_CALL( ch->getNPC( ), extract, count );
    return false;
}

/*
 * Extract хорошодропнутого моба
 */
void extract_mob_dropped( NPCharacter *mob )
{
    Object *obj, *obj_next;
    
    if (mob->extracted) {
        LogStream::sendError( ) << "Warning! Extraction of " << mob->getNameP( ) << endl;
        return;
    }
    else
        mob->extracted = true;

    for (obj = mob->carrying; obj != 0; obj = obj_next) {
        obj_next = obj->next_content;
        extract_obj_1( obj, true );
    }

    char_from_room( mob );

    --mob->pIndexData->count;
    
    char_from_list( mob, &char_list );

    mprog_extract( mob, true );
    NPCharacterManager::extract( mob );
}



