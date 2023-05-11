/* $Id: mobilebehavior.h,v 1.1.2.6.6.10 2009/01/01 13:25:28 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef MOBILEBEHAVIOR_H
#define MOBILEBEHAVIOR_H

#include <sstream>

#include "xmlvariablecontainer.h"
#include "xmlpersistent.h"

class Character;
class NPCharacter;
class Object;
class SpellTarget;

class MobileBehavior : public XMLVariableContainer {
XML_OBJECT
public:    
    typedef ::Pointer<MobileBehavior> Pointer;
    
    MobileBehavior( );
    virtual ~MobileBehavior( );

    virtual void setChar( NPCharacter * );
    virtual void unsetChar( );
    NPCharacter * getChar( );

    static const DLString NODE_NAME;

    virtual bool spec( ) { return false; }
    
    virtual void bribe( Character *victim, int gold, int silver ) { }
    virtual void entry(        ) { }
    virtual void greet( Character *victim ) { }
    virtual void give( Character *victim, Object *obj ) { }
    virtual void fight( Character *victim, string command = "" ) { }
    virtual bool assist( Character *, Character * ) { return false; }
    virtual bool death( Character *killer ) { return false; }
    virtual bool kill( Character * ) { return false; }
    virtual bool area( ) { return false; }
    virtual bool hourly( ) { return false; }
    virtual void speech( Character *victim, const char *speech ) { }
    virtual void tell ( Character *victim, const char *speech ) { }
    virtual void show( Character *victim, std::basic_ostringstream<char> &buf ) { }
    virtual bool spell( Character *caster, int sn, bool before ) { return false; }
    virtual void cast( ::Pointer<SpellTarget>, int sn, bool before ) { }
    virtual void stopfol( Character *master ) { }
    virtual bool extract( bool );
    virtual bool look_inv( Character *looker ) { return false; }
    virtual bool social( Character *, Character *, const DLString & ) { return false; }
    virtual bool aggress( ) { return false; }
    virtual bool track( ) { return false; }
    virtual void shot( Character *, int ) { }
    virtual void flee( ) { }
    virtual void save( ) { }
    virtual void load( DLString ) { }
    virtual bool command( Character *, const DLString &, const DLString & );

    virtual bool isSaved( ) const { return true; }
    virtual bool hasDestiny( ) { return true; }
    virtual bool extractNotify( Character *, bool, bool ) { return false; }
    virtual int  getOccupation( ) { return 0; }
    virtual int  getExpBonus( Character * ) { return 0; }
    virtual bool canCancel( Character * ) { return false; }
    virtual bool isAfterCharm( ) const { return false; }
    virtual long long getLastCharmTime() const { return 0; }
    virtual bool hasSpecialName() const { return false; }

protected:
    NPCharacter *ch;
};

extern template class XMLStub<MobileBehavior>;

#endif

