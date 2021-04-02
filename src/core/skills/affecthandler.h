/* $Id: affecthandler.h,v 1.1.2.3.18.5 2010-08-24 20:12:33 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __AFFECTHANDLER_H__
#define __AFFECTHANDLER_H__

#include <sstream>

#include "xmlpolymorphvariable.h"
#include "skillaction.h"
#include "wrappertarget.h"
#include "spelltarget.h"

class Affect;
class Character;
class Object;
class Room;

class AffectHandler : public virtual SkillAction, public WrapperTarget, public virtual XMLPolymorphVariable 
{
public:
    typedef ::Pointer<AffectHandler> Pointer;
    
    virtual ~AffectHandler( );

    virtual long long getID() const = 0;

    // List of triggers called from various places in the code.

    bool onFight(const SpellTarget::Pointer &target, Affect *paf, Character *victim);
    bool onImmune(const SpellTarget::Pointer &target, Affect *paf, Character *attacker, int &dam, const char *damType, Object *wield, int damFlag, const char *skillName);
    bool onHit(const SpellTarget::Pointer &target, Affect *paf, Character *attacker, int dam, const char *damType, Object *wield);
    bool onRemove(const SpellTarget::Pointer &target, Affect *paf);
    bool onGet(const SpellTarget::Pointer &target, Affect *paf, Character *actor);
    bool onSpec(const SpellTarget::Pointer &target, Affect *paf);
    bool onPourOut(const SpellTarget::Pointer &target, Affect *paf, Character *actor, Object *out, const char *liqname, int amount);
    bool onUpdate(const SpellTarget::Pointer &target, Affect *paf);
    bool onEntry(const SpellTarget::Pointer &target, Affect *paf, Character *walker = 0);
    bool onLeave(const SpellTarget::Pointer &target, Affect *paf, Character *walker);
    bool onDispel(const SpellTarget::Pointer &target, Affect *paf);
    bool onLook(const SpellTarget::Pointer &target, Affect *paf, Character *victim);
    bool onSmell(const SpellTarget::Pointer &target, Affect *paf, Character *victim);
    bool onSaves(const SpellTarget::Pointer &target, Affect *paf, Character *victim, int &saves, int dam_type);
    bool onStopfol(const SpellTarget::Pointer &target, Affect *paf);
    bool onDescr(const SpellTarget::Pointer &target, Affect *paf, ostringstream &buf);
    
    // Overrides for various types of triggers. Have smaller priority than Fenia overrides.

    virtual void remove( Character * ); 
    virtual void remove( Object * ); 
    virtual void remove( Room * ); 
    virtual void update( Character *, Affect * ); 
    virtual void update( Object *, Affect * ); 
    virtual void update( Room *, Affect * ); 
    virtual void entry( Character *, Affect * );
    virtual void entry( Room *, Character *, Affect * );
    virtual void leave( Room *, Character *, Affect * );
    virtual void dispel( Character * );
    virtual void look( Character *, Character *, Affect * );
    virtual bool smell( Character *, Character *, Affect * );
    virtual void toStream( ostringstream &, Affect * );
    virtual void saves( Character *, Character *, int &, int, Affect * );
    virtual void stopfol( Character *, Affect * );

    virtual bool isDispelled( ) const;
    virtual bool isCancelled( ) const;
};

#endif
