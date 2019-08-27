/* $Id: npcharacter.h,v 1.1.2.10.6.14 2009/09/17 18:08:56 rufina Exp $
 * 
 * ruffina, 2004
 */

#ifndef NPCHARACTER_H
#define NPCHARACTER_H

#include <map>

#include "xmlstreamable.h"
#include "character.h"
#include "merc.h"
#include "mobilebehavior.h"
#include "profession.h"
#include "russianstring.h"

class NPCharacter : public Character {
XML_OBJECT;
public:
    typedef ::Pointer<NPCharacter> Pointer;

public:
    NPCharacter( );
    virtual ~NPCharacter( );
    
    // recycle
    virtual void init( );

    // gate to pc/npc info
    virtual PCharacter *getPC();
    virtual NPCharacter *getNPC();
    virtual const PCharacter *getPC( ) const;
    virtual const NPCharacter *getNPC( ) const;
    virtual bool is_npc( ) const;
    
    // additional set-get methods
    virtual const GlobalBitvector & getWearloc( );
    virtual short getModifyLevel( ) const;

    // set-get methods for string fields
    virtual void setDescription( const DLString& );
    virtual const char * getDescription( ) const;
    void setShortDescr( const DLString& );
    void setShortDescr( const char * );
    inline const char * getShortDescr( ) const;
    void setLongDescr( const char * );
    void setLongDescr( const DLString& );
    inline const char * getLongDescr( ) const;
    
    inline const char * getRealShortDescr( ) const;
    inline const char * getRealLongDescr( ) const;
    inline const char * getRealDescription( ) const;

    // name and sex formatting
    inline const char *getNameP( ) const;
    virtual DLString getNameP( char gram_case ) const;
    virtual NounPointer toNoun( const DLObject *forWhom = NULL, int flags = 0 ) const;
    virtual void updateCachedNoun( );
    
    // npc skills
    virtual int applyCurse( int );

    // stats
    virtual int getCurrStat( int );

    // trust and immortality 
    virtual bool isCoder( ) const;
    virtual int get_trust( ) const;
    virtual bool is_immortal( ) const;

    // misc
    virtual bool is_vampire( ) const;
    virtual bool is_mirror( ) const;

    // configuration
    virtual PlayerConfig::Pointer getConfig( ) const;

public:
    MOB_INDEX_DATA *        pIndexData;
    AREA_DATA *                zone;
    /** Room VNUM where this mob was reset. */
    int reset_room;
    
protected:    
    char *  description;
    char *  short_descr;
    char *  long_descr;
    RussianString::Pointer cachedNoun;
    
public:    
    int                group;
    int        off_flags;
    int                damage[3];
    int                start_pos;
    int                default_pos;
    
    // behavior
    ProgWrapper<SPEC_FUN> spec_fun;
    XMLPersistentStreamable<MobileBehavior> behavior;

    // switch
    PCharacter                *switchedFrom;
};

inline const char * NPCharacter::getNameP( ) const 
{ 
    return Character::getNameP( ); 
}
inline const char * NPCharacter::getShortDescr( ) const
{
    return short_descr ? short_descr : pIndexData->short_descr;
}
inline const char * NPCharacter::getLongDescr( ) const
{
    return long_descr ? long_descr : pIndexData->long_descr;
}
inline const char * NPCharacter::getRealShortDescr( ) const
{
    return short_descr;
}
inline const char * NPCharacter::getRealLongDescr( ) const
{
    return long_descr;
}
inline const char * NPCharacter::getRealDescription( ) const
{
    return description;
}
#endif
