/* $Id: gangsters.cpp,v 1.1.2.3.6.7 2009/09/24 14:09:12 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "dlscheduler.h"
#include "race.h"
#include "pcrace.h"
#include "room.h"
#include "roomutils.h"
#include "object.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "globalquestmanager.h"
#include "gqchannel.h"
#include "gqexceptions.h"
#include "xmlattributereward.h"

#include "gangsters.h"
#include "gangstersinfo.h"
#include "objects.h"
#include "xmlattributegangsters.h"
#include "gangchef.h"
#include "gangmob.h"
#include "fight_extract.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "interp.h"
#include "act.h"
#include "loadsave.h"
#include "vnum.h"
#include "msgformatter.h"
#include "merc.h"
#include "def.h"
#include "behavior.h"
#include "l10n.h"

BHV(cityguard);

Gangsters* Gangsters::thisClass = NULL;

Gangsters::Gangsters( )
{
}

Gangsters::Gangsters( const DLString& id ) : GlobalQuest( id )
{
    thisClass = this;
}

Gangsters::~Gangsters( )
{
    thisClass = NULL;
}

static Room *findRecallRoom( PCMemoryInterface *pci )
{
    int recallVnum;
    Room *recall;

    if (!pci)
        return NULL;

    recallVnum = pci->getClan( )->getRecallVnum( );
    if (recallVnum > 0)
        recall = get_room_instance( recallVnum );
    else
        recall = get_room_instance( pci->getHometown( )->getRecall( ) );

    if (!recall)
        recall = get_room_instance( ROOM_VNUM_TEMPLE );

    return recall;
}

static Room *findChefKillerRoomAfterKill( Gangsters *gquest, Room *lair, const DLString &killerName )
{
    PCharacter *killer;

    if (!gquest || killerName.empty( ))
        return NULL;

    killer = PCharacterManager::findPlayer( killerName );
    if (killer) {
        if (killer->in_room == lair)
            gquest->exorcism( killer );

        return killer->in_room;
    }

    return findRecallRoom( PCharacterManager::find( killerName ) );
}

static void moveChefCorpseToKillerRoom( Gangsters *gquest, Room *lair, const DLString &killerName, int chefVnum )
{
    Object *obj;
    Room *targetRoom;

    if (!lair)
        return;

    targetRoom = findChefKillerRoomAfterKill( gquest, lair, killerName );
    if (!targetRoom)
        return;

    for (obj = lair->contents; obj; obj = obj->next_content) {
        if (obj->pIndexData->vnum != OBJ_VNUM_CORPSE_NPC)
            continue;

        if (obj->value3( ) != chefVnum)
            continue;

        obj_from_room( obj );
        obj_to_room( obj, targetRoom );
        return;
    }
}

void Gangsters::create( const Config& )  
{
    AreaList areaList;
    
    for(auto &area: areaIndexes) {
        if (area->low_range <= minLevel 
            && !IS_SET(area->area_flag, AREA_WIZLOCK|AREA_HOMETOWN|AREA_HIDDEN
                                        |AREA_NOQUEST|AREA_NOGATE) ) 
        {
            areaList.push_back(area);
        } 
    }
    
    while (!areaList.empty( )) {
        int msize;
        int areaIndex;
        MobileList people;
        RoomList mobRooms, portalRooms;
        AreaIndexData *area;

        areaIndex = number_range(0, areaList.size( ) - 1);
        area = areaList[ areaIndex ];
        areaList.erase( areaList.begin( ) + areaIndex );
        
        mobRoomVnums.clear( );
        portalRoomVnums.clear( );
        
        for (auto &r: area->area->rooms) {  
            Room *room = r.second;
            
            if (checkRoom( room )) {
                mobRooms.push_back( room );
                mobRoomVnums.push_back( room->vnum );

                for (Character *ch = room->people; ch; ch = ch->next_in_room) {
                    if (!getActor( ch )->is_npc( ))
                        continue;

                    if (ch->getNPC( )->pIndexData->area != area)
                        continue;

                    if (ch->getRace( )->isPC( ))
                        people.push_back( ch->getNPC( ) );
                }
            }

            if (GangPortal::canDrop( room ))
                portalRooms.push_back( room );
        }
        
        try {
            msize = mobRooms.size( );
            
            if (!people.empty( ) && !portalRooms.empty( ) && msize > 10) {
                int numPortal, number;

                numPortal = number = (msize > 100 ? 3 : (msize > 40 ? 2 : 1));
                    
                while (!portalRooms.empty( ) && number > 0) 
                    if (createPortal( portalRooms ))
                        number--;

                if (number == 0) {
                    log("good area: " << area->getName());
                    populateArea( area, mobRooms, numPortal );
                    createFirstHint( people );
                    populateLair( );
                    return;
                }
            }        
        }
        catch (const Exception& e) {
            cleanup( false );
            throw e;
        }
        
        cleanup( false );

    } 
    
    
    throw GQCannotStartException(minLevel, maxLevel);
}

void Gangsters::cleanup( bool performance )
{
    Character *ch, *ch_next;
    Object *obj, *obj_next;
    Room *lair;
    
    for (obj = object_list; obj; obj = obj_next) {
        obj_next = obj->next;

        if (!obj->behavior)
            continue;
        
        if (obj->behavior.getDynamicPointer<GangPortal>( ))
            extract_obj( obj );
        else {
            GangKey::Pointer behavior = obj->behavior.getDynamicPointer<GangKey>( );

            if (behavior) {
                behavior->needsReset = false;
                extract_obj( obj );
            }
        }
    }
    
    if (!mobRoomVnums.empty( )) {
        AreaIndexData *area;
        area = get_room_instance( mobRoomVnums.front( ) )->areaIndex();
        REMOVE_BIT(area->area_flag, AREA_NOGATE);
    }
    
    for (ch = char_list; ch; ch = ch_next) {
        ch_next = ch->next;
        
        if (!ch->is_npc() || !ch->getNPC()->behavior || !ch->getNPC()->behavior.getDynamicPointer<GangMob>( ))
            continue;
            
        if (performance) {
            if (ch->position >= POS_RESTING)
                do_say(ch, "Hasta la vista, baby!");
            
            if (ch->position >= POS_MORTAL)        
                oldact(_("$c1 исчезает в клубе дыма."), ch, 0, 0, TO_ROOM);
        }
        
        extract_char( ch );
    }
    
    lair = get_room_instance( GangstersInfo::getThis( )->vnumLair );

    if (state == ST_CHEF_KILLED)
        moveChefCorpseToKillerRoom( this, lair, chefKiller.getValue( ), GangstersInfo::getThis( )->vnumChef );

    wipeRoom( lair );
}

void Gangsters::destroy( ) 
{
    cleanup( true );
    
    switch (state.getValue( )) {
    case ST_CHEF_KILLED:
        rewardChefKiller( );
        break;
    case ST_BROKEN:
        rewardNobody( );
        break;
    default:
        rewardLeader( );
        break;
    }
}

class GangKeysResetTask: public SchedulerTask {
public:
    typedef ::Pointer<GangKeysResetTask> Pointer;
    
    GangKeysResetTask( Gangsters::Pointer gq ) : gquest( gq ) 
    {
    }
    virtual void run( )
    {
        gquest->resetKeys( );
    }
    virtual int getPriority( ) const
    {
        int prio = DLScheduler::getThis( )->getPriority( );

        prio = max( prio, (int)SCDP_INITIAL ) + 100;
        return prio;
    }

private:
    Gangsters::Pointer gquest;
};

void Gangsters::resume( )
{
    int rtime = getTaskTime( );
    
    if (rtime < 0) 
        scheduleDestroy( );
    else {
        DLScheduler::getThis( )->putTaskInSecond( rtime * 60, Gangsters::Pointer( this ) );
        GlobalQuestManager::getThis( )->activate( this );
        DLScheduler::getThis( )->putTaskNOW( GangKeysResetTask::Pointer( NEW, this ) );
    }
}

void Gangsters::after( )
{
    if (state != ST_NONE && state != ST_NO_MORE_HINTS)
        return;

    switch (hintCount++) {
    case 0:
        if (state == ST_NONE)
            if (!createSecondHint( ))
                createThirdHint( );
        
        break;
        
    case 1:
        if (state == ST_NONE)
            createThirdHint( );
        
        break;

    default:
        scheduleDestroy( );
        return;
    }
    
    GlobalQuest::after( );
}

int Gangsters::getTaskTime( ) const
{
    int r = getRemainingTime( ) / (3 - hintCount.getValue( ));
    log("::getTaskTime: time left " << getRemainingTime( ) << ", task time " << r);
    return r;
}

void Gangsters::report( std::ostringstream &buf, PCharacter *ch ) const
{
    if (isLevelOK( ch )) {
        XMLAttributeGangsters::Pointer attr;
        
        attr = ch->getAttributes( ).findAttr<XMLAttributeGangsters>( getQuestID( ) );
        
        if (attr && attr->getKilled( ) > 0)
            buf << fmt( ch, _("Число убитых тобой преступников: {Y%1$d{y"), attr->getKilled( ) )
                << endl;

        buf << fmt( ch, _("До конца охоты остается ") );
        printRemainedTime( buf, ch );
        buf << "." << endl;
    }
}

void Gangsters::progress( std::ostringstream &buf ) const
{
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        XMLAttributeGangsters::Pointer attr; 
        
        attr = i->second->getAttributes( ).findAttr<XMLAttributeGangsters>( getQuestID( ) );
        
        if (!attr || attr->getKilled( ) <= 0)
            continue;
        
        buf << GQChannel::NORMAL
            << fmt(0, "%-15s", i->second->getName( ).c_str( ) ) << " "
            << GQChannel::BOLD << fmt(0, "%-4d", attr->getKilled( ) )
            << GQChannel::NORMAL << endl;
    }
}

void Gangsters::getQuestDescription( std::ostringstream &buf, Character * ) const
{
    // start message + hint interpolate RU-declined mob/room names -- kept RU
    // (localizing the frame around a RU noun would produce a mixed-language line).
    getQuestStartMessage( buf );
    buf << endl        << getHint( ) << endl;
}

void Gangsters::getQuestStartMessage( std::ostringstream &buf ) const
{
    buf << "Шайка преступников атаковала мирных жителей. "
        << "Ищутся храбрецы "
        << GQChannel::BOLD << minLevel << "-" << maxLevel << GQChannel::NORMAL
        << " уровней для уничтожения бандитов и их главаря.";
}

MultiMessage Gangsters::getStartBroadcast( ) const
{
    std::basic_ostringstream<char> levels;
    levels << minLevel << "-" << maxLevel;

    MultiMessage frame = _("Шайка преступников атаковала мирных жителей. Ищутся храбрецы {Y%1$s{y уровней для уничтожения бандитов и их главаря.");
    DLString en = frame.getMessage( LANG_EN ); en.replaces( "%1$s", levels.str( ) );
    DLString ru = frame.getMessage( LANG_RU ); ru.replaces( "%1$s", levels.str( ) );
    DLString ua = frame.getMessage( LANG_UA ); ua.replaces( "%1$s", levels.str( ) );
    return MultiMessage( en, ru, ua );
}

/*****************************************************************************/

