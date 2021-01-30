/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          object.h  -  description
                             -------------------
    begin                : Fri Apr 27 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef OBJECT_H
#define OBJECT_H

#include "xmlvariablecontainer.h"
#include "xmlstreamable.h"
#include "core/fenia/wrappertarget.h"
#include "merc.h"
#include "objectbehavior.h"
#include "wearlocation.h"
#include "nounholder.h"
#include "russianstring.h"
#include "affectlist.h"

class Character;

/**
 * @author Igor S. Petrenko
 */
class Object : public Grammar::NounHolder,
               public XMLVariableContainer, 
               public WrapperTarget 
{
XML_OBJECT
public:  
    typedef ::Pointer<Object> Pointer;
    typedef ::Pointer<ObjectBehavior> ObjectBehaviorPointer;

public:
    static const DLString TYPE;

public:
    Object( );
    virtual ~Object( );

    virtual void extract( );

protected:
    long long ID;
    char              *  name;
    char              *  owner;
    char              *  short_descr;
    char              *  description;
    char              *  material;
    RussianString::Pointer cachedNoun;
    int                  value  [5];

public:
    Object            *  next;
    Object            *  prev;
    Object            *  next_content;
    Object            *  contains;
    Object            *  in_obj;
    DLString pocket;
    Object            *  on;
    Character          *  carried_by;
    EXTRA_DESCR_DATA  *  extra_descr;
    AffectList  affected;
    OBJ_INDEX_DATA    *  pIndexData;
    Room              *  in_room;
    bool                enchanted;
    
    int              item_type;
    int             extra_flags;
    int             wear_flags;
    WearlocationReference wear_loc;
    int              weight;
    int                  cost;
    int              level;
    int              condition;
    int              timer;
    time_t           timestamp;
    char              *  from;
    bool                extracted;
    int                  water_float;

    char              *  killer;  // for corpse
    int                  count;
    Grammar::MultiGender gram_gender;
    Properties           properties;
    XMLPersistentStreamable<ObjectBehavior> behavior;

    /** ID of a container where this item was reset. */
    long long reset_obj;
    /** ID of a mobile in whose inventory this item was reset. */
    long long reset_mob;
    /** Room VNUM where this item was reset on the floor. */
    int reset_room;


    // member funcions.

    inline bool is_obj_stat(int stat)
    {
        return IS_SET(extra_flags, stat);
    }

    inline bool can_wear(int part)
    {
        return IS_SET(wear_flags,part);
    }

    int getWeightMultiplier( ) const;

    const char *get_cond_alias(void);
    int floating_time(void);
    bool may_float(void);
    Character * getCarrier( );
    Room * getRoom( );
    list<Object *> getItems();
    void addExtraDescr( const DLString &keys, const DLString &value );
    bool mustDisappear( Character * );
    bool isAntiAligned( Character * ) const;
    bool wasAntiAligned( Character * ) const;

    int getNumber( ) const;
    int getWeight( ) const;
    int getTrueWeight( ) const;
    
    bool isAffected( int ) const;


    inline long long getID( ) const;
    inline void setID( long long );

    inline const char * getRealName( ) const;
    inline const char * getRealShortDescr( ) const;
    inline const char * getRealDescription( ) const;
    inline const char * getRealMaterial( ) const;
    inline const char * getOwner( ) const;

    inline const char * getName( ) const;
    inline const char * getShortDescr( ) const;
    inline const char * getDescription( ) const;
    inline const char * getMaterial( ) const;

    void updateCachedNoun( );

    void setOwner( const char * );
    void setName( const char * );
    void setShortDescr( const char * );
    void setDescription( const char * );
    void setMaterial( const char * );
    
    void fmtName( const char *, ... );
    void fmtShortDescr( const char *, ... );
    void fmtDescription( const char *, ... );
    void fmtMaterial( const char *, ... );
    
    DLString getFirstName( ) const;
    DLString getShortDescr( char );
    bool hasOwner( const Character * ) const;

    /** Return value of a given property or an empty string if not found. */
    DLString getProperty(const DLString &key) const;

    /** Remove property from instance. Can still have inherited property from the proto. */
    void removeProperty(const DLString &key);

    /** Add property to the instance. */
    void addProperty(const DLString &key, const DLString &value);

    virtual NounPointer toNoun( const DLObject *forWhom = NULL, int flags = 0 ) const;

    /** Return instance or index data value[0], depending on item type. */
    inline int value0() const;

    /** Return instance or index data value[1], depending on item type. */
    inline int value1() const;

    /** Return instance or index data value[2], depending on item type. */
    inline int value2() const;

    /** Return instance or index data value[3], depending on item type. */
    inline int value3() const;

    /** Return instance or index data value[4], depending on item type. */
    inline int value4() const;

    inline void value0(int);
    inline void value1(int);
    inline void value2(int);
    inline void value3(int);
    inline void value4(int);

    int valueByIndex(int index) const;
    void valueByIndex(int index, int value);
    bool getsValueFromProto(int index) const;

protected:
    int itemOrProtoValue(int index) const;
    void itemOrProtoValue(int index, int value);
};


long long Object::getID( ) const
{
    return ID;
}
void Object::setID( long long id )
{
    ID = id;
}

inline const char * Object::getRealName( ) const
{
    return name;
}
inline const char * Object::getRealShortDescr( ) const
{
    return short_descr;
}
inline const char * Object::getRealDescription( ) const
{
    return description;
}
inline const char * Object::getRealMaterial( ) const
{
    return material;
}
inline const char * Object::getOwner( ) const
{
    return owner;
}

inline const char * Object::getName( ) const
{
    return name ? name : pIndexData->name;    
}
inline const char * Object::getShortDescr( ) const
{
    return short_descr ? short_descr : pIndexData->short_descr;
}
inline const char * Object::getDescription( ) const
{
    return description ? description : pIndexData->description;
}
inline const char * Object::getMaterial( ) const
{
    return material ? material : pIndexData->material;
}

inline void Object::value0(int v)
{
    itemOrProtoValue(0, v);
}

inline void Object::value1(int v)
{
    itemOrProtoValue(1, v);
}

inline void Object::value2(int v)
{
    itemOrProtoValue(2, v);
}

inline void Object::value3(int v)
{
    itemOrProtoValue(3, v);
}

inline void Object::value4(int v)
{
    itemOrProtoValue(4, v);
}

inline int Object::value0() const
{
    return itemOrProtoValue(0);
}

inline int Object::value1() const
{
    return itemOrProtoValue(1);
}

inline int Object::value2() const
{
    return itemOrProtoValue(2);
}

inline int Object::value3() const
{
    return itemOrProtoValue(3);
}

inline int Object::value4() const
{
    return itemOrProtoValue(4);
}


#endif
