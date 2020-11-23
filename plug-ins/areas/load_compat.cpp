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

FILE *                        fpArea;
char                        strArea[MAX_INPUT_LENGTH];

RESET_DATA               *reset_first = 0;
RESET_DATA               *reset_last = 0;

bool dup_room_vnum( int vnum );
void area_update( void );
void        new_load_area( FILE *fp );
void        load_helps( FILE *fp );
void         load_mobiles( FILE *fp );
void         load_objects( FILE *fp );
void        load_resets( FILE *fp );
void        load_rooms( FILE *fp );
void        load_shops( FILE *fp );
void        load_specials( FILE *fp );
void  load_olimits( FILE *fp );
void  load_practicer( FILE *fp );
void  load_resetmsg( FILE *fp );
void  load_aflag( FILE *fp );

void        fix_exits( bool );
void        fix_resets( void );
void        fix_door_levels( bool );

bool 
__lowrangecmp__(AreaIndexData *l, AreaIndexData *r)
{
    return l->min_vnum < r->min_vnum;
}

void
vnumck()
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

void load_area_header(FILE *fp, AreaIndexData *pArea)
{
#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )                        \
                if ( !str_cmp( word, literal ) )        \
                {                                        \
                    field  = value;                        \
                    fMatch = true;                        \
                    break;                                \
                }
                
#ifdef SKEY
#undef SKEY
#endif
#define SKEY( literal, field )                                \
                if ( !str_cmp( word, literal ) )        \
                {                                        \
                    field  = fread_string(fp);                \
                    fMatch = true;                        \
                    break;                                \
                }

    char *word;
    
    for(;!feof(fp);) {
        word = fread_word(fp);
        bool fMatch = false;

        switch(Char::upper(*word)) {
        case 'A':
            SKEY("Authors", pArea->authors);
            SKEY("Altname", pArea->altname);
            break;
        case 'B':
            if (!str_cmp(word, "Behavior")) {
                pArea->behavior.clear( );
                AreaBehaviorManager::parse( pArea, fp );
                fMatch = true;
            }
            break;
        case 'C':
            SKEY("Credits", pArea->credits);
            break;
        case 'E':
            if (!str_cmp(word, "End"))
                return;
            break;
        case 'F':
            KEY("Flags", pArea->area_flag, fread_flag(fp));
            break;
        case 'L':
            if (!str_cmp(word, "LevelRange")) {
                pArea->low_range = fread_number(fp);
                pArea->high_range = fread_number(fp);
                fMatch = true;
            }
            break;
        case 'N':
            SKEY("Name", pArea->name);
            break;
        case 'R':
            SKEY("ResetMessage", pArea->resetmsg);
            break;
        case 'T':
            SKEY("Translator", pArea->translator);
            break;
        case 'S':
            KEY("Security", pArea->security, fread_number(fp));
            break;
        case 'V':
            if (!str_cmp(word, "VNUMs")) {
                pArea->min_vnum = fread_number(fp);
                pArea->max_vnum = fread_number(fp);
                fMatch = true;
            }
            break;
        }

        if(!fMatch) {
            LogStream::sendError() << "no match for " << word << " token in #AREADATA section" << endl;
        }
    }
    LogStream::sendError() << "No End marker found in #AREADATA section" << endl;
#undef KEY
#undef SKEY
}

void new_load_area( FILE *fp )
{
    AreaIndexData *pArea;
    
    pArea                = new AreaIndexData;
    pArea->authors      = str_dup("");
    pArea->altname      = str_dup("");
    pArea->name         = str_dup("");
    pArea->credits      = str_dup("");
    pArea->translator   = str_dup("");
    pArea->speedwalk    = str_dup("");

    load_area_header(fp, pArea);

    pArea->area_file        = area_file_list;
    area_file_list->area= pArea;
    pArea->vnum                = top_area;

    areaIndexes.push_back(pArea);

    top_area++;

    // Create new single area instance for this index (FIXME)
    pArea->create();

    return;
}

/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    for ( ; ; )
    {
        AreaHelp::Pointer help( NEW );

        help->setLevel( fread_number( fp ) );
        help->addAutoKeyword( fread_string( fp ) );
        help->areafile = area_file_list;

        if (help->getAllKeywordsString( )[0] == '$') 
            break;

        help->setText( fread_string( fp ) );
        helpManager->registrate( help );
    }
}


