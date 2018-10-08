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
    Affect            *  affected;
    OBJ_INDEX_DATA    *  pIndexData;
    Room              *  in_room;
    bool                enchanted;
    
    int              item_type;
    int                  extra_flags;
    int                  wear_flags;
    WearlocationReference wear_loc;
    int              weight;
    int                  cost;
    int              level;
    int              condition;
    int              timer;
    time_t           timestamp;
    int                  value  [5];
    char              *  from;
    bool                extracted;
    int                  water_float;

    char              *  killer;  // for corpse
    int                  count;

    XMLPersistentStreamable<ObjectBehavior> behavior;


    // member funcions.

    inline bool is_obj_stat(int stat)
    {
        return IS_SET(extra_flags, stat);
    }

    inline bool can_wear(int part)
    {
        return IS_SET(wear_flags,part);
    }

    inline int getWeightMultiplier( ) const
    {
        return item_type == ITEM_CONTAINER ? value[4] : 100;
    }

    const char *get_cond_alias(void);
    int floating_time(void);
    bool may_float(void);
    Character * getCarrier( );
    Room * getRoom( );
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

    virtual NounPointer toNoun( const DLObject *forWhom = NULL, int flags = 0 ) const;
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


#endif
