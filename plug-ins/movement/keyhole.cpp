#include <string.h>
#include "keyhole.h"
#include "skillreference.h"
#include "core/object.h"
#include "character.h"
#include "room.h"
#include "directions.h"
#include "save.h"
#include "skill_utils.h"
#include "string_utils.h"
#include "loadsave.h"
#include "act.h"
#include "vnum.h"
#include "merc.h"
#include "def.h"

GSN(golden_eye);
GSN(pick_lock);

/*------------------------------------------------------------------------
 * Keyhole base class
 *-----------------------------------------------------------------------*/
const int Keyhole::MAX_KEY_TYPES     = 8;
const int Keyhole::LOCK_VALUE_MULTI  = -1;
const int Keyhole::LOCK_VALUE_BLANK  = -2;
const int Keyhole::ERROR_KEY_TYPE    = -3;

Keyhole::Keyhole( )
          : ch( NULL ), lockpick( NULL ), keyring( NULL ), key( NULL )
{
}

Keyhole::Keyhole( Character *ach ) 
             : ch( ach )
{
}

Keyhole::Keyhole( Character *ach, Object *akey ) 
             : ch( ach ), key( akey )
{
}

Keyhole::~Keyhole( )
{
}

Keyhole::Pointer Keyhole::locate( Character *ch, Object *key )
{
    Keyhole::Pointer null;
    int keyVnum = key->pIndexData->vnum;
    
    for (auto &room: roomInstances) {
        if (!ch->can_see( room ))
            continue;

        for (int d = 0; d < DIR_SOMEWHERE; d++)
            if (room->exit[d] && room->exit[d]->key == keyVnum)
                if (!room->exit[d]->u1.to_room || ch->can_see( room->exit[d] ))
                    return DoorKeyhole::Pointer( NEW, ch, room, d, key );

        for (auto &ex: room->extra_exits)
            if (ex->key == keyVnum)
                if (ch->can_see( ex ))
                    return ExtraExitKeyhole::Pointer( NEW, ch, room, ex, key );
    }

    for (Object *obj = object_list; obj; obj = obj->next) {
        if (!ch->can_see( obj ) 
            || !ch->can_see( obj->getRoom( ) )
            || (obj->getCarrier( ) && !ch->can_see( obj->getCarrier( ) )))
            continue;

        if (obj->item_type == ITEM_PORTAL && obj->value4() == keyVnum)
            return PortalKeyhole::Pointer( NEW, ch, obj, key );

        if (obj->item_type == ITEM_CONTAINER && obj->value2() == keyVnum)
            return ContainerKeyhole::Pointer( NEW, ch, obj, key );
    }

    return null;
}

Keyhole::Pointer Keyhole::create( Character *ch, const DLString &arg )
{
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;
    bool canBeDoor = direction_lookup(arg.c_str()) >= 0;
    Keyhole::Pointer null;

    if ((peexit = ch->in_room->extra_exits.find(arg))
                && ch->can_see( peexit ))
    {
        return ExtraExitKeyhole::Pointer( NEW, ch, ch->in_room, peexit );
    }

    if (( door = find_exit( ch, arg.c_str( ), 
                            FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY) ) >= 0)
    {
        return DoorKeyhole::Pointer( NEW, ch, ch->in_room, door );
    }

    if (!canBeDoor && ( obj = get_obj_here( ch, arg.c_str( ) ) )) {
        if (obj->item_type == ITEM_PORTAL)
            return PortalKeyhole::Pointer( NEW, ch, obj );

        if (obj->item_type == ITEM_CONTAINER)
            return ContainerKeyhole::Pointer( NEW, ch, obj );

        oldact("В $o6 нет замочной скважины.", ch, obj, 0, TO_CHAR );
        return null;
    }

    return null;
}

void Keyhole::argsPickLock( const DLString &arg )
{
    char buf[MAX_INPUT_LENGTH];
    char *pbuf = buf;

    strcpy( buf, arg.c_str( ) );
    
    while (*pbuf++) {
        if (*pbuf == ':') {
            argLockpick = pbuf + 1;
            *pbuf = 0;
            argKeyring = buf;
            return;
        }
    }

    argLockpick = arg;
}

bool Keyhole::isPickProof( )
{
    return IS_SET(getLockFlags( ), bitPickProof( ));
}

bool Keyhole::isCloseable( )
{
    return IS_SET(getLockFlags( ), bitCloseable( ));
}

