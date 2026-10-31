/* $Id$
 *
 * ruffina, 2004
 */
#include "personalnamerepair.h"
#include "heroitems.h"
#include "itemevents.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "object.h"
#include "lang.h"
#include "string_utils.h"
#include "merc.h"
#include "def.h"
#include "logstream.h"
#include "act.h"

/*----------------------------------------------------------------------------
 * PersonalNameRepair
 *---------------------------------------------------------------------------*/
void PersonalNameRepair::initialization( )
{
    eventBus->subscribe( typeid(ItemReadEvent), Pointer(this) );
}

void PersonalNameRepair::destruction( )
{
    eventBus->unsubscribe( typeid(ItemReadEvent), Pointer(this) );
}

void PersonalNameRepair::handleEvent( const type_index &eventType, const Event &event ) const
{
    if (eventType == typeid(ItemReadEvent))
        eventItemRead( static_cast<const ItemReadEvent &>(event) );
}

/** How many substitutions the prototype asks for in this language. */
static int personal_arg_count( const DLString &pattern )
{
    int count = 0;

    for (DLString::size_type pos = pattern.find("%s");
         pos != DLString::npos;
         pos = pattern.find("%s", pos + 2))
        count++;

    return count;
}

/** True when every format spec in the pattern is a plain "%s". Counting "%s"
 *  alone would let a prototype that grew a "%d" through as long as the "%s"
 *  count still matched, and handing printf a string where it wants an int is
 *  undefined. The money objects already carry "%d", so this is not imaginary. */
static bool personal_args_all_strings( const DLString &pattern )
{
    for (DLString::size_type pos = pattern.find('%');
         pos != DLString::npos;
         pos = pattern.find('%', pos + 2))
        if (pos + 1 >= pattern.length( ) || pattern.at(pos + 1) != 's')
            return false;

    return true;
}

/** True when this language slot still needs filling: the instance has nothing
 *  of its own and the prototype behind it is a template we know how to fill. */
static bool personal_slot_needs_repair( Object *obj, lang_t lang )
{
    if (!obj->getRealShortDescr(lang).empty())
        return false;

    const DLString &pattern = obj->pIndexData->short_descr.get(lang);

    return personal_args_all_strings( pattern ) && personal_arg_count( pattern ) > 0;
}

/** Recover which adjective was engraved on a hero item. The buyer's alignment at
 *  purchase time is recorded nowhere else, so read it back off the Russian name.
 *  Returns 0 when the name starts with none of them, which is the signal to
 *  leave the object alone rather than guess. */
static const LangText * personal_engraved_adjective( const DLString &shortRu )
{
    for (int a = 0; a < 3; a++)
        for (int g = 0; g < 3; g++) {
            DLString adjective = HERO_ADJECTIVES[a][g].ru;

            if (adjective.strPrefix( shortRu ))
                return &HERO_ADJECTIVES[a][g];
        }

    return 0;
}

/** The prototypes this repair may touch, and nothing else.
 *
 *  A "%s" in a short descr is not a promise that the missing word is the owner:
 *  across the 34 prototypes in the world that have one it can be a material
 *  (battle poncho), a liquid (puddle), a deity (tattoo), the mob a steak was cut
 *  from, or the victim pictured on an assassin card. An object carrying an owner
 *  field does not settle it either -- the dungeon storage box has both an owner
 *  and a "%s", and they mean the same thing there only by coincidence. So this
 *  is a list of the objects whose template really is "adjective, owner" or
 *  "owner", not a rule inferred from the data.
 *
 *  Hero quest items take two substitutions, adjective and owner. */
static bool personal_repairable( int vnum )
{
    switch (vnum) {
    case 94: case 95: case 96: case 103:                    // hero girth, ring, weapon, bag
        return true;
    }

    return false;
}

void PersonalNameRepair::eventItemRead( const ItemReadEvent &event ) const
{
    Object *obj = event.obj;

    if (!personal_repairable( obj->pIndexData->vnum ))
        return;

    if (obj->getOwner( ).empty( ))
        return;

    bool anyGap = false;
    for (int l = LANG_MIN; l < LANG_MAX && !anyGap; l++)
        anyGap = personal_slot_needs_repair( obj, (lang_t)l );

    if (!anyGap)
        return;

    // The owner may well be offline; a memory record declines per language just
    // as a live character does. A record can be genuinely gone (deleted player),
    // and then there is no name to write.
    PCMemoryInterface *owner = PCharacterManager::find( obj->getOwner( ) );
    if (!owner)
        return;

    // Two substitutions mean adjective first and owner second; one means the
    // owner alone.
    const LangText *adjective = 0;

    if (personal_arg_count( obj->pIndexData->short_descr.get(LANG_DEFAULT) ) > 1) {
        adjective = personal_engraved_adjective( obj->getRealShortDescr(LANG_DEFAULT) );

        if (!adjective) {
            // No adjective means the item was renamed and its Russian slot holds
            // custom text, not an engraving. Dozens of these exist, all restrung
            // before XMLItemRestring::dress learned to write every language
            // (2026-07-24). There is nothing to translate in a custom name, so
            // mirror Russian into the empty slots, exactly as a restring does
            // today. Leaving them empty is not neutral:
            // display falls through to the PROTOTYPE, and the prototype is a
            // template, so the reader gets a raw "%s".
            const DLString &restrung = obj->getRealShortDescr(LANG_DEFAULT);

            if (!restrung.empty( ))
                for (int l = LANG_MIN; l < LANG_MAX; l++)
                    if (obj->getRealShortDescr((lang_t)l).empty( ))
                        obj->setShortDescr( restrung, (lang_t)l );

            return;
        }
    }

    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        if (!personal_slot_needs_repair( obj, lang ))
            continue;

        const DLString &pattern = obj->pIndexData->short_descr.get(lang);
        DLString name = owner->getNameP( '2', lang );

        // Only fill a slot whose template asks for exactly what we have. Together
        // with the all-strings check above, a prototype edited later can neither
        // take the wrong number of arguments nor the wrong kind.
        int args = personal_arg_count( pattern );

        if (adjective && args == 2)
            obj->setShortDescr( fmt(0, pattern.c_str(), adjective->get(lang), name.c_str()), lang );
        else if (!adjective && args == 1)
            obj->setShortDescr( fmt(0, pattern.c_str(), name.c_str()), lang );
    }
}
