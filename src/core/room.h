/* $Id: room.h,v 1.1.2.7.6.9 2014-09-19 11:44:46 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __ROOM_H__
#define __ROOM_H__

#include "jsoncpp/json/value.h"
#include "xmlmap.h"
#include "xmlboolean.h"
#include "globalbitvector.h"
#include "xmlstreamable.h"

#include "core/fenia/wrappertarget.h"
#include "clanreference.h"
#include "liquid.h"
#include "roombehavior.h"
#include "affectlist.h"
#include "xmlmultistring.h"
#include "extradescription.h"
#include "exits.h"
#include "resets.h"

struct AreaIndexData;
struct Area;
struct reset_data;
class Room;
class RoomIndexData;

typedef set<Room *> RoomSet;
/** A small collection of rooms with affects on them, to avoid going through the whole list in updates. */
extern RoomSet roomAffected;

typedef map<int, RoomIndexData *> RoomIndexMap;
/** Map of all room prototypes by vnum, for quick access. */
extern RoomIndexMap roomIndexMap;

typedef vector<Room *> RoomVector;
/** List of all room instances. */
extern RoomVector roomInstances;


struct RoomHistoryEntry {
    RoomHistoryEntry( DLString n, DLString rn, int w )
            : name( n ), rname(rn), went( w )
    {
    }
    DLString name;
    DLString rname;
    int went;
};

struct RoomHistory : public list<RoomHistoryEntry> {
    void record( Character *, int );
    void erase( );
    bool traverse( Room *, Character * ) const;
    int  went( Character * ) const;
    int  went( DLString &, bool ) const;
    void toStream( ostringstream & ) const;
    static const unsigned int MAX_SIZE;
};

struct RoomIndexData : public virtual DLObject, public WrapperTarget {
    RoomIndexData();
    virtual ~RoomIndexData();

    Room * create(); // Implemented in loadsave plugin.

    ExtraDescrList extraDescriptions;
    exit_data *        exit        [6];
    ExtraExitList extra_exits;

    XMLMultiString name;
    XMLMultiString description;
    XMLMultiString smell;
    XMLMultiString sound;

    int         vnum;
    int         room_flags;
    int         sector_type;
    int         heal_rate;
    int         mana_rate;
    ClanReference clan;
    GlobalBitvector guilds;
    LiquidReference liquid;
    ::Pointer<XMLDocument> behavior;
    GlobalBitvector behaviors;
    Json::Value props;

    Scripting::Register init;

    AreaIndexData *areaIndex;

    Room *room; // FIXME wil be replaces with a list of instances

    ResetList resets;
};

class Room : public virtual DLObject, public WrapperTarget {
public:
    Room( );

    bool isPrivate( ) const;
    int  getCapacity( ) const;
    bool isCommon( );
    inline long long getID( ) const;
    inline void setID( long long );

    // room affects
    void affectModify( Affect *paf, bool fAdd );
    void affectTo( Affect *paf );
    void affectJoin( Affect *paf );
    void affectCheck( const FlagTable *table, int vector );
    void affectRemove( Affect *paf, bool verbose = false );
    void affectStrip( int sn, bool verbose = false );
    bool isAffected( int sn ) const;
    
    bool isDark( ) const;
    /** Recalculate 'light' field for current room content. */
    void updateLight();

    void vecho( int, const char *, va_list ) const;
    void echo( int, const char *, ... ) const;
    list<Character*> getPeople( );
    
    bool hasExits() const;
    
    /** Shorthand to return prototype's extra descriptions. */
    inline const ExtraDescrList & getExtraDescr() const;

    /** Shorthand to return prototype's room name. */
    inline const char *getName() const;

    /** Shorthand to return prototype's room description. */
    inline const char *getDescription() const;

    /** Calculate current heal rate (100% is the default). */
    int getHealRate() const;

    /** Calculate current mana rate (100% is the default). */
    int getManaRate() const;

    /** Return sector type, either from prototype or as changed by the affects. */
    int getSectorType() const;

    /** Return room liquid type, either from prototype or its own override. */
    LiquidReference &getLiquid();

    /** Quick access to area prototype for this room instance. */
    inline AreaIndexData *areaIndex() const;

    /** Quick access to room area name, in nominative case by default. */
    DLString areaName(char gcase = '1') const;

    /** This room's position in the global roomInstances vector. Needed for backward compat. */
    int position;

    Character *   people;
    Object *      contents;
    Area *area;
    exit_data *   exit[6];
    ExtraExitList extra_exits;
    int        vnum;
    int        room_flags;
    int        light;

    RoomHistory history;
    AffectList affected;
    int        affected_by;

    XMLPersistentStreamable<RoomBehavior> behavior;
    Scripting::Register init;

    RoomIndexData *pIndexData;

protected:
    /** Sector type if different from the prototype. */
    int sector_type;

    /** Room liquid if different from the prototype. */
    LiquidReference liquid;

    /** How much default heal rate is changed by affects. */
    int mod_heal_rate;

    /** How much default heal rate is changed by affects. */
    int mod_mana_rate;

    long long ID;
};

inline long long Room::getID( ) const
{
    return ID;
}

inline void Room::setID( long long id )
{
    ID = id;
}

inline const ExtraDescrList & Room::getExtraDescr() const
{
    return pIndexData->extraDescriptions;
}

inline const char * Room::getName() const
{
    return pIndexData->name.get(LANG_DEFAULT).c_str();
}

inline const char * Room::getDescription() const
{
    return pIndexData->description.get(LANG_DEFAULT).c_str();
}

inline AreaIndexData *Room::areaIndex() const
{
    return pIndexData->areaIndex;
}

/*
 * room macros
 */

#define IS_ROOM_AFFECTED(room, sn)         (IS_SET((room)->affected_by, (sn)))

/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
RoomIndexData * get_room_index( int vnum );

/** Looks up default room instance by vnum. (FIXME) */
Room * get_room_instance(int vnum);

#endif