bool Keyhole::hasKey( )
{
    return getKey( ) > 0;
}

bool Keyhole::isLockable( )
{
    if (!hasKey( ))
        return false;
    
    if (bitUnlockable( ) == 0)
        return true;
        
    return !IS_SET(getLockFlags( ), bitUnlockable( ));
}

int Keyhole::getLockType( )
{
    return (hasKey( ) ? getKey( ) % MAX_KEY_TYPES : ERROR_KEY_TYPE);
}

bool Keyhole::doPick( const DLString &arg )
{
    bitstring_t flags = getLockFlags( );

    if (!isLockable( )) {
        ch->pecho( "Здесь нет замочной скважины." );
        return false;
    }

    if (!IS_SET(flags, bitLocked( ))) {
        ch->pecho( "Здесь уже не заперто." );
        return false;
    }
    
    if (!checkGuards( ))
        return false;
    
    if (isPickProof( )) {
        ch->pecho( "Этот замок защищен от взлома." );
        return false;
    }
    
    argsPickLock( arg );

    if (!findLockpick( ))
        return false;
    
    msgTryPickOther( );

    if (!checkLockPick( lockpick )) {
        oldact("Ты не смо$gгло|г|гла пропихнуть $o4 в эту замочную скважину.", ch, lockpick, 0, TO_CHAR );
        return false;
    }
    
    msgTryPickSelf( );

    if (number_percent( ) >= gsn_pick_lock->getEffective( ch )) {
        if (number_percent( ) >= gsn_pick_lock->getEffective( ch )
            && number_percent( ) > lockpick->value1()) 
        {
            ch->pecho( "  ...но, слишком резко надавив, ломаешь %1$P2!", lockpick );
            extract_obj( lockpick );
        }
        else
            ch->pecho( "  ...но твои манипуляции ни к чему не приводят." );

        gsn_pick_lock->improve( ch, false );
        return false;
    }
    
    unlock( );

    gsn_pick_lock->improve( ch, true );
    record( lockpick );
    return true;
}

void Keyhole::unlock( )
{
    setLockFlags(getLockFlags() & ~bitLocked());
    ch->in_room->echo( POS_RESTING, "*Щелк*" );
}

bool Keyhole::checkLockPick( Object *o )
{
    if (o->item_type != ITEM_LOCKPICK)
        return false;
        
    if (!ch->can_see( o ) && !ch->can_hear( o ))
        return false;
        
    if (o->value0() == LOCK_VALUE_MULTI)
        return true;
        
    return o->value0() == getLockType( );
}

bool Keyhole::checkGuards( )
{
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (rch->is_npc( )
                && IS_AWAKE(rch)
                && ch->getModifyLevel( ) + 5 < rch->getModifyLevel( ))
        {
            oldact("$C1 маячит перед тобой, загораживая вожделенный замок.", ch, 0, rch, TO_CHAR );
            return false;
        }

    return true;
}

bool Keyhole::findLockpick( )
{
    if (!argKeyring.empty( )) {
        if (!( keyring = get_obj_list_type( ch, argKeyring, ITEM_KEYRING, ch->carrying ) )) {
            ch->pecho( "У тебя нет такого кольца для ключей." );
            return false;
        }

        if (!( lockpick = get_obj_list_type( ch, argLockpick, ITEM_LOCKPICK, keyring->contains ) )) {
            oldact("На $o6 не нанизано ничего похожего.", ch, keyring, 0, TO_CHAR );
            return false;
        }
    }
    else if (!( lockpick = get_obj_list_type( ch, argLockpick, ITEM_LOCKPICK, ch->carrying )) ) {
        ch->pecho( "У тебя нет такой отмычки." );
        return false;
    }

    return true;
}

// Remember the door name on this key/lockpick after successful use
// TODO multi-language descriptions
void Keyhole::record( Object *obj )
{
    char *ed_text;
    DLString edEntry = getDescription().ruscase('2');
    DLString edKey = String::toString(obj->getKeyword( ));
    DLString edText = obj->getExtraDescr(edKey, LANG_DEFAULT);

    if (edText.empty()) {
        edText = "Подходит для открытия:\n";
    }
    
    if (edText.find( edEntry ) != DLString::npos)
        return;

    obj->addExtraDescr(edKey, 
                       edText + edEntry + "\n",
                       LANG_DEFAULT );
}

