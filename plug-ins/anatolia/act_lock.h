/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ACT_LOCK_H
#define ACT_LOCK_H

#include "dlobject.h"
#include "dlstring.h"
#include "bitstring.h"

class Character;
class Object;
struct exit_data;
struct extra_exit_data;

class Keyhole : public virtual DLObject {
public:
    typedef ::Pointer<Keyhole> Pointer;

    Keyhole( );
    Keyhole( Character * );
    Keyhole( Character *, Object * );
    virtual ~Keyhole( );

    bool doPick( const DLString & );
    bool doClose( ); /* TODO */
    bool doOpen( )   /* TODO */;
    bool doLock( )   /* TODO */;
    bool doUnlock( ) /* TODO */;
    bool doExamine( );
    bool doLore( ostringstream & );
    
    void record( Object * );
    bool isPickProof( );
    bool isCloseable( );
    bool isLockable( );
    int getLockType( );
    bool hasKey( );
    virtual int getKey( ) = 0;
    virtual DLString getDescription( ) = 0;

    static Pointer create( Character *, const DLString & );
    static Pointer locate( Character *, Object * );
    static const int MAX_KEY_TYPES;
    static const int LOCK_VALUE_MULTI;
    static const int LOCK_VALUE_BLANK;
    static const int ERROR_KEY_TYPE;

protected:
    virtual int getLockFlags( ) = 0;
    virtual void setLockFlags(int flags) = 0;
    virtual bitstring_t bitCloseable( ) = 0;
    virtual bitstring_t bitLocked( ) = 0;
    virtual bitstring_t bitPickProof( ) = 0;
    virtual bitstring_t bitUnlockable( ) = 0;
    virtual void unlock( );
    virtual bool checkGuards( );
    
    virtual void msgTryPickSelf( ) = 0;
    virtual void msgTryPickOther( ) = 0;

    Character *ch;
    Object *lockpick;
    Object *keyring;
    Object *key;

private:
    bool checkLockPick( Object * );
    void argsPickLock( const DLString & );
    bool findLockpick( );

    DLString argLockpick, argKeyring;
};


class ItemKeyhole : public virtual Keyhole {
public:
    ItemKeyhole( Character *, Object * );
    ItemKeyhole( Character *, Object *, Object * );

    virtual DLString getDescription( );

protected:
    virtual int getLockFlags( );
    virtual void setLockFlags(int flags);
    virtual bool checkGuards( );
    virtual void unlock( );

    virtual void msgTryPickSelf( );
    virtual void msgTryPickOther( );

    Object *obj;
};

class ContainerKeyhole : public ItemKeyhole {
public:
    typedef ::Pointer<ContainerKeyhole> Pointer;

    ContainerKeyhole( Character *, Object * );
    ContainerKeyhole( Character *, Object *, Object * );

    virtual int getKey( );

protected:
    virtual bitstring_t bitPickProof( );
    virtual bitstring_t bitLocked( );
    virtual bitstring_t bitCloseable( );
    virtual bitstring_t bitUnlockable( );
};

class ExitKeyhole : public virtual Keyhole {
public:
protected:
    virtual bitstring_t bitPickProof( );
    virtual bitstring_t bitLocked( );
    virtual bitstring_t bitCloseable( );
    virtual bitstring_t bitUnlockable( );
};

class PortalKeyhole : public ItemKeyhole, public ExitKeyhole {
public:
    typedef ::Pointer<PortalKeyhole> Pointer;

    PortalKeyhole( Character *, Object * );
    PortalKeyhole( Character *, Object *, Object * );

    virtual int getKey( );
};

class DoorKeyhole : public ExitKeyhole {
public:
    typedef ::Pointer<DoorKeyhole> Pointer;

    DoorKeyhole( Character *, Room *, int );
    DoorKeyhole( Character *, Room *, int, Object * );

    virtual int getKey( );
    virtual DLString getDescription( );

protected:
    virtual int getLockFlags( );
    virtual void setLockFlags(int flags);
    virtual void unlock( );
    
    virtual void msgTryPickSelf( );
    virtual void msgTryPickOther( );

    int door;
    exit_data *pexit, *pexit_rev;
    Room *room, *to_room;
};

class ExtraExitKeyhole : public ExitKeyhole {
public:
    typedef ::Pointer<ExtraExitKeyhole> Pointer;
    
    ExtraExitKeyhole( Character *, Room *, extra_exit_data * );
    ExtraExitKeyhole( Character *, Room *, extra_exit_data *, Object * );

    virtual int getKey( );
    virtual DLString getDescription( );

protected:
    virtual int getLockFlags( );
    virtual void setLockFlags(int flags);
    
    virtual void msgTryPickSelf( );
    virtual void msgTryPickOther( );

    extra_exit_data *peexit;
    Room *room;
};

#endif