void
new_reset(RoomIndexData *pR, RESET_DATA *pReset)
{
    if(!pR)
        return;

    if(!pR->reset_last) {
        pR->reset_first = pReset;
        pR->reset_last = pReset;
    } else {
        pR->reset_last->next = pReset;
        pR->reset_last = pReset;
    }

    pR->reset_last->next = NULL;
    top_reset++;
}


/*
 * Snarf a reset section.
 */
void load_resets( FILE *fp ) {
    char letter = '\0';
    RESET_DATA *pReset = 0;
    DLString a;
    int wear_loc;

    if (areaIndexes.empty())
    {
        throw FileFormatException( "Load_resets: no #AREA seen yet in %s", strArea );
    }

    for ( ; ; )
    {
        if ( ( letter = fread_letter( fp ) ) == 'S' )
            break;

        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }

        pReset                = ( RESET_DATA* )alloc_mem( sizeof(*pReset) );
        pReset->command        = letter;

        /* if_flag */          fread_number( fp );
        pReset->arg1        = fread_number( fp );
        pReset->arg2        = fread_number( fp );

        switch (letter) {
        case 'G':
            pReset->arg3 = wear_none;
            break;
        case 'E':
            a = fread_word( fp );
            
            if (a.isNumber( )) {
                wear_loc = a.toInt( );

                if (wear_loc == -1)
                    wear_loc = wear_none;
                else if (wear_loc >= wearlocationManager->size( ))
                    throw FileFormatException( "Load_resets:  invalid equip location for %d %d", pReset->arg1, pReset->arg2 );
                else
                    wear_loc = wearlocationManager->lookup( wear_loc_flags.name( wear_loc ) );
            }
            else
                wear_loc = wearlocationManager->lookup( a );
            
            pReset->arg3 = wear_loc;
            break;
        case 'R':
            warn("...Random reset not supported: %c %d %d", letter, pReset->arg1, pReset->arg2);
            break;
        default:
            pReset->arg3 = fread_number( fp );
            break;
        }
        
        pReset->arg4        = (letter == 'P' || letter == 'M') ? fread_number(fp) : 0;

                          fread_to_eol( fp );
        
        pReset->next = 0;
        if(!reset_first) 
            reset_last = reset_first = pReset;
        else {
            reset_last->next = pReset;
            reset_last = pReset;
        }
    }
}

void bad_reset( const char *fmt, ... )
{
    va_list ap;
    
    va_start( ap, fmt );

    if (dreamland->hasOption(DL_BUILDPLOT)) 
        logerror( fmt, ap );
    else
        throw FileFormatException( fmt, ap );

    va_end( ap );
}

void fix_resets()
{
    char letter = '\0';
    RESET_DATA *pReset, *pReset_next;
    int iLastRoom = 0, iLastObj = 0;

    LogStream::sendNotice() << "Fix_resets..." << endl;

    for (pReset = reset_first; pReset; pReset = pReset_next)
    {
        RoomIndexData *pRoomIndex;
        EXIT_DATA *pexit;
        OBJ_INDEX_DATA *temp_index;

        pReset_next = pReset->next;

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
                new_reset(pRoomIndex, pReset);
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
                new_reset(pRoomIndex, pReset);
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
                new_reset(pRoomIndex, pReset);
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
                new_reset(pRoomIndex, pReset);
                iLastObj = iLastRoom;
            }
            else
                bad_reset( "Load_resets: 'G''E': bad room: %d.", pReset->arg3 );
            break;

        case 'D':
            pRoomIndex = get_room_index( pReset->arg1 );

            if ( pReset->arg2 == DIR_SOMEWHERE ) {
                if ( pRoomIndex->extra_exits.empty() ) {
                    bad_reset( "Load_resets: 'D': room %d does not contain extra exits", pReset->arg1 );
                    break;
                }
            } else {
                if ( pReset->arg2 < 0
                        || pReset->arg2 > 5
                        || ( pexit = pRoomIndex->exit[pReset->arg2] ) == 0
                        || !IS_SET( pexit->exit_info, EX_ISDOOR ) ) {
                    bad_reset( "Load_resets: 'D': exit %d from room %d not door", pReset->arg2, pRoomIndex->vnum );
                    break;
                }

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
            
            new_reset(pRoomIndex, pReset);
            break;
        }
    }

    for (auto &r: roomIndexMap) {
        RoomIndexData *pRoom = r.second;

        for(pReset = pRoom->reset_first;pReset;pReset = pReset->next) 
            switch (pReset->command) {
            case 'G': case 'O':
            case 'E': case 'P':
                if (!( get_obj_index( pReset->arg1 ) )) {
                    bad_reset( "Load_resets: %c: bad obj: %d.", pReset->command, pReset->arg1 );
                    break;
                }
                get_obj_index( pReset->arg1 )->reset_num++;
                break;
            }
    }
}