bool Keyhole::doLore( ostringstream &buf )
{
    if (number_percent( ) >= gsn_golden_eye->getEffective( ch ) + skill_level_bonus(*gsn_golden_eye, ch))
        return false;

    if (!isLockable( )) 
        buf << "Это ключ от поломанного замка." << endl;
    else if (isPickProof( )) 
        buf << "Открывает защищенный от взлома замок на "
            << getDescription( ).ruscase( '6' ) << "." << endl;
    else
        buf << "Открывает замок на "
            << getDescription( ).ruscase( '6' ) << "." << endl;
    
    if (key->value0() == 0)
        buf << "Рассыпается, лежа в кармане." << endl;

    if (key->value1() > 0)
        buf << "Рассыпается, лежа на земле." << endl;

    gsn_golden_eye->improve( ch, true );
    return true;
}

bool Keyhole::doExamine( )
{
    if (!isLockable( ))
        return false;

    if (number_percent( ) >= gsn_golden_eye->getEffective( ch ) + skill_level_bonus(*gsn_golden_eye, ch))
        return false;
        
    if (isPickProof( )) 
        oldact("Замок защищен от взлома.", ch, 0, 0, TO_CHAR );
    else {
        oldact("Замок не устоит перед хорошим взломщиком.", ch, 0, 0, TO_CHAR );

        for (Object *o = ch->carrying; o; o = o->next_content) {
            if (checkLockPick( o )) {
                ch->pecho( "%1$^O1 тихонько звяка%1$nет|ют.", o );
                continue;
            }

            if (!ch->can_see( o ) && !ch->can_hear( o ))
                continue;

            if (o->item_type == ITEM_KEYRING) 
                for (Object *l = o->contains; l; l = l->next_content)
                    if (checkLockPick( l )) 
                        ch->pecho( "%1$^O1 на %2$O6 тихонько звяка%1$nет|ют.", o, l );
        }
    }
    
    gsn_golden_eye->improve( ch, true );
    return true;
}


/*------------------------------------------------------------------------
 * ItemKeyhole 
 *-----------------------------------------------------------------------*/
ItemKeyhole::ItemKeyhole( Character *ch, Object *obj )
{
    this->ch = ch;
    this->obj = obj;
}
ItemKeyhole::ItemKeyhole( Character *ch, Object *obj, Object *key )
{
    this->ch = ch;
    this->obj = obj;
    this->key = key;
}
int ItemKeyhole::getLockFlags( )
{
    return obj->value1();
}
void ItemKeyhole::setLockFlags(int flags)
{
    obj->value1(flags);
}
bool ItemKeyhole::checkGuards( )
{
    return !obj->in_room || Keyhole::checkGuards( );
}
void ItemKeyhole::unlock( )
{
    Keyhole::unlock( );

    if (obj->in_room)
        save_items( obj->in_room );
}
void ItemKeyhole::msgTryPickSelf( )
{
    oldact("Ты осторожно поворачиваешь $o4 в замочной скважине $O2.", ch, lockpick, obj, TO_CHAR );
}
void ItemKeyhole::msgTryPickOther( )
{
    oldact("$c1 ковыряется в замке $O2.", ch, lockpick, obj, TO_ROOM );
}
DLString ItemKeyhole::getDescription( )
{
    DLString buf;

    // TODO multi-langage descriptions
    buf << obj->getShortDescr(LANG_DEFAULT);
    if (obj->getCarrier( ) == 0)
        buf << " из '" << obj->getRoom()->getName() << "'";

    return buf;
}
/*------------------------------------------------------------------------
 * ContainerKeyhole 
 *-----------------------------------------------------------------------*/
ContainerKeyhole::ContainerKeyhole( Character *ch, Object *obj )
          : ItemKeyhole( ch, obj )
{
}
ContainerKeyhole::ContainerKeyhole( Character *ch, Object *obj, Object *key )
          : ItemKeyhole( ch, obj, key )
{
}
bitstring_t ContainerKeyhole::bitPickProof( )
{
    return CONT_PICKPROOF;
}
bitstring_t ContainerKeyhole::bitLocked( )
{
    return CONT_LOCKED;
}
bitstring_t ContainerKeyhole::bitCloseable( ) 
{
    return CONT_CLOSEABLE;
}
bitstring_t ContainerKeyhole::bitUnlockable( ) 
{
    return 0;
}
int ContainerKeyhole::getKey( )
{
    return obj->value2();
}
/*------------------------------------------------------------------------
 * ExitKeyhole 
 *-----------------------------------------------------------------------*/