/*
 *  rewards
 */

void Gangsters::rewardLeader( )
{
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    std::list<PCMemoryInterface *> leaders;
    int max = 0, killed;

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        XMLAttributeGangsters::Pointer attr;
        
        attr = i->second->getAttributes( ).findAttr<XMLAttributeGangsters>( getQuestID( ) );
        
        if (!attr) 
            continue;

        killed = attr->getKilled( );
        if (killed && killed > max) {
            max = killed;
            leaders.clear( );
            leaders.push_back( i->second );
        } else if (killed && killed == max)
            leaders.push_back( i->second );
    }
    
    GQChannel::gecho( this, _("Главаря шайки так никто и не убил.") );

    if (leaders.empty( )) {
        GQChannel::gecho( this, _("Более того, ни один бандит не пострадал.") );
        return;
    }

    XMLReward reward;

    reward.qpoints = max * number_range( 10, 15 ) + number_fuzzy( 10 );
    reward.gold = max * number_range( 10, 15 );
    reward.experience = max * number_fuzzy( 50 );
    reward.reason[LANG_RU] = "За убийство самого большого количества бандитов ты получаешь: ";
    reward.reason[LANG_EN] = "For slaying the most bandits, you receive: ";
    reward.reason[LANG_UA] = "За вбивство найбільшої кількості бандитів ти отримуєш: ";
    reward.id = getQuestID( );

    MultiMessage frame;
    if (leaders.size( ) == 1)
        frame = _("Самый лучший охотник за бандитами:");
    else
        frame = _("Самые успешные охотники за бандитами:");

    // Player names decline per language (EN gets the Latin login name), so the
    // roll-call is composed once per language instead of once for everyone.
    std::basic_ostringstream<char> roll[LANG_MAX];
    for (int lg = LANG_MIN; lg < LANG_MAX; lg++)
        roll[lg] << frame.getMessage( (lang_t)lg ) << GQChannel::BOLD;

    while (!leaders.empty( )) {
        PCMemoryInterface *pci = leaders.back( );
        leaders.pop_back( );

        for (int lg = LANG_MIN; lg < LANG_MAX; lg++) {
            roll[lg] << " " << pci->getNameP( '1', (lang_t)lg );
            if (!leaders.empty( ))
                roll[lg] << ",";
        }

        log("reward leader " << pci->getName( ));
        GlobalQuestManager::getThis( )->rewardChar( pci, reward );
    }

    GQChannel::gecho( this,
        MultiMessage( roll[LANG_EN].str( ), roll[LANG_RU].str( ), roll[LANG_UA].str( ) ) );
}

