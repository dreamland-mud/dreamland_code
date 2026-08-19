/* $Id: personalquestreward.h,v 1.1.2.6.22.1 2007/09/11 00:31:25 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef PERSONALQUESTREWARD_H
#define PERSONALQUESTREWARD_H

#include "objectbehavior.h"
#include "objectbehaviorplugin.h"
#include "questreward.h"

class DLString;

class PersonalQuestReward : public QuestReward {
XML_OBJECT
public:
        typedef ::Pointer<PersonalQuestReward> Pointer;

        virtual void get( Character * );

        // Stat-bearing personal rewards (girth, ring, weapon) stamp the item to
        // the wearer's level and hang level-scaled affects on every wear. The
        // numbers live in Fenia (.tmp.questreward), the plumbing stays here.
        // Base equip() drives the shared girth/ring path; QuestWeapon overrides
        // it for its conditional affects and weapon generator. A reward family
        // with an empty questFamily() (base, QuestBag) has no stats and equip()
        // is a no-op.
        virtual void equip( Character * );

        // "girth" | "ring" | "weapon" | "" (no stats). Selects the Fenia formula.
        virtual DLString questFamily( ) const;

        // Upgrade tier stored on the instance as the "questTier" object property
        // (absent -> 0 = T0). Shared with RefitQuestArticle's fee calc.
        static int questTier( ::Object * );

        // Ask .tmp.questreward.modifier(ch, fam, loc, tier) for one apply
        // location. Returns true and fills 'out' on success; returns false if
        // the Fenia module is missing or errors, so callers can leave an
        // existing affect untouched instead of zeroing worn gear.
        static bool feniaModifier( Character *ch, const DLString &fam, int loc, int tier, int &out );
};

#endif
