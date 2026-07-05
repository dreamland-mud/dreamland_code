/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DEFAULTWEARLOCATION_H__ 
#define __DEFAULTWEARLOCATION_H__ 

#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlmultistring.h"
#include "xmlboolean.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmlvariablecontainer.h"
#include "xmltableelement.h"

#include "wearlocation.h"


class DefaultWearlocation : public Wearlocation, 
                            public XMLTableElement,
                            public XMLVariableContainer {
XML_OBJECT
public:        
    typedef ::Pointer<DefaultWearlocation> Pointer;
    
    DefaultWearlocation( );

    virtual const DLString & getName( ) const;
    virtual void setName( const DLString & );
    virtual bool isValid( ) const;
    virtual bool givesAffects() const;
    virtual void loaded( );
    virtual void unloaded( );
    
    inline virtual const DLString & getRibName( ) const;
    inline virtual const DLString & getPurpose( lang_t lang = LANG_DEFAULT ) const;
    inline virtual int getDestroyChance( ) const;
    inline virtual int getOrderWear( ) const;
    inline virtual int getOrderDisplay( ) const;
    virtual Object * find( Character *ch );
    virtual bool matches( Character *ch );
    virtual bool matches( Object *obj );
    
    virtual void reset( Object * );
    virtual bool equip( Object *obj );
    virtual void unequip( Object *obj );
    virtual bool remove( Object *, int flags );
    virtual bool remove( Character *ch, int flags );
    virtual  int wear( Object *obj, int flags );
    virtual bool wearAtomic( Character *ch, Object *obj, int flags );

    virtual void display( Character *, DisplayList &, lang_t lang );
    virtual DLString displayLocation(Character *ch, Object *obj, lang_t lang);

    virtual int canWear( Character *ch, Object *obj, int flags );
    virtual bool canWear( Character *ch, int flags );
    virtual bool canRemove( Character *ch, Object *obj, int flags );
    virtual bool canRemove( Character *ch, int flags );

    virtual void affectsOnEquip( Character *ch, Object *obj );
    virtual void affectsOnUnequip( Character *ch, Object *obj );

protected:
    bool canEquip( Character *ch, Object *obj );
    void triggersOnEquip( Character *ch, Object *obj );
    virtual void triggersOnWear( Character *ch, Object *obj );
    void triggersOnUnequip( Character *ch, Object *obj );
    
    void saveDrops( Character *ch );

    virtual const DLString &getMsgSelfWear(Character *ch, Object *obj) const;
    virtual const DLString &getMsgSelfRemove(Object *obj) const;
    virtual const DLString &getMsgRoomWear(Object *obj) const;
    virtual const DLString &getMsgRoomRemove(Object *obj) const;

    XML_VARIABLE XMLEnumerationNoEmpty itemType;
    XML_VARIABLE XMLFlagsNoEmpty       itemWear;
    XML_VARIABLE XMLWearlocationReference conflict;
    XML_VARIABLE XMLWearlocationReference pair;
    // Ukrainization (card 2567): per-player self-messages are language-aware
    // (getForLang resolves the wearer's lang, bare RU auto-loads as l="ru").
    // The msgRoom* broadcasts stay single-string RU -- room echoes reach a
    // mixed-language audience, localized separately (or not at all).
    XML_VARIABLE XMLMultiString        msgSelfWear;
    XML_VARIABLE XMLStringNoEmpty      msgRoomWear;
    XML_VARIABLE XMLStringNoEmpty      msgSelfRemove, msgRoomRemove;
    XML_VARIABLE XMLMultiString        msgSelfNoRib;
    XML_VARIABLE XMLStringNoEmpty      msgRoomNoRib;
    XML_VARIABLE XMLStringNoEmpty      msgSelfConflict;
    XML_VARIABLE XMLIntegerNoEmpty     waitstateRemove;
    XML_VARIABLE XMLIntegerNoEmpty     armorCoef;
    XML_VARIABLE XMLBoolean            needRib;
    XML_VARIABLE XMLIntegerNoEmpty     orderWear, orderDisplay;
    XML_VARIABLE XMLBoolean            displayAlways;
    XML_VARIABLE XMLMultiString        msgDisplay;
    XML_VARIABLE XMLStringNoEmpty      ribName;
    XML_VARIABLE XMLMultiString        purpose;
    XML_VARIABLE XMLIntegerNoEmpty     destroyChance;
};

inline const DLString &DefaultWearlocation::getRibName( ) const
{
    return ribName.getValue( );
}
inline const DLString &DefaultWearlocation::getPurpose( lang_t lang ) const
{
    return purpose.getForLang( lang );
}
inline int DefaultWearlocation::getOrderWear( ) const
{
    return orderWear.getValue( );
}
inline int DefaultWearlocation::getOrderDisplay( ) const
{
    return orderDisplay.getValue( );
}
int DefaultWearlocation::getDestroyChance( ) const
{
    return destroyChance.getValue( );
}
#endif
