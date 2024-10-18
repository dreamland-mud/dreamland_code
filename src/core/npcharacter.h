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
#include "inflectedstring.h"

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
    virtual short getModifyLevel( ) const;

    // set-get methods for string fields
    virtual void setDescription( const DLString&, lang_t lang );
    virtual const char * getDescription( lang_t lang ) const;
    virtual const XMLMultiString & getDescription( ) const;
    virtual void setDescription( const XMLMultiString & );     
    void setShortDescr( const DLString&, lang_t lang );
    void setShortDescr( const char *, lang_t lang );
    const char * getShortDescr( lang_t lang ) const;
    void setLongDescr( const char *, lang_t lang );
    void setLongDescr( const DLString&, lang_t lang );
    const char * getLongDescr( lang_t lang ) const;
    
    const char * getRealShortDescr( lang_t lang ) const;
    const char * getRealLongDescr( lang_t lang ) const;
    const char * getRealDescription( lang_t lang ) const;

    // name and sex formatting
    virtual DLString getNameP( char gram_case ) const;
    virtual NounPointer toNoun( const DLObject *forWhom = NULL, int flags = 0 ) const;
    virtual void updateCachedNoun( );
    
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
    virtual PlayerConfig getConfig( ) const;

public:
    MOB_INDEX_DATA *        pIndexData;
    AreaIndexData *                zone;
    /** Room VNUM where this mob was reset. */
    int reset_room;
    
protected:    
    XMLMultiString description;
    XMLMultiString short_descr;
    XMLMultiString long_descr;
    InflectedString::Pointer cachedNoun;
    
public:    
    int                group;
    int                off_flags;
    int                damage[3];
    int                start_pos;
    int                default_pos;
    
    // behavior
    ProgWrapper<SPEC_FUN> spec_fun;
    XMLPersistentStreamable<MobileBehavior> behavior;

    // switch
    PCharacter                *switchedFrom;
};


#endif
