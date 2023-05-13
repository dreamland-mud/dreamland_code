/* $Id$
 *
 * ruffina, 2004
 */

#include <list>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "profiler.h"
#include "mallocexception.h"
#include "fileformatexception.h"
#include "logstream.h"
#include "class.h"
#include "char.h"
#include "plugininitializer.h"
#include "dlfileop.h"

#include "dlscheduler.h"
#include "schedulertaskroundplugin.h"

#include "skillreference.h"

#include "mobilebehavior.h"
#include "areabehaviormanager.h"
#include "areabehavior.h"
#include "roombehaviormanager.h"

#include "fenia/register-impl.h"
#include "feniamanager.h"
#include "wrapperbase.h"

#include "affect.h"
#include "race.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "areahelp.h"

#include "dreamland.h"
#include "compatflags.h"
#include "itemflags.h"
#include "fread_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

using namespace std;

WEARLOC(none);
LIQ(none);
LIQ(water);

char                        strArea[MAX_INPUT_LENGTH];
list<RESET_DATA *> areaResets;

bool dup_room_vnum( int vnum );

static bool __lowrangecmp__(AreaIndexData *l, AreaIndexData *r)
{
    return l->min_vnum < r->min_vnum;
}

static void vnumck()
{
    typedef list<AreaIndexData *> alist;
    alist areas;
    
    LogStream::sendNotice() << "Checking vnum ranges..." << endl;

    for(auto &area: areaIndexes)
        areas.push_back(area);

    areas.sort( ptr_fun(__lowrangecmp__) );

    for(alist::iterator i = areas.begin();i != areas.end();) {
        MOB_INDEX_DATA *mob;
        OBJ_INDEX_DATA *obj;
        RoomIndexData *room;
        char *aname = (*i)->name;
        int l = (*i)->min_vnum, h = (*i)->max_vnum, hashi;
        int minused = 0xFFFFL, maxused = 0;
        
        if(l > h)
            throw FileFormatException(
                "%s: min vnum is higher then max.", aname);
        
        for (auto &r: roomIndexMap) {
            room = r.second;
            if(room->areaIndex != *i)
                continue;
            
            if(room->vnum < l || room->vnum > h)
                throw FileFormatException(
                        "%s: room %d out of allocated range.", 
                        aname, room->vnum);
            
            if(minused > room->vnum)
                minused = room->vnum;
            if(maxused < room->vnum)
                maxused = room->vnum;
        }
        
        for(hashi = 0;hashi < MAX_KEY_HASH;hashi++)
            for(mob = mob_index_hash[hashi]; mob; mob = mob->next) {
                if(mob->area != *i)
                    continue;
                
                if(mob->vnum < l || mob->vnum > h)
                    throw FileFormatException(
                            "%s: mob %d out of allocated range.",
                            aname, mob->vnum);
                
                if(minused > mob->vnum)
                    minused = mob->vnum;
                if(maxused < mob->vnum)
                    maxused = mob->vnum;
            }
        
        for(hashi = 0;hashi < MAX_KEY_HASH;hashi++)
            for(obj = obj_index_hash[hashi]; obj; obj = obj->next) {
                if(obj->area != *i)
                    continue;
                
                if(obj->vnum < l || obj->vnum > h)
                    throw FileFormatException(
                            "%s: obj %d out of allocated range.",
                            aname, obj->vnum);
                
                if(minused > obj->vnum)
                    minused = obj->vnum;
                if(maxused < obj->vnum)
                    maxused = obj->vnum;
            }

        i++;
        int next_l = 0xffffL;
        
        if(i != areas.end()) {
            next_l = (*i)->min_vnum;
            h++;
        
            if(h > next_l) {
                throw FileFormatException(
                        "%s: NEGATIVE GAP(%d)! Area is intersected with %s!",  
                        aname, next_l - h, (*i)->name);
            }
        }
    }
}