/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
    RoomIndexData *pRoomIndex;

    if (areaIndexes.empty())
    {
        throw FileFormatException( "Load_resets: no #AREA seen yet in %s.", strArea );
    }

    for ( ; ; )
    {
        int vnum;
        char letter;
        int door;

        letter        = fread_letter( fp );
        if ( letter != '#' )
        {
                throw FileFormatException( "Load_rooms: # not found in %s", strArea );
        }
    
        vnum        = fread_number( fp );
        if ( vnum == 0 )
                break;

        if (dup_room_vnum( vnum ))
            throw FileFormatException( "Load_rooms: vnum %d duplicated in %s", vnum, strArea );
        
        pRoomIndex = new RoomIndexData;
        pRoomIndex->areaIndex   = areaIndexes.back();
        pRoomIndex->vnum        = vnum;
        pRoomIndex->name        = fread_string( fp );
        pRoomIndex->description        = fread_string( fp );
        /* Area number */           fread_number( fp );
        pRoomIndex->room_flags        = fread_flag( fp );

        if ( 3000 <= vnum && vnum < 3400 )
            SET_BIT(pRoomIndex->room_flags,ROOM_LAW);

        pRoomIndex->sector_type        = fread_number( fp );

        if( pRoomIndex->sector_type < 0 )
        {
            pRoomIndex->sector_type = 0;
        }

        for ( ; ; )
        {
            letter = fread_letter( fp );

            if ( letter == 'S' )
                    break;

            if ( letter == 'C') // clan room
            {
                    char * cbuf = fread_string( fp );
                    DLString name( cbuf );
                    
                    name.toLower( );
                    pRoomIndex->clan.setName( name );
                    free_string( cbuf );
            }
            else if (letter == 'G') // guild room
            {
                    char * cbuf = fread_string( fp );
                    DLString args = cbuf;
                    pRoomIndex->guilds.fromString( args );
                    free_string( cbuf );
            }

            else if ( letter == 'D' ) // door
            {
                    EXIT_DATA *pexit;
                    EXTRA_EXIT_DATA *peexit;
                    int locks;

                    door = fread_number( fp );
                    if ( door < 0 || door > 6 )
                    {
                            throw FileFormatException( "Fread_rooms: vnum %d has bad door number in %s", vnum, strArea );
                    }

                    switch ( door )
                    {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            pexit= ( EXIT_DATA* )alloc_perm( sizeof(*pexit) );
                            pexit->description= fread_string( fp );
                            pexit->keyword= fread_string( fp );
                            locks = fread_number( fp );

                            if ( locks == 6 )
                                pexit->exit_info = fread_flag( fp );
                            else
                                pexit->exit_info = 0;

                            pexit->key= fread_number( fp );
                            pexit->u1.vnum= fread_number( fp );
                            pexit->orig_door= door;

                            switch ( locks ) {
                                case 1: pexit->exit_info = EX_ISDOOR;                break;
                                case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
                                case 3: pexit->exit_info = EX_ISDOOR | EX_NOPASS;    break;
                                case 4: pexit->exit_info = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF; break;
                                case 5: pexit->exit_info = EX_NOFLEE;                break;
                            }

                            pexit->exit_info_default = pexit->exit_info;

                            if (pexit->keyword == &str_empty[0] && IS_SET(pexit->exit_info, EX_ISDOOR))
                                pexit->keyword = str_dup("дверь");

                            pRoomIndex->exit[door]= pexit;
                            break;

                        case 6:
                            peexit        = ( EXTRA_EXIT_DATA* )alloc_perm( sizeof(*peexit) );
                            peexit->description = fread_string( fp );
                            peexit->keyword        = fread_string( fp );
                            peexit->short_desc_from        = fread_string( fp );
                            peexit->short_desc_to =  fread_string( fp );
                            peexit->room_description = fread_string( fp );
                            peexit->exit_info = fread_flag( fp );
                            peexit->exit_info_default = peexit->exit_info;
                            peexit->key = fread_number( fp );
                            peexit->u1.vnum        = fread_number( fp );
                            peexit->max_size_pass = fread_number( fp );
                            peexit->moving_from = fread_number( fp );
                            peexit->moving_mode_from = fread_number( fp );
                            peexit->moving_to = fread_number( fp );
                            peexit->moving_mode_to = fread_number( fp );
                            
                            pRoomIndex->extra_exits.push_front(peexit);
                            break;
                    }

                    top_exit++;
            }

            else if ( letter == 'E' ) // extra desc
            {
                    EXTRA_DESCR_DATA *ed;

                    ed                = new_extra_descr();
                    ed->keyword        = fread_string( fp );
                    ed->description        = fread_string( fp );
                    
                    ed->next                                = pRoomIndex->extra_descr;
                    pRoomIndex->extra_descr        = ed;
                    top_ed++;
            }
            
            else if ( letter == 'H') { // healing room
                    pRoomIndex->heal_rate = fread_number(fp);
            }
            else if ( letter == 'M') { // mana room
                    pRoomIndex->mana_rate = fread_number(fp);
            }
            else if (letter == 'X') { // some extensions
                char *token = fread_word(fp);

                if (!str_cmp(token, "liquid")) {
                    pRoomIndex->liquid.setName(fread_word(fp));
                }
                else 
                    LogStream::sendWarning() << "Unknown room token " << token << endl;
            }
            else if (letter == 'B') {
                fread_to_eol( fp );
                RoomBehaviorManager::parse( pRoomIndex, fp );
            }

            else
            {
                    throw FileFormatException( "Load_rooms: vnum %d has flag not 'DES' in %s", vnum, strArea );
            }
        }

        if ((pRoomIndex->sector_type == SECT_WATER_NOSWIM || pRoomIndex->sector_type == SECT_WATER_SWIM) 
                && pRoomIndex->liquid == liq_none)
            pRoomIndex->liquid = liq_water;

        top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;    /* OLC */

        roomIndexMap[vnum] = pRoomIndex;
        pRoomIndex->areaIndex->roomIndexes[vnum] = pRoomIndex;

        // Create new single room instance for this index (FIXME)
        pRoomIndex->create();
    }

    return;
}



