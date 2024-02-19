/* $Id: impl.cpp,v 1.1.2.6.6.7 2009/09/24 14:09:12 rufina Exp $
 *
 * ruffina, 2005
 */
#include "logstream.h"
#include "so.h"
#include "date.h"
#include "mocregistrator.h"

#include "skillcommandtemplate.h"
#include "spelltemplate.h"
#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "npcharacter.h"
#include "clanreference.h"
#include "room.h"

#include "xmlattributecards.h"
#include "mobiles.h"
#include "objects.h"
#include "ccard.h"
#include "cardskill.h"

#include "save.h"

#include "def.h"

CLAN(none);

class CardStartersRefresh : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<CardStartersRefresh> Pointer;

    virtual void run( ) 
    {
        Character *ch;
        NPCharacter *mob;
        int cnt = 0;

        for (ch = char_list; ch; ch = ch->next) 
            if (ch->is_npc( ) 
                    && ch->getNPC( )->behavior
                    && ch->getNPC( )->behavior.getDynamicPointer<CardStarterBehavior>( ))
                cnt++;

        if (cnt > 10)
            return;

        for (ch = char_list; ch; ch = ch->next) {
            if (cnt > 10)
                break;

            if (!ch->is_npc( ) || ch->is_mirror())
                continue;
            
            mob = ch->getNPC( );
            
            if (chance( 99 ))
                continue;
            if (IS_GOOD(mob))
                continue;
            if (!IS_SET(mob->form, FORM_BIPED))
                continue;
            if (IS_SET(mob->in_room->area->area_flag, AREA_HIDDEN|AREA_NOQUEST|AREA_WIZLOCK))
                continue;
            if (IS_SET(mob->in_room->room_flags, ROOM_SAFE|ROOM_NO_DAMAGE))
                continue;
            if (mob->in_room->pIndexData->clan != clan_none)
                continue;
            if (mob->behavior && mob->behavior->hasDestiny( )) 
                continue;
            
            CardStarterBehavior::Pointer bhv( NEW );
            bhv->setChar( mob );
            mob->behavior.setPointer( *bhv );
            save_mobs( mob->in_room );
            cnt++;
            
            LogStream::sendNotice( ) 
                << "CARDS autoset " << mob->getNameP( '1' ) << " [" 
                << mob->pIndexData->vnum << "] in room " 
                << mob->in_room->getName() << " [" << mob->in_room->vnum << "]" << endl;
        }
    }
    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_MONTH, Pointer( this ) );    
    }
    virtual int getPriority( ) const
    {
        return SCDP_ROUND + 90;
    }
};

extern "C"
{
    SO::PluginList initialize_cards( ) {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<CardSkill> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCards> >( ppl );

        Plugin::registerPlugin<MobileBehaviorRegistrator<CardStarterBehavior> >( ppl );
        Plugin::registerPlugin<MobileBehaviorRegistrator<CardSellerBehavior> >( ppl );
        
        Plugin::registerPlugin<ObjectBehaviorRegistrator<CardPackBehavior> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<CardBehavior> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<CardSleevesBehavior> >( ppl );

        Plugin::registerPlugin<CCard>( ppl );

        Plugin::registerPlugin<CardStartersRefresh>( ppl );
        
        return ppl;
    }
}


