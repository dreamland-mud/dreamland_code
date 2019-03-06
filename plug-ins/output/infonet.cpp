/* $Id$
 *
 * ruffina, 2004
 */
#include "pcharacter.h"
#include "object.h"

#include "infonet.h"
#include "act.h"
#include "descriptor.h"

#define OBJ_VNUM_PAGER                        102 

/*
 * Info channel
 */
Object * get_pager( Character *ch )
{
    Object *obj;

    for (obj = ch->carrying; obj; obj = obj->next_content) 
        if (obj->pIndexData->vnum == OBJ_VNUM_PAGER)
            return obj;
    
    return NULL;
}

void infonet( const char *string, Character *ch, int min_level )
{
  Descriptor *d;
  Object *obj;

  for ( d = descriptor_list; d != 0; d = d->next )
  {
      if (!d->character || d->connected != CON_PLAYING)
          continue;

      if (ch && ch->is_immortal( ))
          continue;

      if (d->character->get_trust() < min_level)
          continue;
      
      if (d->character == ch)
          continue;
        
      if (( obj = get_pager( d->character ) ))
           act_p( string, d->character, obj, ch, TO_CHAR, POS_DEAD);
  }
}