/*
 * Snarf a shop section.
 */
void convert_shop( MOB_INDEX_DATA *pMobIndex, MobileBehavior::Pointer shopper )
{
    if (pMobIndex->behavior)
        throw FileFormatException( "mob %d has both shop and behavior", pMobIndex->vnum );

    std::basic_ostringstream<char> ostr;
    std::basic_istringstream<char> istr;
    XMLPersistentStreamable<MobileBehavior> behavior( MobileBehavior::NODE_NAME );

    behavior.setPointer( *shopper );
    behavior.toStream( ostr );
    istr.str( ostr.str( ) );
    pMobIndex->behavior.construct( );
    pMobIndex->behavior->load( istr );
}

MobileBehavior::Pointer allocate_shop( )
{
    MobileBehavior::Pointer shopper;

    try {
        AllocateClass::Pointer alloc = Class::allocateClass("ShopTrader");
        shopper = alloc.getDynamicPointer<MobileBehavior>( );
    }
    catch (const ExceptionClassNotFound &) {
    }

    return shopper;
}

void load_shops( FILE *fp )
{
        for ( ; ; )
        {
            int vnum = fread_number( fp );
            MOB_INDEX_DATA *pMobIndex = get_mob_index( vnum );

            if (!pMobIndex)
                throw FileFormatException("Unknown keeper for shop %d", vnum);

            MobileBehavior::Pointer shopper = allocate_shop( );
            if (shopper) {
                shopper->load( fread_dlstring_to_eol( fp ) );
                convert_shop( pMobIndex, shopper );
            }
        }

        return;
}


/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        char letter, *c;

        switch ( letter = fread_letter( fp ) )
        {
        default:
            throw FileFormatException( "Load_specials: letter '%c' not *MS in %s", letter, strArea );
        case 'S':
            return;

        case '*':
            break;

        case 'M':
            pMobIndex                = get_mob_index        ( fread_number ( fp ) );
            pMobIndex->spec_fun        = spec_lookup        ( c = fread_word   ( fp ) );
            if ( *pMobIndex->spec_fun == 0 )
            {
                throw FileFormatException( "Load_specials: 'M': vnum %d in %s.", pMobIndex->vnum, strArea );
            }

            break;
        }

        fread_to_eol( fp );
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

/*
 * Snarf can prac declarations.
 */