void Gangsters::rewardChefKiller( )
{
    XMLReward r;
    PCMemoryInterface *pci = PCharacterManager::find( chefKiller );

    r.gold = number_range( getMaxLevel( ), 2 * getMaxLevel( ) );
    r.qpoints = number_range( 200, 250 );
    r.experience = number_range( 300, 500 );
    r.practice = number_range( -6, 3 );
    r.reason[LANG_RU] = "Поздравляем! Шеф убит и все бандиты разбежались. В награду ты получаешь: ";
    r.reason[LANG_EN] = "Congratulations! The boss is dead and the bandits have scattered. As your reward you receive: ";
    r.reason[LANG_UA] = "Вітаємо! Шефа вбито, і всі бандити розбіглися. У нагороду ти отримуєш: ";
    r.id = getQuestID( );

    GlobalQuestManager::getThis( )->rewardChar( pci, r );

    // Verb agreement is language-specific (EN has none), so the sentence is
    // composed per language around one catalog frame; %2$s is the gender ending.
    MultiMessage frame = _("{Y%1$s{y уничтожил%2$s главаря шайки!");
    DLString en = frame.getMessage( LANG_EN );
    en.replaces( "%1$s", pci->getNameP( '1', LANG_EN ) );
    DLString ru = frame.getMessage( LANG_RU );
    ru.replaces( "%1$s", pci->getNameP( '1', LANG_RU ) );
    ru.replaces( "%2$s", GET_SEX( pci, "", "о", "а" ) );
    DLString ua = frame.getMessage( LANG_UA );
    ua.replaces( "%1$s", pci->getNameP( '1', LANG_UA ) );
    ua.replaces( "%2$s", GET_SEX( pci, "в", "ло", "ла" ) );

    GQChannel::gecho( this, MultiMessage( en, ru, ua ) );

    pci->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" )
                    ->rememberVictory( getQuestID( ) );
}

