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

        // Stat-bearing personal rewards (girth, ring, weapon, bag) stamp the
        // item to the wearer's level and rebuild it on every wear. The numbers
        // live in Fenia (.tmp.questreward), the plumbing stays here: shape()
        // writes the item's values first, then every candidate apply location
        // is upserted from modifier(). A reward family with an empty
        // questFamily() (the base class) has no stats and equip() is a no-op.
        virtual void equip( Character * );

        // "girth" | "ring" | "weapon" | "bag" | "" (no stats). Selects the Fenia formula.
        virtual DLString questFamily( ) const;

        // Upgrade tier stored on the instance as the "questTier" object property
        // (absent -> 0 = T0). Shared with RefitQuestArticle's fee calc.
        static int questTier( ::Object * );

protected:
        // Ask .tmp.questreward.modifier(ch, obj, fam, loc, tier) for one apply
        // location. Returns true and fills 'out' on a number; returns false on
        // null (a location the family does not own) or if the Fenia module is
        // missing or errors, so callers leave an existing affect untouched
        // instead of zeroing worn gear.
        bool feniaModifier( Character *ch, const DLString &fam, int loc, int tier, int &out );

        // Call .tmp.questreward.shape(ch, obj, fam, tier): item values that
        // are not char stats (a weapon's dice and flags). A missing function is
        // not an error: families without a shape skip it.
        void feniaShape( Character *ch, const DLString &fam, int tier );
};

#endif