void load_practicer( FILE *fp )
{
    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        char letter;

        switch ( letter = fread_letter( fp ) )
        {
        default:
            throw FileFormatException( "Load_specials: letter '%c' not *MS (in %s)", letter, strArea );

        case 'S':
            return;

        case '*':
            break;

        case 'M':
            pMobIndex        = get_mob_index        ( fread_number ( fp ) );
            pMobIndex->practicer.fromString( fread_word( fp ) );
            break;
        }

        fread_to_eol( fp );
    }
}


void load_olimits(FILE *fp)
{
  int vnum;
  int limit;
  char ch;
  OBJ_INDEX_DATA *pIndex;

  for (ch = fread_letter(fp); ch != 'S'; ch = fread_letter(fp) )
    {
      switch(ch)
        {
        case 'O':
          vnum = fread_number(fp);
          limit = fread_number(fp);
          if ( (pIndex = get_obj_index(vnum)) == 0)
            {
              throw FileFormatException( "Load_olimits: bad vnum %d in %s", vnum, strArea );
            }
          else pIndex->limit = limit;
          
          break;

        case '*':
          fread_to_eol(fp);
          break;
        default:
          throw FileFormatException( "Load_olimits: bad command '%c' in %s", ch, strArea );
        }
    }
}


void load_resetmsg( FILE *fp )
{
    areaIndexes.back()->resetmsg = fread_string(fp);
}

void load_aflag( FILE *fp )
{
    areaIndexes.back()->area_flag = fread_flag(fp);
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
        
        for(RESET_DATA *pReset = pRoomIndex->reset_first;pReset;pReset = pReset->next)
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
load_areas( )
{
        ProfilerBlock prof("load areas");
        /*
         * Read in all the area files.
         */
        DLFileRead areaListFile( dreamland->getAreaDir( ), dreamland->getAreaListFile( ) );
        
        if (!areaListFile.exist( )) {
            LogStream::sendNotice( ) 
                << "Compat areas not loaded, area-list '" 
                << areaListFile.getPath( ) << "' not found" << endl;
            return;
        }

        if (!areaListFile.open( )) {
            throw FileFormatException(
                    "Error opening file %s: %s",
                    areaListFile.getPath( ).c_str( ),
                    strerror(errno));
        }
        
        FILE *fpList = areaListFile.getFP( );
        
        for ( ; ; )
        {
                strcpy( strArea, fread_word( fpList ) );
                if ( strArea[0] == '$' )
                    break;

                DLFileRead areaFile( dreamland->getAreaDir( ), strArea );

                if (!areaFile.open( )) {
                    LogStream::sendSystem( ) << "areafile " << strArea << endl;
                    break;
                }

                fpArea = areaFile.getFP( );

                LogStream::sendNotice( ) << "Area file '" << areaFile.getPath( ) << "' loading..." << endl;
                new_area_file(strArea);

                for ( ; ; ) {
                        char *word;

                        if ( fread_letter( fpArea ) != '#' )
                        {
                                throw FileFormatException(
                                        "Boot_db: # not found.");;
                        }

                        word = fread_word( fpArea );
                        notice("...loading section %s", word);

                        if (word[0] == '$')  
                            break;
                        else if ( !str_cmp( word, "AREADATA" ) ) new_load_area(fpArea);
                        else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (fpArea);
                        else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
                        else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (fpArea);
                        else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (fpArea);
                        else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
                        else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (fpArea);
                        else if ( !str_cmp( word, "OLIMITS"  ) ) load_olimits (fpArea);
                        else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(fpArea);
                        else if ( !str_cmp( word, "PRACTICERS" ) ) load_practicer(fpArea);
                        else if ( !str_cmp( word, "RESETMESSAGE" ) ) load_resetmsg(fpArea);
                        else if ( !str_cmp( word, "FLAG" ) )         load_aflag(fpArea);
                        else
                        {
                                LogStream::sendError( ) << "Boot_db: bad section name " << word << endl;
                                fread_letter( fpArea );
                        }
                }

                fpArea = 0;
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

class AreaLoadTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<AreaLoadTask> Pointer;

    virtual void run( )
    {
        if (DLScheduler::getThis()->getCurrentTick( ) == 0) {
            try {
                load_areas( );
            } catch(const Exception &ex) {
                LogStream::sendWarning() << "failed to load compat areas: " << ex.what( ) << endl;
            }
        }
    }
    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 5;
    }
};

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

PluginInitializer<AreaLoadTask> initLoadCompatAreas;
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