void Gangsters::rewardNobody( ) 
{
    GQChannel::gecho( this, _("Шефа банды убила противоборствующая группировка.") );
}

void Gangsters::rewardMobKiller( PCharacter *killer, Character *mob )
{
    XMLReward r;
    int diff = mob->getRealLevel( ) - killer->getModifyLevel( );

    r.experience = number_range( 10, 30 );
    r.qpoints = number_range( diff, 8 );
    r.gold = number_range( diff, 8 );
    r.reason[LANG_RU] = "Твоя награда за уничтожение преступника составляет: ";
    r.reason[LANG_EN] = "Your reward for eliminating the criminal is: ";
    r.reason[LANG_UA] = "Твоя нагорода за знищення злочинця становить: ";
    r.id = getQuestID( );
    GlobalQuestManager::getThis( )->rewardChar( killer, r );        

    XMLAttributeGangsters::Pointer attr = killer->getAttributes( ).getAttr<XMLAttributeGangsters>( getQuestID( ) );
    attr->setKilled( attr->getKilled( ) + 1 );
    
    if (state == ST_NONE)
        state = ST_NO_MORE_HINTS;
}

/*****************************************************************************/

/*
 * hint messages
 */
void Gangsters::createFirstHint( MobileList &people )
{
    std::basic_ostringstream<char> buf;
    NPCharacter *informer, *mob;
    DLString name;
    
    informer = people[ number_range( 0, people.size( ) - 1 ) ];
    mob = createMob( );
    char_to_room( mob, informer->in_room );
   
    name = informer->getNameP('1');
    name.upperFirstCharacter( );
    informerName = name;
    informerRoom = informer->in_room->getName();

    buf << "{1" << name << "{2 сообщил" << GET_SEX( informer, "", "о", "а" );
    
    switch (number_range(1, 4)) {
    case 1: case 2: buf << " Хассану"; break;
    case 3: case 4: buf << " Валькирии"; break;
    }
    
    buf << ", что видел" << GET_SEX( informer, "", "о", "а" )
        << " бандитов возле местности под названием " << informer->in_room->getName() << ". ";
    setHint( buf.str( ) );
}            

