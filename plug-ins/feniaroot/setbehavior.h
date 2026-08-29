/*
 * Item-set behavior: perma-affects engine (#2758), phase 3.
 *
 * Moves the completion bonus of an item set out of 18 hand-written Fenia
 * onEquip bodies and into data. A SetBehavior carries the granted affects
 * (<affects>) and the assembled-message (<msgComplete>); one generic Fenia
 * pair reads them through the behavior wrapper and applies/strips the
 * materialized "set <X>" affect exactly as before. Piece-counting props
 * (total_count/double_neck/double_wrist/max_level) stay on the base behavior.
 *
 * Kept self-contained in feniaroot -- SetAffect mirrors the phase-2 XMLAffect
 * shape rather than reusing it, so the set engine and the gear advisor (both
 * feniaroot) read it without a new feniaroot->areas link (which would reorder
 * plugin unload).
 */
#ifndef SETBEHAVIOR_H
#define SETBEHAVIOR_H

#include "behaviorloader.h"
#include "xmlflags.h"
#include "xmlinteger.h"
#include "xmlglobalbitvector.h"
#include "xmlmultistring.h"
#include "xmllist.h"
#include "skillreference.h"
#include "bitstring.h"

class Affect;

/** Local mirror of areas' XMLApply. Reads/writes <apply to="str">N</apply>. */
struct SetApply : public XMLIntegerNoEmpty {
    SetApply( );

    bool toXML( XMLNode::Pointer & ) const;
    void fromXML( const XMLNode::Pointer & );

    bitstring_t location;
};

/** One affect a completed set grants: same <affect> shape as the phase-2
 *  XMLAffect (bits + apply + global), minus the item <grant> field -- sets
 *  apply stat/flag affects, they do not grant skills. */
class SetAffect : public XMLVariableContainer {
XML_OBJECT
public:
    // Populate a runtime Affect's location/modifier/bits/global. Leaves
    // type/level/duration to the caller (Fenia sets type="set <X>", -2, level).
    void fill( Affect & ) const;

    XML_VARIABLE XMLFlagsWithTable bits;
    XML_VARIABLE SetApply apply;
    XML_VARIABLE XMLGlobalBitvector global;
};

/** A behavior attached to the pieces of an item set, carrying the completion
 *  bonus as data. Dispatched from XML by type="SetBehavior". */
class SetBehavior : public DefaultBehavior {
XML_OBJECT
public:
    typedef ::Pointer<SetBehavior> Pointer;

    // Completion message in the viewer's language (falls back RU->EN).
    const DLString & getMsgComplete( lang_t lang = LANG_DEFAULT ) const;

    XML_VARIABLE XMLListBase<SetAffect> affects;
    XML_VARIABLE XMLMultiString msgComplete;
    // Usable skills the completed set grants its wearer (temporary, re-granted
    // each login since temp skills clear then). The generic eqset engine reads
    // these via .setSkills and give/removeTemporary. Multi-word skill names
    // ("camouflage move") are why this is a node list, not a name bitvector.
    XML_VARIABLE XMLListBase<XMLSkillReference> grantSkills;
};

#endif
