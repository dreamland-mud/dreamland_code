/* $Id$
 *
 * ruffina, 2004
 */
#include "groupchannel.h"
#include "replay.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "loadsave.h"

#include "follow_utils.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

/*-------------------------------------------------------------------------
 * GroupChannel
 *------------------------------------------------------------------------*/
const DLString GroupChannel::COMMAND_NAME = "gtell";

GroupChannel::GroupChannel( ) 
{
    name = COMMAND_NAME;
}

GroupChannel::~GroupChannel( )
{
}

bool GroupChannel::isGlobalListener( Character *ch, Character *victim ) const
{
    if (!is_same_group( victim, ch ))
        return false;

    return GlobalChannel::isGlobalListener( ch, victim );
}

void GroupChannel::findListeners( Character *ch, Listeners &listeners ) const
{
    Character *gch;

    for (gch = char_list; gch != 0; gch = gch->next)
        if (isGlobalListener( ch, gch ))
            listeners.push_back( gch );
}

void GroupChannel::triggers( Character *ch, const DLString &msg ) const
{
    GlobalChannel::triggers( ch, msg );

    if (!ch->is_npc( ) && (!str_prefix( msg.c_str( ), "where are you?" )
                || !str_prefix( msg.c_str( ), "где ты?" ))) {
            NPCharacter *pet = ch->getPC( )->pet;
    
        if (pet && pet->position > POS_SLEEPING){
            if(IS_AFFECTED(pet, AFF_BLIND)){
            tell_raw( ch, pet, "%s, я ничего не вижу!",
                        GET_SEX( ch, "Хозяин", "Хозяин", "Хозяйка"));
            }
            else if(eyes_darkened(pet)){
            tell_raw( ch, pet, "%s, тут слишком темно, я ничего не вижу!",
                        GET_SEX( ch, "Хозяин", "Хозяин", "Хозяйка"));
            }
            else{   
            tell_raw( ch, pet, "%s, я нахожусь в {hh%s{hx - %s",
                        GET_SEX( ch, "Хозяин", "Хозяин", "Хозяйка"),
                        pet->in_room->area->name, pet->in_room->name );
            }
        }
    }
}

bool GroupChannel::canTalkGlobally( Character *ch ) const
{
    if (!GlobalChannel::canTalkGlobally( ch ))
        return false;

    if (IS_SET( ch->comm, COMM_NOTELL )) {
        ch->println( "Твое сообщение не получено!" );
        return false;
    }

    return true;
}

void GroupChannel::postOutput( Character *outputTo, const DLString &message ) const
{
    if (outputTo->getPC( ))
        remember_history_near( outputTo->getPC( ), message );
}