static void bad_reset( const char *fmt, ... )
{
    char buf[1024];
    va_list ap;  
    va_start( ap, fmt );
    vsnprintf( buf, sizeof( buf ), fmt, ap );
    va_end( ap );

    if (dreamland->hasOption(DL_BUILDPLOT)) 
        logerror( buf );
    else
        throw FileFormatException( buf );
}

void fix_resets()
{
    char letter = '\0';
    int iLastRoom = 0, iLastObj = 0;

    LogStream::sendNotice() << "Fix_resets..." << endl;

    for (auto &pReset: areaResets)
    {
        RoomIndexData *pRoomIndex;
        EXIT_DATA *pexit;
        OBJ_INDEX_DATA *temp_index;

        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch ( pReset->command )
        {
        default:
            throw FileFormatException( "Load_resets: bad command '%c'.", letter );
            break;

        case 'M':
            get_mob_index  ( pReset->arg1 );
            if( (pRoomIndex = get_room_index ( pReset->arg3 ))) {
                pRoomIndex->resets.push_back(pReset);
                iLastRoom = pReset->arg3;
            }
            else
                bad_reset( "Load_resets: 'M': bad room: %d.", pReset->arg3 );
            break;
        case 'O':
            if (!( temp_index = get_obj_index( pReset->arg1 ) )) {
                bad_reset( "Load_resets: 'O': bad obj: %d.", pReset->arg1 );
                break;
            }
            if( (pRoomIndex = get_room_index ( pReset->arg3 ))) {
                pRoomIndex->resets.push_back(pReset);
                iLastObj = pReset->arg3;
            }
            else
                bad_reset( "Load_resets: 'O': bad room: %d.", pReset->arg3 );
            break;
        case 'P':
            if (!( temp_index = get_obj_index( pReset->arg1 ) )) {
                bad_reset( "Load_resets: 'P': bad obj: %d.", pReset->arg1 );
                break;
            }
            if (!( temp_index = get_obj_index( pReset->arg3 ) )) {
                bad_reset( "Load_resets: 'P': bad obj: %d.", pReset->arg3 );
                break;
            }
            if( (pRoomIndex = get_room_index ( iLastObj ))) {
                pRoomIndex->resets.push_back(pReset);
            }
            else
                bad_reset( "Load_resets: 'O': bad room: %d.", pReset->arg3 );
            break;

        case 'G':
        case 'E':
            if (!( temp_index = get_obj_index( pReset->arg1 ) )) {
                bad_reset( "Load_resets: 'G''E': bad obj: %d.", pReset->arg1 );
                break;
            }
            if( (pRoomIndex = get_room_index ( iLastRoom ))) {
                pRoomIndex->resets.push_back(pReset);
                iLastObj = iLastRoom;
            }
            else
                bad_reset( "Load_resets: 'G''E': bad room: %d.", pReset->arg3 );
            break;

        case 'D':
            pRoomIndex = get_room_index( pReset->arg1 );

            notice("...checking D reset arg1=%d arg2=%d arg3=%d. Found room %s", pReset->arg1, pReset->arg2, pReset->arg3, pRoomIndex->name);

            
            if ( pReset->arg2 == DIR_SOMEWHERE ) {
                if ( pRoomIndex->extra_exits.empty() ) {
                    bad_reset( "Load_resets: 'D': room %d does not contain extra exits", pReset->arg1 );
                    break;
                }
            } else {
                if ( pReset->arg2 < 0
                        || pReset->arg2 > 5
                        || ( pexit = pRoomIndex->exit[pReset->arg2] ) == 0)
                {
                    bad_reset( "Load_resets: 'D': exit %d from room %d not door", pReset->arg2, pRoomIndex->vnum );
                    break;
                }

                SET_BIT( pexit->exit_info, EX_ISDOOR );
                SET_BIT( pexit->exit_info_default, EX_ISDOOR );

                switch(pReset->arg3) {
                    default:
                        bad_reset( "Load_resets: 'D': bad 'locks': %d.", pReset->arg3 );
                        break;
                    
                    case 0:
                        break;
                    case 1:
                        SET_BIT(pexit->exit_info, EX_CLOSED);
                        SET_BIT(pexit->exit_info_default, EX_CLOSED);
                        break;
                    case 2:
                        SET_BIT(pexit->exit_info, EX_CLOSED | EX_LOCKED);
                        SET_BIT(pexit->exit_info_default, EX_CLOSED | EX_LOCKED);
                        break;
                }
            }

            free_mem(pReset, sizeof(*pReset));

            break;

        case 'R':
            if (!( pRoomIndex= get_room_index( pReset->arg1 ) )) {
                bad_reset( "Load_resets: 'R': bad room: %d.", pReset->arg1 );
                break;
            }

            if ( pReset->arg2 < 0 || pReset->arg2 > 6 ) {
                bad_reset( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
                break;
            }
            
            pRoomIndex->resets.push_back(pReset);
            break;
        }
    }

    for (auto &r: roomIndexMap) {
        RoomIndexData *pRoom = r.second;

        for(auto &pReset: pRoom->resets) 
            switch (pReset->command) {
            case 'G': case 'O':
            case 'E': case 'P':
                if (!( get_obj_index( pReset->arg1 ) )) {
                    bad_reset( "Load_resets: Room [%d] bad obj [%d].", pRoom->vnum, pReset->arg1 );
                    break;
                }
                get_obj_index( pReset->arg1 )->reset_num++;
                break;
            }
    }
}



/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( bool verbose )
{
        Room *to_room = 0;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev = 0;
        int door;
        static const int rev_dir [] = { 2, 3, 0, 1, 5, 4 };

        LogStream::sendNotice( ) << "Fixing exits..." << endl;

        for (auto &r: roomIndexMap) {
            bool fexit;

            fexit = false;
            RoomIndexData *pRoomIndex = r.second;
            Room *room = pRoomIndex->room; // FIXME all instances will be linked

            for ( door = 0; door < DIR_SOMEWHERE; door++ )
            {
                    if ( ( pexit = room->exit[door] ) != 0 )
                    {
                            if ( pexit->u1.vnum <= 0
                                    || get_room_instance(pexit->u1.vnum) == 0 )
                            {
                                    pexit->u1.to_room = 0;
                            }
                            else
                            {
                                    fexit = true;
                                    pexit->resolve();
                            }
                    }
            }

            for (auto &peexit: room->extra_exits)
            {
                    if ( peexit->u1.vnum <= 0
                            || get_room_instance(peexit->u1.vnum) == 0 )
                    {
                            peexit->u1.to_room = 0;
                    }
                    else
                    {
                            fexit = true;
                            peexit->resolve();
                    }
            }

            if ( !fexit )
                    SET_BIT(room->room_flags,ROOM_NO_MOB);
        }

        for (auto &r: roomIndexMap) {
            RoomIndexData *pRoomIndex = r.second;
            Room *room = pRoomIndex->room;

            for ( door = 0; door <= 5; door++ )
            {
                    if ( ( pexit     = room->exit[door]       ) != 0
                            && ( to_room   = pexit->u1.to_room            ) != 0
                            && ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
                            && pexit_rev->u1.to_room != room
                            && ( room->vnum < 1200 || room->vnum > 1299 ))
                    {
                        if (verbose)
                            LogStream::sendWarning( ) << "Fix_exits: " << room->vnum << ':'
                                    << door << " -> " << to_room->vnum << ':' << rev_dir[door] << " -> "
                                    << ( pexit_rev->u1.to_room == 0 ? 0 : pexit_rev->u1.to_room->vnum )
                                    << '.' << endl;
                    }
            }
        }

        return;
}


/*--------------------------------------------------------------------------
 * assigning levels for locked passable doors
 *-------------------------------------------------------------------------*/
static int get_obj_reset_level( AreaIndexData *pArea, int keyVnum )
{
    int mobVnum = -1;
    int level = 200;
    
    if (keyVnum <= 0 || !get_obj_index( keyVnum ))
        return pArea->low_range;

    for (auto &r: roomIndexMap) {
        RoomIndexData *pRoomIndex  = r.second;
        
        for(auto &pReset: pRoomIndex->resets)
            switch(pReset->command) {
                case 'M':
                    mobVnum = pReset->arg1;
                    break;
                case 'G':
                case 'E':
                    if (pReset->arg1 == keyVnum && mobVnum > 0)
                        level = min( get_mob_index(mobVnum)->level, level );
                    break;
                case 'O':
                    if (pReset->arg1 == keyVnum) 
                        level = min( pArea->low_range, level );
                    break; 
                case 'P':
                    if (pReset->arg1 == keyVnum) {
                        OBJ_INDEX_DATA *in = get_obj_index( pReset->arg3 );

                        if (in->item_type == ITEM_CONTAINER
                            && IS_SET(in->value[1], CONT_LOCKED))
                        {
                            level = min( get_obj_reset_level( pRoomIndex->areaIndex, in->value[2] ), 
                                         level );
                        }
                        else
                            level = min( pArea->low_range, level );
                    }
                    break;
            }
    }
    
    return (level == 200 ? pArea->low_range : level);
}            

void fix_door_levels( bool verbose )
{
    LogStream::sendNotice( ) << "Assigning door levels..." << endl;

    for (auto &r: roomInstances) {
        for (int d = 0; d < DIR_SOMEWHERE; d++) {
            EXIT_DATA *ex = r->exit[d];

            if (!ex)
                continue;
            if (!IS_SET(ex->exit_info_default, EX_CLOSED))
                continue;
            if (IS_SET(ex->exit_info_default, EX_NOPASS))
                continue;
            
            ex->level = get_obj_reset_level( r->area->pIndexData, ex->key );
            
            if (verbose)
                LogStream::sendNotice( )
                    << "[" << r->vnum << "] " << r->getName() << " " << d << ": "
                    << "level " <<  ex->level << endl;
        }

        for (auto &ex: r->extra_exits) {
            if (!IS_SET(ex->exit_info_default, EX_CLOSED))
                continue;
            if (IS_SET(ex->exit_info_default, EX_NOPASS))
                continue;
            
            ex->level = get_obj_reset_level( r->area->pIndexData, ex->key );

            if (verbose)
                LogStream::sendNotice( )
                    << "[" << r->vnum << "] " << r->getName() << " extra: "
                    << "level " <<  ex->level << endl;
        }
    }
}

void
fix_areas( )
{
        /*
         * Fix up exits.
         * Declare db booting over.
         * Reset all areas once.
         */
        fix_exits( false );
        fix_resets( );
        fix_door_levels( false );

        vnumck( );
}

class AreaFixTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<AreaFixTask> Pointer;

    virtual void run( )
    {
        if (DLScheduler::getThis()->getCurrentTick( ) == 0) 
            fix_areas( );
    }

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 6;
    }
};

PluginInitializer<AreaFixTask> initFixAreas;

bool dup_mob_vnum( int vnum )
{
    MOB_INDEX_DATA *pMob;

    for (pMob = mob_index_hash[vnum % MAX_KEY_HASH]; pMob; pMob = pMob->next)
        if ( pMob->vnum == vnum )
            return true;

    return false;
}


bool dup_obj_vnum( int vnum )
{
    OBJ_INDEX_DATA *pObj;

    for (pObj = obj_index_hash[vnum % MAX_KEY_HASH]; pObj; pObj = pObj->next)
        if ( pObj->vnum == vnum )
            return true;

    return false;
}

bool dup_room_vnum( int vnum )
{
    return roomIndexMap.find(vnum) != roomIndexMap.end();
}