Room * Gangsters::findHintRoom( MultiMessage &msg )
{
    Room *room = NULL;

    for (unsigned int i = 0; i < mobRoomVnums.size( ); i++) {
        room = get_room_instance( mobRoomVnums[i] );
        
        if (informerRoom.getValue( ) == room->getName())
            continue;

        for (Character *ch = room->people; ch; ch = ch->next_in_room) {
            DLString name;

            if (!getActor( ch )->is_npc( ))
                continue;

            if (ch->getNPC()->pIndexData->area != room->areaIndex()) 
                continue;
            
            if (!ch->getRace( )->isPC( ))
                continue;
            
            name = ch->getNameP('1');
            if (name == informerName.getValue( ))
                continue;

            /* from the same area but not informer */
            
            // Mob and room names render in each language; the verb agrees in
            // RU/UA (%2$s) and carries no ending in EN.
            DLString nameEn = ch->getNameP( '1', LANG_EN ); nameEn.upperFirstCharacter( );
            DLString nameRu = ch->getNameP( '1', LANG_RU ); nameRu.upperFirstCharacter( );
            DLString nameUa = ch->getNameP( '1', LANG_UA ); nameUa.upperFirstCharacter( );

            MultiMessage frame = _("%1$s столкнул%2$s с гангстерами возле %3$s.");
            DLString en = frame.getMessage( LANG_EN );
            en.replaces( "%1$s", nameEn );
            en.replaces( "%3$s", room->getName( LANG_EN ) );
            DLString ru = frame.getMessage( LANG_RU );
            ru.replaces( "%1$s", nameRu );
            ru.replaces( "%2$s", GET_SEX( ch, "ся", "ось", "ась" ) );
            ru.replaces( "%3$s", room->getName( LANG_RU ) );
            DLString ua = frame.getMessage( LANG_UA );
            ua.replaces( "%1$s", nameUa );
            ua.replaces( "%2$s", GET_SEX( ch, "вся", "лося", "лася" ) );
            ua.replaces( "%3$s", room->getName( LANG_UA ) );

            msg = MultiMessage( en, ru, ua );
            return room;
        }
    }

    /* cannot find mob, give hint only about a room they're in */
    if (room) {
        MultiMessage frame = _("Гангстеры были также замечены неподалеку от %1$s.");
        DLString en = frame.getMessage( LANG_EN ); en.replaces( "%1$s", room->getName( LANG_EN ) );
        DLString ru = frame.getMessage( LANG_RU ); ru.replaces( "%1$s", room->getName( LANG_RU ) );
        DLString ua = frame.getMessage( LANG_UA ); ua.replaces( "%1$s", room->getName( LANG_UA ) );
        msg = MultiMessage( en, ru, ua );
    }

    return room;
}