bitstring_t ExitKeyhole::bitPickProof( )
{
    return EX_PICKPROOF;
}
bitstring_t ExitKeyhole::bitLocked( )
{
    return EX_LOCKED;
}
bitstring_t ExitKeyhole::bitCloseable( ) 
{
    return EX_ISDOOR;
}
bitstring_t ExitKeyhole::bitUnlockable( ) 
{
    return EX_NOLOCK;
}
/*------------------------------------------------------------------------
 * PortalKeyhole 
 *-----------------------------------------------------------------------*/
PortalKeyhole::PortalKeyhole( Character *ch, Object *obj )
          : ItemKeyhole( ch, obj )
{
}
PortalKeyhole::PortalKeyhole( Character *ch, Object *obj, Object *key )
          : ItemKeyhole( ch, obj, key )
{
}
int PortalKeyhole::getKey( )
{
    return obj->value4();
}
/*------------------------------------------------------------------------
 * DoorKeyhole 
 *-----------------------------------------------------------------------*/
DoorKeyhole::DoorKeyhole( Character *ch, Room *room, int door )
{
    this->ch = ch;
    this->room = room;
    this->door = door;
    pexit = room->exit[door];
    to_room = pexit->u1.to_room;
    pexit_rev = (to_room ? to_room->exit[dirs[door].rev] : 0);
}

DoorKeyhole::DoorKeyhole( Character *ch, Room *room, int door, Object *key )
{
    this->ch = ch;
    this->room = room;
    this->door = door;
    this->key = key;
    pexit = room->exit[door];
    to_room = pexit->u1.to_room;
    pexit_rev = (to_room ? to_room->exit[dirs[door].rev] : 0);
}

int DoorKeyhole::getLockFlags( )
{
    return pexit->exit_info;
}
void DoorKeyhole::setLockFlags(int flags)
{
    pexit->exit_info = flags;
}
void DoorKeyhole::unlock( )
{
    ExitKeyhole::unlock( );
    
    if (pexit_rev && pexit_rev->u1.to_room == room) {
        REMOVE_BIT(pexit_rev->exit_info, bitLocked( ));
        to_room->echo( POS_RESTING, "Дверной замок щелкает." );
    }
}
void DoorKeyhole::msgTryPickSelf( )
{
    oldact("Ты осторожно поворачиваешь $o4 в замочной скважине.", ch, lockpick, 0, TO_CHAR );
}
void DoorKeyhole::msgTryPickOther( )
{
    oldact("$c1 ковыряется в замке двери $t отсюда.", ch, dirs[door].leave, 0, TO_ROOM );
}
DLString DoorKeyhole::getDescription( )
{
    DLString buf;
    
    buf << "двер|ь|и|и|ь|ью|и из '" << room->getName() << "'";
    if (to_room)
        buf <<  " в '" << to_room->getName() << "'";

    return buf;
}
int DoorKeyhole::getKey( )
{
    return pexit->key;
}
/*------------------------------------------------------------------------
 * ExtraExitKeyhole 
 *-----------------------------------------------------------------------*/
ExtraExitKeyhole::ExtraExitKeyhole( Character *ch, Room *room, EXTRA_EXIT_DATA *peexit )
{
    this->ch = ch;
    this->room = room;
    this->peexit = peexit;
}

ExtraExitKeyhole::ExtraExitKeyhole( Character *ch, Room *room, EXTRA_EXIT_DATA *peexit, Object *key )
{
    this->ch = ch;
    this->room = room;
    this->peexit = peexit;
    this->key = key;
}

int ExtraExitKeyhole::getLockFlags( )
{
    return peexit->exit_info;
}

void ExtraExitKeyhole::setLockFlags(int flags)
{
    peexit->exit_info = flags;
}

void ExtraExitKeyhole::msgTryPickSelf( )
{
    oldact("Ты осторожно поворачиваешь $o4 в замочной скважине $N2.", ch, lockpick, peexit->short_desc_from.get(LANG_DEFAULT).c_str(), TO_CHAR );
}
void ExtraExitKeyhole::msgTryPickOther( )
{
    oldact("$c1 ковыряется в замке $N2.", ch, 0, peexit->short_desc_from.get(LANG_DEFAULT).c_str(), TO_ROOM );
}

DLString ExtraExitKeyhole::getDescription( )
{
    return peexit->short_desc_from.get(LANG_DEFAULT);
}
int ExtraExitKeyhole::getKey( )
{
    return peexit->key;
}
