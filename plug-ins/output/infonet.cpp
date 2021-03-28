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

/**
 * Send a infonet message with a bit more flexible formatting.
 * 'prefix' normally contains '{CТихий голос из $o2{x' part but can be anything.
 */
void infonet( Character *ch, int min_level, const DLString &prefix, const char *fmt, ...)
{
    va_list args;
    DLString msg;

    va_start( args, fmt );
    msg = prefix + vfmt( NULL, fmt, args );
    va_end( args );

    infonet(msg.c_str(), ch, min_level);
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
           oldact_p( string, d->character, obj, ch, TO_CHAR, POS_DEAD);
  }
}