bool Gangsters::createSecondHint( )
{
    MultiMessage msg;
    Room *room = findHintRoom( msg );

    if (room) {
        GQChannel::gecho( this, msg );

        // Stored hint stays RU -- it feeds the RU quest-description display.
        setHint( getHint( ) + " " + msg.getRu( ) );
        char_to_room( createMob( ), room );
    }

    return (room != NULL);
}

void Gangsters::createThirdHint( )
{
    // Stored hint stays RU (it feeds the RU hint/quest-description display,
    // which interpolates declined mob/room names and is kept RU by design).
    std::basic_ostringstream<char> buf;
    buf << "Больше всего от руки бандитов пострадала местность {hh"
        << areaName.getForLang( LANG_RU ) << "{hx.";
    setHint( buf.str( ) );

    // Per-viewer broadcast: catalog frame + the area name in each viewer's
    // language (falls back to RU where an EN/UA area name is unset -- same as
    // every other area-name display). Built as an explicit 3-language message;
    // the {hh help-link resolves on the RU name keyword in every case.
    MultiMessage frame = _("Больше всего от руки бандитов пострадала местность {hh%1$s{hx.");
    DLString en = frame.getMessage( LANG_EN ); en.replaces( "%1$s", areaName.getForLang( LANG_EN ) );
    DLString ru = frame.getMessage( LANG_RU ); ru.replaces( "%1$s", areaName.getForLang( LANG_RU ) );
    DLString ua = frame.getMessage( LANG_UA ); ua.replaces( "%1$s", areaName.getForLang( LANG_UA ) );
    GQChannel::gecho( this, MultiMessage( en, ru, ua ) );

    state = ST_NO_MORE_HINTS;
}

/*****************************************************************************/


NPCharacter * Gangsters::createMob( )
{
    NPCharacter *ch;
    MOB_INDEX_DATA *pMobIndex;
    GangMember::Pointer behavior( NEW );
    int vnum = GangstersInfo::getThis( )->vnumMob;
    
    if (!(pMobIndex = get_mob_index( vnum )))
        throw MobileNotFoundException( vnum );
        
    ch = create_mobile( pMobIndex );
    behavior->setChar( ch );
    behavior->config( number_range( minLevel, maxLevel ) );
    ch->behavior.setPointer( *behavior );
    
    return ch;
}

NPCharacter * Gangsters::createChef( )
{
    NPCharacter *ch;
    MOB_INDEX_DATA *pMobIndex;
    GangChef::Pointer behavior( NEW );
    int vnum = GangstersInfo::getThis( )->vnumChef;
    
    if (!(pMobIndex = get_mob_index( vnum )))
        throw MobileNotFoundException( vnum );
        
    ch = create_mobile( pMobIndex );
    behavior->setChar( ch );
    behavior->config( maxLevel );
    ch->behavior.setPointer( *behavior );

    return ch;
}

