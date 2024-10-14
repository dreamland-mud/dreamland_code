#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "core/object.h"
#include "affect.h"
#include "affecthandler.h"
#include "pcharactermanager.h"
#include "npcharactermanager.h"
#include "skillreference.h"
#include "spelltarget.h"
#include "desire.h"
#include "descriptor.h"
#include "follow_utils.h"
#include "fight_extract.h"
#include "fight_position.h"
#include "interp.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

GSN(doppelganger);

/*--------------------------------------------------------------
 * character extraction
 *--------------------------------------------------------------*/
static bool char_is_nodrop( Character *ch )
{
    NPCharacter *npc = ch->getNPC();

    if (!npc)
        return false;
    
    if (IS_SET(npc->pIndexData->area->area_flag, AREA_NOSAVEDROP))
        return true;
    
    if (IS_SET(ch->act, ACT_NOSAVEDROP))
        return true;

    return false;
}

/*
 * nuke pet on extraction
 */
void nuke_pets( PCharacter *ch, int flags )
{
    NPCharacter *pet = ch->pet;
    
    if (!pet)
        return;

    follower_stop(pet);
    
    if (IS_SET(flags, FEXTRACT_TOTAL))
        extract_char( pet, IS_SET(flags, FEXTRACT_COUNT));
    else
        pet->setDead( );

    ch->pet = NULL;
}

/*
 * Оповестить о extract-е всех, кто на нас ссылался
 */
void notify_referers( Character *ch, int flags )
{
    Character *wch;
    AffectSource chSource(ch);

    for (wch = char_list; wch != 0; wch = wch->next) {
        if (IS_SET(flags, FEXTRACT_TOTAL) && wch->reply == ch)
            wch->reply = 0;

        if (wch->doppel == ch && wch->isAffected(gsn_doppelganger)) {
            wch->pecho("Ты принимаешь свое истинное обличье.");
            
            // TODO rework with verbose affect strip
            if (gsn_doppelganger->getAffect( ))
                gsn_doppelganger->getAffect( )->onRemove(SpellTarget::Pointer(NEW, wch), 0);

            affect_strip(wch,gsn_doppelganger);
        }

        // All affects on others that ch created are no longer associated with this char.
        for (auto &paf: wch->affected)
            paf->sources.remove(chSource);

        if (wch->is_npc( ) && wch->getNPC( )->behavior) 
            wch->getNPC( )->behavior->extractNotify( ch, IS_SET(flags, FEXTRACT_TOTAL), IS_SET(flags, FEXTRACT_COUNT) );
        
        if (IS_SET(flags, FEXTRACT_TOTAL|FEXTRACT_LASTFOUGHT) && wch->last_fought == ch)
            wch->last_fought = 0;
    }
    
    // All room affects that ch created are no longer associated with this char,
    // meaning no damage will be done on behalf of ch after their death.
    for (auto &room: roomAffected)
        for (auto &paf: room->affected)
            paf->sources.remove(chSource);

    guarding_clear( ch );
}

/*
 * Extract a char from the world.
 */
void extract_char( Character *ch, bool count )
{
    Object *obj;
    Object *obj_next;
    int flags;

    if (ch->extracted)  {
        LogStream::sendError( ) << "Warning! Extraction of " << ch->getNameC() << endl;
        return; 
    }
    else
        ch->extracted = true;  

    NPCharacter *npc = ch->getNPC( );
    PCharacter *pc = ch->getPC( );
    
    if (char_is_nodrop( ch ))
        count = true;

    flags = FEXTRACT_TOTAL | (count ? FEXTRACT_COUNT : 0);

    if (!npc)
        nuke_pets( pc, flags );
        
    follower_die(ch);

    stop_fighting( ch, true );

    for (obj = ch->carrying; obj != 0; obj = obj_next) {
        obj_next = obj->next_content;
        extract_obj_1( obj, count );
    }
    
    undig( ch );
    ch->dismount( );
    
    notify_referers( ch, flags );

    if (npc && npc->switchedFrom)
        interpret_raw( ch, "return" );

    char_from_room( ch );

    if (npc) {
        if (count)
            --npc->pIndexData->count;
    }

    /*paranoid*/
    if (!npc && pc->switchedTo && pc->switchedTo->desc) {
        pc->switchedTo->desc->close( );
        pc->switchedTo->switchedFrom = 0;
        pc->switchedTo = 0;
        LogStream::sendError() 
            << "attempt to extract original PCwhile in switch: " 
            << pc->getNameC() << endl;
    }

    char_from_list( ch, &char_list );
    
    if (ch->desc)
        ch->desc->character = 0;
        
    mprog_extract( ch, count );
    
    if (!npc)
        PCharacterManager::extract( pc );
    else
        NPCharacterManager::extract( npc );
}

/*
 * Extract мертвого игрока
 */
void extract_dead_player( PCharacter *ch, int flags )
{
    Room *altar;
    
    nuke_pets( ch, flags );
    follower_die(ch);

    undig( ch );
    ch->dismount( );
    
    if (( altar = get_room_instance( ch->getHometown( )->getAltar( ) ) )) {
        char_from_room( ch );
        char_to_room( ch, altar );
    }

    notify_referers( ch, flags );

    for (auto &paf: ch->affected.clone())
        affect_remove( ch, paf );

    ch->affected_by    = 0;
    ch->detection    = 0;
    ch->armor.clear( );
    ch->armor.fill( 100 );
    ch->position    = POS_STANDING;
    ch->hit        = ch->max_hit / 10;
    ch->mana    = ch->max_mana / 10;
    ch->move    = ch->max_move;
    ch->shadow = -1;
    ch->parts = ch->getRace()->getParts();

    for (int i = 0; i < desireManager->size( ); i++)
        desireManager->find( i )->reset( ch );
}