Object * Gangsters::createKey( )
{
    Object *key;
    OBJ_INDEX_DATA *pObjIndex;
    GangKey::Pointer behavior( NEW );
    int vnum = GangstersInfo::getThis( )->vnumKey;
    
    if (!(pObjIndex = get_obj_index( vnum )))
        throw ObjectNotFoundException( vnum );
        
    key = create_object( pObjIndex, 0 );
    behavior->setObj( key );
    behavior->setQuest(*this);
    key->behavior.setPointer( *behavior );

    return key;
}

void Gangsters::resetKeys( )
{
    std::vector<NPCharacter *> mobiles;
    Character *ch;
    NPCharacter *mob;
    Object *obj;
    int keyCnt = 0;
   
    if (!keyCount)
        return;

    for (obj = object_list; obj; obj = obj->next) 
        if (obj->behavior && obj->behavior.getDynamicPointer<GangKey>( ))
            keyCnt++;
    
    if (keyCnt >= keyCount)
        return;

    for (ch = char_list; ch; ch = ch->next) {
        if (!ch->is_npc( ))
            continue;

        mob = ch->getNPC( );
        
        if (mob->in_room->vnum == GangstersInfo::getThis( )->vnumLair)
            continue;

        if (!mob->behavior || !mob->behavior.getDynamicPointer<GangMob>( ))
            continue;
            
        for (obj = mob->carrying; obj; obj = obj->next_content) 
            if (obj->pIndexData->vnum == GangstersInfo::getThis( )->vnumKey)
                break;

        if (obj)
            continue;
        
        mobiles.push_back( mob );
    }
    
    while (keyCnt++ < keyCount) {
        if (mobiles.empty( )) {
            mob = createMob( );
            char_to_room( mob, pickRandomRoom( ) );
        }
        else {
            int i = number_range( 0, mobiles.size( ) - 1 );
            
            mob = mobiles[i];
            mobiles.erase( mobiles.begin( ) + i );
        }
        
        obj_to_char( createKey( ), mob );
        log("new key to mob in room " << mob->in_room->getName() ); 
    }
}

Object * Gangsters::createPortal( RoomList &portalRooms ) 
{
    int i; 
    Room *room;
    Object *portal = NULL;
    
    i = number_range(0, portalRooms.size( ) - 1);
    room = portalRooms[i]; 
    portalRooms.erase( portalRooms.begin( ) + i );
    
    switch (room->getSectorType()) {
    case SECT_FOREST:
    case SECT_FIELD:
    case SECT_DESERT:
    case SECT_HILLS:
    case SECT_MOUNTAIN:
        portal = create_object( get_obj_index( GangstersInfo::getThis( )->vnumPortalForest ), 0 );
        break;
    case SECT_CITY:
    case SECT_UNUSED:
    default:
        portal = create_object( get_obj_index( GangstersInfo::getThis( )->vnumPortalCity ), 0 );
        break;
    }
    
    if (!portal->behavior)
        throw BadObjectBehaviorException( portal->pIndexData->vnum );

    portal->value1(portal->value1() | EX_ISDOOR|EX_CLOSED|EX_LOCKED|EX_NOPASS|EX_PICKPROOF);
    portal->value4(GangstersInfo::getThis( )->vnumKey);

    obj_to_room( portal, room );
    portalRoomVnums.push_back( room->vnum );
    log("put portal in " << room->getName());

    return portal;
}

Room * Gangsters::recursiveWalk( Room *room, int depth, int maxDepth ) 
{
    Room *pRoom; 
    int j, i;
    Room * targets [DIR_SOMEWHERE];

    if (depth >= maxDepth) 
        return room;
    
    for (i = 0, j = 0; i < DIR_SOMEWHERE; i++) {
        EXIT_DATA *door;
        if (!room->exit[i])
            continue;
        
        pRoom = room->exit[i]->u1.to_room;
        if (!pRoom)
            continue;

        if (IS_SET(pRoom->room_flags, ROOM_MARKER))
            continue;
        
        door = pRoom->exit[dirs[i].rev];
        if (!door || door->u1.to_room != room)
            continue;

        targets[j++] = pRoom;
    }

    for (i = 0; i < j; i++) {
        int i0 = number_mm( ) % j;
        pRoom = targets[i];
        targets[i] = targets[i0];
        targets[i0] = pRoom;
    }
        
    SET_BIT(room->room_flags, ROOM_MARKER);
    pRoom = NULL;
    
    for (i = 0; i < j; i++) {
        pRoom = recursiveWalk( targets[i], depth + 1, maxDepth );
        if (pRoom) 
            break;            
    }
    
    REMOVE_BIT(room->room_flags, ROOM_MARKER);
    return pRoom;
}

Room * Gangsters::pickRandomRoom( )
{
    int i = number_range( 0, mobRoomVnums.size( ) - 1 );
    return get_room_instance( mobRoomVnums[i] );
}

DLString Gangsters::lairHint( ) 
{
    if (!portalRoomVnums.empty( )) {
        int i = number_range(0, portalRoomVnums.size( ) - 1);
        int vnum = portalRoomVnums[i];
        Room *room = get_room_instance( vnum );

        if (room && (room = recursiveWalk( room, 0, number_range( 1, 2 ) )))
            return room->getName();
    }
    
    return "";
}

void Gangsters::populateArea( AreaIndexData *area, RoomList &mobRooms, int numPortal )
{
    int number;
    
    // capture the area name in every language now (runtime state, regenerated
    // each quest) so the third hint can broadcast it per viewer later
    areaName[LANG_EN] = area->getName( LANG_EN, '1' );
    areaName[LANG_RU] = area->getName( LANG_RU, '1' );
    areaName[LANG_UA] = area->getName( LANG_UA, '1' );
    SET_BIT( area->area_flag, AREA_NOGATE );
    
    number = number_fuzzy( mobRooms.size( ) / 5 );

    for (int j = 0; j <= number; j++) {
        Object *key;
        Character *mob;
        
        mob = createMob( );
        char_to_room( mob, mobRooms[number_range( 0, mobRooms.size( ) - 1 )] );
        
        if (numPortal-- > 0) {
            key = createKey( );
            keyCount++;
            obj_to_char( key, mob );
            log("key to mob in room " << mob->in_room->getName() ); 
        }
    }
}

void Gangsters::populateLair( )
{
    Room *lair;
    int number;

    lair = get_room_instance( GangstersInfo::getThis( )->vnumLair );
    wipeRoom( lair );
    char_to_room( createChef( ), lair );

    number = number_range( 2, 3 );

    while (number-- > 0) 
        char_to_room( createMob( ), lair );
}
    
bool Gangsters::isPoliceman( Character *ch ) 
{
    NPCharacter *mob;
    
    if (!ch->is_npc( ))
        return false;
    
    mob = ch->getNPC();

    if (IS_SET( mob->off_flags, ASSIST_GUARD ) ||
         mob->pIndexData->behaviors.isSet(bhv_cityguard))
        return true;
    
    list<const char *> guardNames {"guard", "guardian", "shiriff", "bodyguard", "cityguard", "стражник", "охранник", "шериф", "телохранитель"};
    for (auto &name: guardNames)
        if (mob->pIndexData->keyword.matchesUnstrict(name))
            return true;

    return false;
}

bool Gangsters::checkRoom( Room *const pRoomIndex )
{
    if (RoomUtils::isWaterOrAir(pRoomIndex))
        return false;
    
    if (IS_SET(pRoomIndex->room_flags, ROOM_SAFE|ROOM_NO_QUEST|ROOM_NO_MOB))
        return false;
        
    if (!pRoomIndex->isCommon( ))
        return false;

    return true;
}

    
