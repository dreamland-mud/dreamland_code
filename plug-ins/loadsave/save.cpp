/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/


#ifdef HAVE_CONFIG_H
        #include "config.h"
#endif

#include <algorithm>

#include <sys/types.h>
#include <ctype.h>
#include <cstdio>
#include <cstring>
#include <time.h>


#include "fileformatexception.h"
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "skill.h"
#include "skillgroup.h"
#include "skillreference.h"

#include "mobilebehavior.h"
#include "mobilebehaviormanager.h"
#include "objectbehavior.h"
#include "objectbehaviormanager.h"

#include "feniamanager.h"

#include "json_utils_ext.h"
#include "pcharactermanager.h"
#include "pcharactermemory.h"
#include "affectmanager.h"
#include "objectmanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "mobtiers.h"
#include "object.h"
#include "affect.h"
#include "room.h"
#include "desire.h"
#include "liquid.h"
#include "wearlocation.h"

#include "dreamland.h"
#include "merc.h"

#include "act.h"
#include "save.h"
#include "loadsave.h"
#include "fread_utils.h"
#include "compatflags.h"
#include "itemevents.h"
#include "vnum.h"
#include "def.h"
#include "l10n.h"

GSN(none);
GSN(doppelganger);
GSN(dispel_affects);
GSN(dispel_magic);
GSN(spell_resistance);
GSN(magic_resistance);
GSN(enchant_weapon);
GSN(enchant_armor);
GSN(katana);
DESIRE(drunk);
DESIRE(hunger);
DESIRE(thirst);
DESIRE(bloodlust);
WEARLOC(none);
CLAN(none);
void password_set( PCMemoryInterface *pci, const DLString &plainText );
const FlagTable * affect_where_to_table(int where);
int affect_table_to_where(const FlagTable *table, const GlobalRegistryBase *registry);
bool limit_is_counted( time_t ts );

// fread_obj bumps the proto count at Vnum for drop/charmed/saved-mob files, before
// TS is read. Take it back for a limited item that expired before boot: that is
// the rule limit_count_on_boot applies to player profiles, and extract_obj_1 will
// not decrement such an item either. Call on every exit that keeps or extracts obj.
static void fread_obj_uncount_expired( Object *obj, Room *room )
{
    if (!create_obj_dropped || obj->pIndexData == 0)
        return;
    if (obj->pIndexData->limit < 0 || limit_is_counted( obj->timestamp ))
        return;

    obj->pIndexData->count--;
    LogStream::sendNotice( ) << "Limited item " << obj->pIndexData->vnum
        << " (" << obj->getID( ) << ") expired in saved file, room #"
        << (room ? room->vnum : -1) << ", not counted" << endl;
}

static DLString id_to_string(long long id)
{
    ostringstream os;
    os << id;
    return os.str();
}

static void convert_skill( int &sn )
{
    if (sn == gsn_dispel_magic)
        sn = gsn_dispel_affects;
    else if (sn == gsn_magic_resistance)
        sn = gsn_spell_resistance;
}

static const DLString &skillname(int index)
{
    Skill *skill = SkillManager::getThis( )->find(index);
    return skill ? skill->getName() : gsn_none->getName();
}

static void convert_obj_values( Object *obj )
{
    static const char * liquid_names [] = {
    "water", "beer", "red wine", "ale", "dark ale", "whisky", "lemonade",
    "firebreather", "local specialty", "slime mold juice",
    "milk", "tea", "coffee", "blood", "salt water", "coke", "root beer",
    "elvish wine", "white wine", "champagne", "mead", "rose wine",
    "benedictine wine", "vodka", "cranberry juice", "orange juice",
    "absinthe", "brandy", "aquavit", "schnapps", "icewine", "amontillado",
    "sherry", "framboise", "rum", "cordial", "valerian tincture",
    "chocolate", "grape juice", 
    };
    static const int liquid_count = 39;

    switch (obj->item_type) {
    case ITEM_FOUNTAIN:
    case ITEM_DRINK_CON:
        if (obj->value2() >= 0 && obj->value2() < liquid_count) 
            obj->value2(liquidManager->lookup( liquid_names[obj->value2()] ));

        break;

    default:
        break;
    }
}

static void convert_religion( PCharacter *pc, int number )
{
    static const char *relig_names [] =  { 
     "none",       "atum-ra",    "zeus",       "siebele",    "shamash",    
     "ahuramazda", "ehrumen",    "deimos",     "phobos",     "odin",       
     "teshub",     "ares",       "goktengri",  "hera",       "venus",      
     "seth",       "enki",       "eros", 
    };
    
    pc->setReligion( religionManager->find( relig_names[number] )->getName( ) );
}

Affect * fread_affect( FILE *fp, bool withSources = false )
{
    Affect *paf;
    int sn;
    char *word = fread_word( fp );
    DLString globalString;
    int where;

    sn = SkillManager::getThis( )->lookup( word );
    convert_skill( sn );

    paf = AffectManager::getThis()->getAffect();

    try {
        paf->type = sn;
        where        = fread_number(fp); 
        paf->level        = fread_number(fp);
        paf->duration        = fread_number(fp);
        paf->modifier        = fread_number(fp);
        paf->location.setTable(&apply_flags);
        paf->location        = fread_number(fp);
        paf->bitvector.setTable(affect_where_to_table(where));
        paf->bitvector.setValue(fread_number(fp));

        // "Aff2" lines carry a count-prefixed block of item-source vnums here,
        // before the global string. Legacy "Affc" lines pass withSources=false and
        // skip straight to the global string.
        if (withSources) {
            int nsrc = fread_number(fp);
            for (int i = 0; i < nsrc; i++)
                paf->sources.addItem( fread_number(fp) );
        }

        globalString    = fread_dlstring_to_eol(fp);
        globalString.substitute('\r', ' ').substitute('\n', ' ');
        globalString.stripWhiteSpace( );

        if (where == TO_LOCATIONS) {
            paf->global.setRegistry( wearlocationManager );
            paf->global.fromString( globalString );
        } else if (where == TO_LIQUIDS) {
            paf->global.setRegistry( liquidManager );
            paf->global.fromString( globalString );
        } else if (where == TO_SKILLS) {
            paf->global.setRegistry( skillManager );
            paf->global.fromString( globalString );
        } else if (where == TO_SKILL_GROUPS) {
            paf->global.setRegistry( skillGroupManager );
            paf->global.fromString( globalString );
        }

    } catch (const FileFormatException &e) {
        AffectManager::getThis()->extract(paf);
        throw e;
    }

    return paf;
}

void fwrite_affect( const char *label, FILE *fp, Affect *paf )
{
    if (paf->type == gsn_doppelganger)
        return;

    // Item (object) sources must survive the pfile so a permanent item-cast affect
    // can still be matched -- and taken back off -- by the unequip backstop after a
    // relog or reboot. Only char/pet affects ("Affc") carry them; char/room sources
    // are intentionally not persisted (unchanged behaviour). When present, write the
    // "Aff2" variant: identical fields plus a count-prefixed vnum block placed BEFORE
    // the to-end-of-line global string, so the global string stays last and legacy
    // "Affc" lines keep reading exactly as before.
    std::list<int> itemVnums;
    if (!strcmp(label, "Affc"))
        itemVnums = paf->sources.objectVnums();

    if (itemVnums.empty()) {
        fprintf( fp, "%s '%s' %3d %3d %3d %3d %3d %10lld %s\n",
                label,
                paf->type->getName( ).c_str( ),
                affect_table_to_where(paf->bitvector.getTable(), paf->global.getRegistry()),
                paf->level.getValue(),
                paf->duration.getValue(),
                paf->modifier.getValue(),
                paf->location.getValue(),
                paf->bitvector.getValue(),
                paf->global.toString( ).c_str( ));
        return;
    }

    fprintf( fp, "Aff2 '%s' %3d %3d %3d %3d %3d %10lld %d",
            paf->type->getName( ).c_str( ),
            affect_table_to_where(paf->bitvector.getTable(), paf->global.getRegistry()),
            paf->level.getValue(),
            paf->duration.getValue(),
            paf->modifier.getValue(),
            paf->location.getValue(),
            paf->bitvector.getValue(),
            (int)itemVnums.size());
    for (int vnum: itemVnums)
        fprintf( fp, " %d", vnum );
    fprintf( fp, " %s\n", paf->global.toString( ).c_str( ));
}

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
Object *        rgObjNest        [MAX_NEST];

int obj_econ_rev = 0;
ObjEconMigrateFn obj_econ_migrate_fn = 0;




/*
 * Write the char.
 */
void fwrite_char( PCharacter *ch, FILE *fp )
{

        fprintf( fp, "#%s\n",  "PLAYER"        );
        for (auto &paf: ch->affected)
            fwrite_affect( "Affc", fp, paf );

        fprintf( fp, "End\n\n" );
}

// Write out non-empty values for multi-language string.
static void fwrite_multistring(FILE *fp, const DLString &label, const XMLMultiString &field)
{
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        const DLString &fieldValue = field.get(lang);
        DLString langName = lang2attr(lang);

        if (!fieldValue.empty()) {
            fprintf(fp, "%s %s %s~\n", 
                    label.c_str(),
                    langName.c_str(),
                    fieldValue.c_str());
        }
    }
}

// Write out key/value pairs for each language.
static void fwrite_multistring(FILE *fp, const DLString &label, const DLString &key, const XMLMultiString &valueField)
{
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        const DLString &value = valueField.get(lang);
        DLString langName = lang2attr(lang);

        fprintf(fp, "%s %s %s~ %s~\n", 
                label.c_str(),
                langName.c_str(),
                key.c_str(),
                value.c_str());
    }
}

/*
 * Mob reform: saved mobiles as diffs against what create_mobile_org produces
 * from today's prototype, plus a stamp (plan §3.6 item 10,
 * docs/plans/mob-reform-fwrite-diff.md variant B, decisions 20 and 36).
 */
static const char * const saved_set_keys[MOBSET_MAX] = {
    "Act", "Off", "AfBy", "Detect", "Imm", "Res", "Vuln", "Form", "Part"
};


static bitstring_t npc_set_get(NPCharacter *mob, int set)
{
    switch (set) {
    case MOBSET_ACT:  return mob->act.getValue();
    case MOBSET_OFF:  return (unsigned int)mob->off_flags;
    case MOBSET_AFF:  return mob->affected_by.getValue();
    case MOBSET_DET:  return mob->detection.getValue();
    case MOBSET_IMM:  return mob->imm_flags.getValue();
    case MOBSET_RES:  return mob->res_flags.getValue();
    case MOBSET_VULN: return mob->vuln_flags.getValue();
    case MOBSET_FORM: return mob->form.getValue();
    default:          return mob->parts.getValue();
    }
}

static void npc_set_put(NPCharacter *mob, int set, bitstring_t v)
{
    switch (set) {
    case MOBSET_ACT:  mob->act.setValue(v); break;
    case MOBSET_OFF:  mob->off_flags = (int)v; break;
    case MOBSET_AFF:  mob->affected_by.setValue(v); break;
    case MOBSET_DET:  mob->detection.setValue(v); break;
    case MOBSET_IMM:  mob->imm_flags.setValue(v); break;
    case MOBSET_RES:  mob->res_flags.setValue(v); break;
    case MOBSET_VULN: mob->vuln_flags.setValue(v); break;
    case MOBSET_FORM: mob->form.setValue(v); break;
    default:          mob->parts.setValue(v); break;
    }
}

/* What create_mobile_org builds for one set from the current prototype. */
static bitstring_t produced_set(MOB_INDEX_DATA *pIndex, int set)
{
    bitstring_t v = (unsigned int)pIndex->bodyBits(set);
    if (set == MOBSET_ACT)
        v |= ACT_IS_NPC;
    if (set == MOBSET_AFF)
        v &= ~AFF_FROM_AFFECTS;
    return v;
}

/* One description slot is written only when it is the instance's own text:
 * set, and neither the prototype's nor what create_mobile_org derives from it. */
static void fwrite_multistring_diff(FILE *fp, const char *label, NPCharacter *mob,
                                    const XMLMultiString &field, const XMLMultiString &proto)
{
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        const DLString &value = field.get(lang);
        if (value.empty())
            continue;

        const DLString &protoValue = proto.get(lang);
        if (value == protoValue)
            continue;
        if (protoValue.find("%1") != DLString::npos && value == fmt(0, protoValue.c_str(), mob))
            continue;

        fprintf(fp, "%s %s %s~\n", label, lang2attr(lang).c_str(), value.c_str());
    }
}

/* Body diffs, numbers and affects read from a #MOBILE/#PET block, applied at End. */
struct SavedMobState {
    bool hasStamp = false;
    unsigned long long stamp = 0;
    bitstring_t add[MOBSET_MAX] = { 0 }, del[MOBSET_MAX] = { 0 };
    DLString wearAdd, wearDel;
    int size = -1;
    int legacyBody = 0;          // legacy Part/Form/... lines seen (ignored)

    // pets only: Act/AfBy/Detect/Wearloc are still written as whole values
    bool petAct = false, petAff = false, petDet = false, petWear = false;
    bitstring_t petActV = 0, petAffV = 0, petDetV = 0;
    DLString petWearV;

    bool hmv = false;
    int hmvV[6] = { 0 };
    bool acs = false;
    int acsV[4] = { 0 };
    bool attr = false, amod = false;
    vector<int> attrV, amodV;
    bool hasHit = false, hasDam = false, hasDamN = false, hasDamT = false, hasSave = false, hasLevel = false;
    int hitV = 0, damV = 0, damNV = 0, damTV = 0, saveV = 0, levelV = 0;
    int tierV = 0;              // instance tier from apply_mob_tier, 0 = prototype's

    list<Affect *> affects;

    bool stampMatches(MOB_INDEX_DATA *pIndex) const
    {
        return hasStamp && stamp == pIndex->bodyStamp();
    }
};

/* Consume one saved key that belongs to the body, the numbers or the affects. */
static bool fread_saved_mob_key(const char *word, FILE *fp, SavedMobState &st, bool pet)
{
    if (!strcmp(word, "BodyVer")) {
        st.hasStamp = true;
        st.stamp = (unsigned long long)fread_number64(fp);
        return true;
    }

    static DLString addKeys[MOBSET_MAX], delKeys[MOBSET_MAX];
    if (addKeys[0].empty())
        for (int s = 0; s < MOBSET_MAX; s++) {
            addKeys[s] = DLString(saved_set_keys[s]) + "Add";
            delKeys[s] = DLString(saved_set_keys[s]) + "Del";
        }

    for (int s = 0; s < MOBSET_MAX; s++) {
        if (addKeys[s] == word) {
            st.add[s] = (unsigned long)fread_flag(fp);
            return true;
        }
        if (delKeys[s] == word) {
            st.del[s] = (unsigned long)fread_flag(fp);
            return true;
        }
    }

    if (!strcmp(word, "WearlocAdd")) {
        st.wearAdd = fread_dlstring(fp);
        return true;
    }
    if (!strcmp(word, "WearlocDel")) {
        st.wearDel = fread_dlstring(fp);
        return true;
    }

    if (pet) {
        if (!strcmp(word, "Act")) { st.petAct = true; st.petActV = (unsigned long)fread_flag(fp); return true; }
        if (!strcmp(word, "AfBy")) { st.petAff = true; st.petAffV = (unsigned long)fread_flag(fp); return true; }
        if (!strcmp(word, "Detect")) { st.petDet = true; st.petDetV = (unsigned long)fread_flag(fp); return true; }
        if (!strcmp(word, "Wearloc")) { st.petWear = true; st.petWearV = fread_dlstring(fp); return true; }
    } else {
        // Whole-value body lines of blocks written before the reform: a body
        // frozen at save time is exactly what the reform stops, so they are
        // read past and the body comes from the prototype (fwrite diff §5).
        static const char * const legacy[] = { "Act", "AfBy", "Detect", "Part", "Form", "Imm", "Res", "Vuln" };
        for (const char *l: legacy)
            if (!strcmp(word, l)) {
                fread_flag(fp);
                st.legacyBody++;
                return true;
            }
        if (!strcmp(word, "Wearloc")) {
            fread_dlstring(fp);
            st.legacyBody++;
            return true;
        }
        if (!strcmp(word, "Size")) {
            st.size = fread_number(fp);
            return true;
        }
    }

    if (!strcmp(word, "HMV")) {
        st.hmv = true;
        for (int i = 0; i < 6; i++)
            st.hmvV[i] = fread_number(fp);
        return true;
    }
    if (!strcmp(word, "ACs")) {
        st.acs = true;
        for (int i = 0; i < 4; i++)
            st.acsV[i] = fread_number(fp);
        return true;
    }
    if (!strcmp(word, "Attr") || !strcmp(word, "AMod")) {
        bool isAttr = !strcmp(word, "Attr");
        vector<int> &v = isAttr ? st.attrV : st.amodV;
        v.clear();
        for (int i = 0; i < stat_table.size; i++)
            v.push_back(fread_number(fp));
        (isAttr ? st.attr : st.amod) = true;
        return true;
    }
    if (!strcmp(word, "Hit"))  { st.hasHit = true;  st.hitV = fread_number(fp);  return true; }
    if (!strcmp(word, "Dam"))  { st.hasDam = true;  st.damV = fread_number(fp);  return true; }
    if (!strcmp(word, "DamN")) { st.hasDamN = true; st.damNV = fread_number(fp); return true; }
    if (!strcmp(word, "DamT")) { st.hasDamT = true; st.damTV = fread_number(fp); return true; }
    if (!strcmp(word, "Save")) { st.hasSave = true; st.saveV = fread_number(fp); return true; }
    if (!strcmp(word, "Levl")) { st.hasLevel = true; st.levelV = fread_number(fp); return true; }
    if (!strcmp(word, "Tier")) { st.tierV = fread_number(fp); return true; }

    if (!strcmp(word, "Affc") || !strcmp(word, "Aff2")) {
        st.affects.push_back(fread_affect(fp, !strcmp(word, "Aff2")));
        return true;
    }

    return false;
}

/* Overlay the saved numbers: the individual roll, and any growth, survives
 * only while the stamp matches (decision 36). */
static void apply_saved_numbers(NPCharacter *mob, const SavedMobState &st)
{
    if (st.hasLevel)
        mob->setLevel(st.levelV);
    if (st.hmv) {
        mob->hit = st.hmvV[0];
        mob->max_hit = st.hmvV[1];
        mob->mana = st.hmvV[2];
        mob->max_mana = st.hmvV[3];
        mob->move = st.hmvV[4];
        mob->max_move = st.hmvV[5];
    }
    if (st.acs)
        for (int i = 0; i < 4; i++)
            mob->armor[i] = st.acsV[i];
    if (st.attr)
        for (int i = 0; i < stat_table.size && i < (int)st.attrV.size(); i++)
            mob->perm_stat[i] = st.attrV[i];
    if (st.amod)
        for (int i = 0; i < stat_table.size && i < (int)st.amodV.size(); i++)
            mob->mod_stat[i] = st.amodV[i];
    if (st.hasHit)
        mob->hitroll = st.hitV;
    if (st.hasDam)
        mob->damroll = st.damV;
    if (st.hasDamN)
        mob->damage[DICE_NUMBER] = st.damNV;
    if (st.hasDamT)
        mob->damage[DICE_TYPE] = st.damTV;
    if (st.hasSave)
        mob->saving_throw = st.saveV;
    if (st.tierV >= MobTiers::TIER_BEST && st.tierV <= MobTiers::TIER_WORST)
        mob->tier = st.tierV;
}

/* Boot forensics: legacy whole-value body lines read past, reported once. */
static int saved_legacy_body_lines = 0;
static int saved_legacy_blocks = 0;

void saved_mobiles_report( )
{
    if (saved_legacy_body_lines > 0)
        LogStream::sendNotice( ) << "Saved mobiles: " << saved_legacy_body_lines
            << " pre-reform body line(s) in " << saved_legacy_blocks
            << " block(s) ignored, bodies re-derived from the prototypes." << endl;
    saved_legacy_body_lines = saved_legacy_blocks = 0;
}

/* fread_mob End: body diffs and numbers if the stamp matches, then the affects. */
static void apply_saved_mob(NPCharacter *mob, SavedMobState &st)
{
    if (st.legacyBody > 0) {
        saved_legacy_body_lines += st.legacyBody;
        saved_legacy_blocks++;
    }

    if (st.stampMatches(mob->pIndexData)) {
        for (int s = 0; s < MOBSET_MAX; s++)
            if (st.add[s] || st.del[s])
                npc_set_put(mob, s, (npc_set_get(mob, s) | st.add[s]) & ~st.del[s]);

        if (!st.wearAdd.empty()) {
            GlobalBitvector w(wearlocationManager);
            w.fromString(st.wearAdd);
            mob->wearloc.set(w);
        }
        if (!st.wearDel.empty()) {
            GlobalBitvector w(wearlocationManager);
            w.fromString(st.wearDel);
            mob->wearloc.remove(w);
        }
        if (st.size >= 0)
            mob->size = st.size;

        apply_saved_numbers(mob, st);
    }

    // The body this instance now has is what affect_check falls back to.
    for (int s = 0; s < MOBSET_MAX; s++)
        mob->baseBits[s] = npc_set_get(mob, s) & (s == MOBSET_AFF ? ~AFF_FROM_AFFECTS : ~(bitstring_t)0);

    for (auto &af: st.affects) {
        affect_to_char(mob, af);
        ddeallocate(af);
    }
    st.affects.clear();
}

/* fread_pet End. Pets write numbers and act/aff/det/wearloc whole and keep
 * their affects in those numbers (no affect_modify on load); a stamp mismatch
 * (or no stamp) re-derives from the prototype, so the affects then have to be
 * applied for real (decision 36). */
static void apply_saved_pet(NPCharacter *pet, SavedMobState &st)
{
    bool keep = st.stampMatches(pet->pIndexData);

    if (keep) {
        if (st.petAct)
            pet->act.setValue(st.petActV);
        if (st.petAff)
            pet->affected_by.setValue(st.petAffV);
        if (st.petDet)
            pet->detection.setValue(st.petDetV);
        if (st.petWear)
            pet->wearloc.fromString(st.petWearV);
        apply_saved_numbers(pet, st);
    }

    for (auto &af: st.affects) {
        if (keep)
            pet->affected.push_front(af);
        else {
            affect_to_char(pet, af);
            ddeallocate(af);
        }
    }
    st.affects.clear();
}

/* write a pet */
void fwrite_pet( NPCharacter *pet, FILE *fp)
{
        fprintf(fp,"#PET\n");

        fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);

        fprintf(fp,"Name %s~\n", pet->getNameC() );

        if (pet->in_room && pet->master && pet->master->in_room != pet->in_room)
            fprintf(fp,"Room %d\n", pet->in_room->vnum);
            
        fprintf( fp, "Id   %s\n", id_to_string(pet->getID()).c_str() );
        fprintf( fp, "BodyVer %lld\n", (long long)pet->pIndexData->bodyStamp() );
        fwrite_multistring(fp, "Keyword", pet->getRealKeyword());
        fwrite_multistring(fp, "ShortDesc", pet->getRealShortDescr());
        fwrite_multistring(fp, "LongDesc", pet->getRealLongDescr());
        fwrite_multistring(fp, "Description", pet->getRealDescription());

        if (pet->getRace( )->getName( ) != pet->pIndexData->race) 
                fprintf(fp,"Race %s~\n", pet->getRace( )->getName( ).c_str( ));

        fprintf(fp,"Sex  %d\n", pet->getSex( ) );

        if (pet->getRealLevel( ) != pet->pIndexData->level)
                fprintf(fp,"Levl %d\n", pet->getRealLevel( ) );

        if (pet->tier > 0 && pet->tier != pet->pIndexData->tier)
                fprintf(fp,"Tier %d\n", pet->tier );

        fprintf(fp, "HMV  %d %d %d %d %d %d\n",
                pet->hit.getValue( ), pet->max_hit.getValue( ), pet->mana.getValue( ), pet->max_mana.getValue( ), pet->move.getValue( ), pet->max_move.getValue( ));

        if (pet->gold > 0)
                fprintf(fp,"Gold %d\n",pet->gold.getValue( ));

        if (pet->silver > 0)
                fprintf(fp,"Silv %d\n",pet->silver.getValue( ));

        if (pet->exp > 0)
                fprintf(fp, "Exp  %d\n", pet->exp.getValue( ));

        if (pet->act != pet->pIndexData->act)
                fprintf(fp, "Act  %s\n", print_flags(pet->act));

        if (pet->affected_by != pet->pIndexData->affected_by)
                fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));

        if (pet->detection != pet->pIndexData->detection)
                fprintf(fp, "Detect %s\n", print_flags(pet->detection));

        if (pet->comm != 0)
                fprintf(fp, "Comm %s\n", print_flags(pet->comm));

        fprintf(fp,"Pos  %d\n", pet->position == POS_FIGHTING ? POS_STANDING : pet->position.getValue( ));

        if (pet->saving_throw != 0)
                fprintf(fp, "Save %d\n", pet->saving_throw.getValue( ));

        if (pet->alignment != pet->pIndexData->alignment)
                fprintf(fp, "Alig %d\n", pet->alignment.getValue( ));

        if (pet->hitroll != pet->pIndexData->hitroll)
                fprintf(fp, "Hit  %d\n", pet->hitroll.getValue( ));

        if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
                fprintf(fp, "Dam  %d\n", pet->damroll.getValue( ));

        if (pet->damage[DICE_NUMBER] != pet->pIndexData->damage[DICE_NUMBER])
                fprintf(fp, "DamN  %d\n", pet->damage[DICE_NUMBER]);

        if (pet->damage[DICE_TYPE] != pet->pIndexData->damage[DICE_TYPE])
                fprintf(fp, "DamT  %d\n", pet->damage[DICE_TYPE]);

        fprintf(fp, "ACs  %d %d %d %d\n",
                pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);

        fprintf(fp, "Attr %d %d %d %d %d %d\n",
                pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
                pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
                pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHA]);

        fprintf(fp, "AMod %d %d %d %d %d %d\n",
                pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
                pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
                pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHA]);

        for (auto &paf: pet->affected)
            fwrite_affect( "Affc", fp, paf );

        fprintf(fp,"End\n");
        return;
}


/* write a mobile */
void fwrite_mob( NPCharacter *mob, FILE *fp)
{
        if ( mob->in_room == 0 )
        {
                bug( "Write_mobile: mobile not in room! ", 0 );
                return;
        }

        if (IS_SET(mob->pIndexData->area->area_flag, AREA_NOSAVEDROP))
            return;

        if (IS_SET(mob->act, ACT_NOSAVEDROP))
            return;

        // fread_mob re-applies every saved affect through affect_to_char, so what
        // goes on disk has to be the mob WITHOUT them. Writing the live value
        // instead means each save/load cycle bakes in another copy of every
        // modifier: that is how the Anon wall guards reached saving_throw -229 and
        // became immune to every spell in the game, one point per save/load cycle.
        // Restored right after the affect block below -- nothing between the two
        // loops can return early.
        for (auto &paf: mob->affected)
            affect_modify( mob, paf, false );

        fprintf(fp,"#MOBILE\n");

        fprintf(fp,"Vnum %d\n",mob->pIndexData->vnum);

        fprintf( fp, "Id   %s\n", id_to_string(mob->getID()).c_str() );
        fprintf( fp, "BodyVer %lld\n", (long long)mob->pIndexData->bodyStamp() );
        
        fwrite_multistring_diff(fp, "Keyword", mob, mob->getRealKeyword(), mob->pIndexData->keyword);
        fwrite_multistring_diff(fp, "ShortDesc", mob, mob->getRealShortDescr(), mob->pIndexData->short_descr);
        fwrite_multistring_diff(fp, "LongDesc", mob, mob->getRealLongDescr(), mob->pIndexData->long_descr);
        fwrite_multistring_diff(fp, "Description", mob, mob->getRealDescription(), mob->pIndexData->description);

        fprintf( fp, "Room %d\n", mob->in_room->vnum );

        if ( mob->zone )
                fprintf( fp, "RZone %s~\n", mob->zone->area_file->file_name.c_str() );
        if (mob->reset_room != 0)
            fprintf(fp, "RRoom %d\n", mob->reset_room); 

        if (mob->getRace( )->getName( ) != mob->pIndexData->race) 
                fprintf(fp,"Race %s~\n", mob->getRace( )->getName( ).c_str( ));

        fprintf(fp,"Sex  %d\n", mob->getSex( ) );

        if (mob->getRealLevel( ) != mob->pIndexData->level)
                fprintf(fp,"Levl %d\n", mob->getRealLevel( ) );

        if (mob->tier > 0 && mob->tier != mob->pIndexData->tier)
                fprintf(fp,"Tier %d\n", mob->tier );

        if (mob->getClan() != clan_none)
            fprintf(fp, "Clan %s~\n", mob->getClan().getName().c_str());

        fprintf(fp, "HMV  %d %d %d %d %d %d\n",
                mob->hit.getValue( ), mob->max_hit.getValue( ), mob->mana.getValue( ), mob->max_mana.getValue( ), mob->move.getValue( ), mob->max_move.getValue( ));

        if (mob->gold > 0)
                fprintf(fp,"Gold %d\n",mob->gold.getValue( ));

        if (mob->silver > 0)
                fprintf(fp,"Silv %d\n",mob->silver.getValue( ));

        if (mob->exp > 0)
                fprintf(fp, "Exp  %d\n", mob->exp.getValue( ));

        if (mob->timer != 0)
                fprintf( fp, "Timer %d\n", mob->timer );

        // Body: diffs against what the prototype produces today, so a
        // prototype fix reaches this mob at the next boot (mob reform).
        for (int s = 0; s < MOBSET_MAX; s++) {
            bitstring_t base = npc_set_get(mob, s), produced = produced_set(mob->pIndexData, s);
            if (s == MOBSET_AFF)
                base &= ~(bitstring_t)AFF_CHARM;
            bitstring_t add = base & ~produced, del = produced & ~base;
            if (add)
                fprintf(fp, "%sAdd %s\n", saved_set_keys[s], print_flags((int)add));
            if (del)
                fprintf(fp, "%sDel %s\n", saved_set_keys[s], print_flags((int)del));
        }
        {
            GlobalBitvector wadd(mob->wearloc), wdel(mob->pIndexData->wearloc);
            wadd.remove(mob->pIndexData->wearloc);
            wdel.remove(mob->wearloc);
            if (!wadd.empty())
                fprintf(fp, "WearlocAdd %s~\n", wadd.toString().c_str());
            if (!wdel.empty())
                fprintf(fp, "WearlocDel %s~\n", wdel.toString().c_str());
        }
        if (mob->size != mob->pIndexData->getSize())
                fprintf(fp, "Size %d\n", mob->size);

        if (mob->comm != 0)
                fprintf(fp, "Comm %s\n", print_flags(mob->comm));

        fprintf(fp,"Pos  %d\n", mob->position == POS_FIGHTING ? POS_STANDING : mob->position.getValue( ));

        if (mob->saving_throw != 0)
                fprintf(fp, "Save %d\n", mob->saving_throw.getValue( ));

        if (mob->alignment != mob->pIndexData->alignment)
                fprintf(fp, "Alig %d\n", mob->alignment.getValue( ));

        if (mob->hitroll != mob->pIndexData->hitroll)
                fprintf(fp, "Hit  %d\n", mob->hitroll.getValue( ));

        if (mob->damroll != mob->pIndexData->damage[DICE_BONUS])
                fprintf(fp, "Dam  %d\n", mob->damroll.getValue( ));

        if (mob->damage[DICE_NUMBER] != mob->pIndexData->damage[DICE_NUMBER])
                fprintf(fp, "DamN  %d\n", mob->damage[DICE_NUMBER]);

        if (mob->damage[DICE_TYPE] != mob->pIndexData->damage[DICE_TYPE])
                fprintf(fp, "DamT  %d\n", mob->damage[DICE_TYPE]);

        fprintf(fp, "ACs  %d %d %d %d\n",
                mob->armor[0],mob->armor[1],mob->armor[2],mob->armor[3]);

        fprintf(fp, "Attr %d %d %d %d %d %d\n",
                mob->perm_stat[STAT_STR], mob->perm_stat[STAT_INT],
                mob->perm_stat[STAT_WIS], mob->perm_stat[STAT_DEX],
                mob->perm_stat[STAT_CON], mob->perm_stat[STAT_CHA]);

        fprintf(fp, "AMod %d %d %d %d %d %d\n",
                mob->mod_stat[STAT_STR], mob->mod_stat[STAT_INT],
                mob->mod_stat[STAT_WIS], mob->mod_stat[STAT_DEX],
                mob->mod_stat[STAT_CON], mob->mod_stat[STAT_CHA]);

        for (auto &paf: mob->affected)
            fwrite_affect( "Affc", fp, paf );

        // Put the mob back the way it was found. Everything past this point --
        // behaviors, carried objects -- must see a live mob, not a stripped one.
        for (auto &paf: mob->affected)
            affect_modify( mob, paf, true );

        fprintf(fp,"End\n");

        MobileBehaviorManager::save( mob, fp );

        if ( mob->carrying != 0 )
        {
                fprintf( fp, "\n" );
                fwrite_obj( mob, mob->carrying, fp, 0 );
        }

        return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( Character *ch, Object *obj, FILE *fp, int iNest )
{
        Object *last = 0;
        Object *curr, *prelast;

        if ( obj == 0 )
                return;

        last = obj;

        while ( last->next_content != 0 )
                last = last->next_content;

        while ( last != 0 )
        {
                curr = obj;

                prelast = 0;
                
                while ( curr != last )
                {
                        prelast = curr;
                        curr = curr->next_content;
                }

                last = prelast;

                fwrite_obj_0( ch, curr, fp, iNest );
        }
}


void fwrite_obj_0( Character *ch, Object *obj, FILE *fp, int iNest )
{
    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
//        if ( obj->next_content != 0 )
//                fwrite_obj( ch, obj->next_content, fp, iNest );


    if (IS_SET(obj->pIndexData->area->area_flag, AREA_NOSAVEDROP))
        return;

    if (IS_SET(obj->extra_flags, ITEM_NOSAVEDROP))
        return;

    /*
     * Castrate storage characters and rooms.
     */
        if (ch != 0 && !ch->is_npc( ) && !ch->is_immortal( ))
        {
            if (!obj->hasOwner( ch ) && obj->mustDisappear( ch ))
            {
                ch->pecho(_("%1$#^O1 рассыпа%1$#nется|ются трухой!"), obj);
                extract_obj( obj );
                return;
            }

            // Someone else's named item crumbles rather than being saved along.
            if (!obj_owner_allows( obj, ch )) {
                ch->pecho(_("%1$#^O1 исчеза%1$#nет|ют!"), obj);
                extract_obj( obj );
                return;
            }

            if (obj->behavior)
                if (obj->behavior->save( ))
                    return;
        }



        try
        {
                fprintf( fp, "#O\n" );
                fprintf( fp, "Vnum %d\n",           obj->pIndexData->vnum                );
                
                if (obj->timestamp > 0) {
                    fprintf( fp, "TS %s\n", id_to_string(obj->timestamp).c_str() );
                }

                fprintf( fp, "Id   %s\n", id_to_string(obj->getID()).c_str() );

                if (obj->reset_room != 0)
                    fprintf(fp, "RRoom %d\n", obj->reset_room);
                if (obj->reset_mob != 0)
                    fprintf(fp, "RMob %s\n", id_to_string(obj->reset_mob).c_str());
                if (obj->reset_obj != 0)
                    fprintf(fp, "RObj %s\n", id_to_string(obj->reset_obj).c_str());

                fprintf( fp, "Cond %d\n",                obj->condition                        );

                fprintf( fp, "Nest %d\n",        iNest                       );

                if (!obj->pocket.empty( ))
                    fprintf( fp, "Pocket %s~\n", obj->pocket.c_str( ) );

                /* these data are only used if they do not match the defaults */
                if (!obj->getRealMaterial( ).empty())
                    fprintf( fp, "Material %s~\n",  obj->getMaterial( ).c_str());

                fwrite_multistring(fp, "Keyword", obj->getRealKeyword());
                fwrite_multistring(fp, "ShortDesc", obj->getRealShortDescr());
                fwrite_multistring(fp, "Description", obj->getRealDescription());

                if (!obj->getOwner().empty())
                    fprintf( fp, "Ownr %s~\n",        obj->getOwner().c_str());

                if ( obj->extra_flags != obj->pIndexData->extra_flags)
                        fprintf( fp, "ExtF %d\n",        obj->extra_flags             );
                if ( obj->wear_flags != obj->pIndexData->wear_flags)
                        fprintf( fp, "WeaF %d\n",        obj->wear_flags                     );
                if ( obj->item_type != obj->pIndexData->item_type)
                        fprintf( fp, "Ityp %d\n",        obj->item_type                     );
                if ( obj->weight != obj->pIndexData->weight)
                        fprintf( fp, "Wt   %d\n",        obj->weight                     );

                        /* variable data */

                fprintf( fp, "Wearloc %s\n",   obj->wear_loc.getName( ).c_str( ) );
                if (obj->level != obj->pIndexData->level)
                        fprintf( fp, "Lev  %d\n",        obj->level                     );
                if (obj->timer != 0)
                        fprintf( fp, "Time %d\n",        obj->timer             );
                fprintf( fp, "Cost %d\n",        obj->cost                     );
                if (obj->econRev > 0)
                        fprintf( fp, "EconRev %d\n",  obj->econRev            );
                if ( obj->value0() != obj->pIndexData->value[0]
                        || obj->value1() != obj->pIndexData->value[1]
                        || obj->value2() != obj->pIndexData->value[2]
                        || obj->value3() != obj->pIndexData->value[3]
                        || obj->value4() != obj->pIndexData->value[4] )
                        fprintf( fp, "Val  %d %d %d %d %d\n",
                                obj->value0(), obj->value1(), obj->value2(), obj->value3(),
                                obj->value4() );

                switch ( obj->item_type )
                {
                case ITEM_POTION:
                case ITEM_SCROLL:
                        if ( obj->value1() > 0 )
                                fprintf( fp, "Spell 1 '%s'\n", skillname(obj->value1()).c_str() );
                        if ( obj->value2() > 0 )
                                fprintf( fp, "Spell 2 '%s'\n", skillname(obj->value2()).c_str() );
                        if ( obj->value3() > 0 )
                                fprintf( fp, "Spell 3 '%s'\n", skillname(obj->value3()).c_str() );
                        if ( obj->value4() > 0 )
                                fprintf( fp, "Spell 4 '%s'\n", skillname(obj->value4()).c_str() );
                        break;

                case ITEM_PILL:
                case ITEM_STAFF:
                case ITEM_WAND:
                        if ( obj->value3() > 0 )
                                fprintf( fp, "Spell 3 '%s'\n", skillname(obj->value3()).c_str() );
                        break;

                case ITEM_FOUNTAIN:
                case ITEM_DRINK_CON:
                        fprintf( fp, "Liquid '%s'\n",  
                                     liquidManager->find( obj->value2() )->getName( ).c_str( ) );
                        break;
                }

                for (auto &paf: obj->affected)
                    fwrite_affect( "Affect", fp, paf );

                for (auto &ed: obj->extraDescriptions) {
                    fwrite_multistring(fp, "ExtraDesc", ed->keyword, ed->description);
                }

                if (!obj->props.empty()) {
                    DLString jsonString = JsonUtils::toString(obj->props);
                    fprintf(fp, "Props %s", jsonString.c_str());
                }

                if (obj->gram_gender != Grammar::MultiGender::UNDEF)
                        fprintf(fp, "Gender %s\n", obj->gram_gender.toString());

                fprintf( fp, "End\n" );
                ObjectBehaviorManager::save( obj, fp );
                fprintf( fp, "\n" );

                if ( obj->contains != 0 )
                        fwrite_obj( ch, obj->contains, fp, iNest + 1 );
        }
        catch(const Exception &e)
        {
            DLString objId = !obj ? "-" : DLString(obj->getID());
            DLString objVnum = !obj ? "-" : (!obj->pIndexData ? "-" : DLString(obj->pIndexData->vnum));
            LogStream::sendError() << "fwrite_obj: " << objId << " " << objVnum << e.what() << endl;
        }

        return;
}




/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                        \
                                if ( !strcmp( word, literal ) )        \
                                {                                        \
                                    field  = value;                        \
                                    fMatch = true;                        \
                                    break;                                \
                                }

#define KEYSKIP( literal )                                                \
                                if ( !strcmp( word, literal ) )        \
                                {                                        \
                                    fread_to_eol( fp );                        \
                                    fMatch = true;                        \
                                    break;                                \
                                }
#define KEYV( literal, field, value )                                        \
                                if ( !strcmp( word, literal ) )        \
                                {                                        \
                                    field = value;                        \
                                    fMatch = true;                        \
                                    break;                                \
                                }

// Read historical name/keyword fields that could be a mix of EN and RU words
static XMLMultiString fread_mixed_multistring(FILE *fp)
{
    DLString valueString = fread_dlstring( fp );
    
    XMLMultiString multi;
    multi.fromMixedString(valueString);
    return multi;
}

// Read new format with 'lang' attribute in it
static void fread_multistring(FILE *fp, DLString &valueString, lang_t &lang)
{
    DLString langString(fread_word(fp));
    lang = attr2lang(langString);

    valueString = fread_dlstring(fp);
}

static void fread_multistring(FILE *fp, DLString &keyString, DLString &valueString, lang_t &lang)
{
    DLString langString(fread_word(fp));
    lang = attr2lang(langString);

    keyString = fread_dlstring(fp);

    valueString = fread_dlstring(fp);
}

static void fread_char_raw( PCharacter *ch, FILE *fp )
{
    const char *word="End";
    bool fMatch = true;
    int dummy;

    LogStream::sendNotice( ) << "Loading " << ch->getName( ) << '.' << endl;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = false;

        switch ( dl_toupper(word[0]) )
        {
        case '*':
            fMatch = true;
            fread_to_eol( fp );
            break;

        case 'A':
            KEY( "Act",                ch->act,                fread_flag( fp ) );
            KEY( "AffectedBy",        ch->affected_by,        fread_flag( fp ) );
            KEY( "AfBy",        ch->affected_by,        fread_flag( fp ) );
            KEY( "Alignment",        ch->alignment,                fread_number( fp ) );
            KEY( "Alig",        ch->alignment,                fread_number( fp ) );
            KEY( "AntKilled",        ch->anti_killed,fread_number( fp ) );

            if (!strcmp(word,"ACs"))
            {
                int i;

                for (i = 0; i < 4; i++)
                    ch->armor[i] = fread_number(fp);
                fMatch = true;
                break;
            }

            if (!strcmp(word, "Affc"))
            {
                Affect *paf = fread_affect( fp );
                ch->affected.push_front(paf);
                fMatch = true;
                break;
            }

            if (!strcmp(word, "Aff2"))
            {
                Affect *paf = fread_affect( fp, true );
                ch->affected.push_front(paf);
                fMatch = true;
                break;
            }

            if ( !strcmp( word, "AttrMod"  ) || !strcmp(word,"AMod"))
            {
                int stat;
                for (stat = 0; stat < stat_table.size; stat ++)
                   ch->mod_stat[stat] = fread_number(fp);
                fMatch = true;
                break;
            }

            if ( !strcmp( word, "AttrPerm" ) || !strcmp(word,"Attr"))
            {
                int stat;

                for (stat = 0; stat < stat_table.size; stat++)
                    ch->perm_stat[stat] = fread_number(fp);
                fMatch = true;
                break;
            }
            break;

        case 'B':
            KEYV( "Bamfin",        ch->bamfin,        fread_dlstring( fp ) );
            KEY( "Banks",       ch->bank_s,     fread_number( fp ) );
            KEY( "Bankg",       ch->bank_g,     fread_number( fp ) );
            KEYV( "Bamfout",        ch->bamfout,        fread_dlstring( fp ) );
            KEYV( "Bin",                ch->bamfin,        fread_dlstring( fp ) );
            KEYV( "Bout",        ch->bamfout,        fread_dlstring( fp ) );
            KEY( "Bless",        dummy,        fread_number( fp ) );

            if(!strcmp(word, "BatlePrompt") || !strcmp(word, "BatleProm")) {
                ch->batle_prompt = fread_dlstring(fp);
                fMatch = true;
                break;
            }
            break;

        case 'C':
            KEYV( "Config",        ch->config,                fread_number( fp ) );

            if (!strcmp(word,"CndC"))
            {
                ch->desires[desire_drunk] = fread_number( fp );
                fread_number( fp ); // legacy 'full' desire slot (removed in the hunger/thirst rework); read and discard to keep the fixed CndC field order parseable
                ch->desires[desire_thirst] = fread_number( fp );
                ch->desires[desire_hunger] = fread_number( fp );
                ch->desires[desire_bloodlust] = fread_number( fp );
                fread_number( fp );
                fMatch = true;
                break;
            }
            KEY( "Comm",        ch->comm,                fread_flag( fp ) );
            KEY( "Comm_Add",        ch->add_comm,           fread_flag( fp ) );
            KEY( "Curse",        dummy,        fread_number( fp ) );

            break;

        case 'D':
            if( !strcmp( word, "DeathT" ) ) {
              ch->last_death_time = fread_number( fp );
              fMatch = true;
              break;
            }
            KEY( "Damroll",        ch->damroll,                fread_number( fp ) );
            KEY( "Dam",                ch->damroll,                fread_number( fp ) );
            KEY( "Dead",        ch->death,        fread_number( fp ) );
                KEY( "Detect",        ch->detection,                fread_flag(fp)     );
            
            if (!strcmp( word, "Description" ) || !strcmp( word, "Desc" )) {
                DLString word = fread_dlstring( fp );
                ch->setDescription( word, LANG_DEFAULT );
                fMatch = true;
            }
            break;

        case 'E':
            if ( !strcmp( word, "End" ) )
            {
                return;
            }
            KEY( "Exp",                ch->exp,                fread_number( fp ) );
            KEYV( "Etho",        ch->ethos,                fread_number( fp ) );
            break;

        case 'G':
            KEY( "GHOST",        ch->ghost_time,        fread_number( fp ) );
            KEY( "Gold",        ch->gold,                fread_number( fp ) );
            break;

        case 'H':
            KEY( "Hitroll",        ch->hitroll,                fread_number( fp ) );
            KEY( "Hit",                ch->hitroll,                fread_number( fp ) );
            KEY( "Haskilled",        ch->has_killed, fread_number( fp ) );
            if ( !strcmp( word, "HpManaMove" ) || !strcmp(word,"HMV"))
            {
                ch->hit                = fread_number( fp );
                ch->max_hit        = fread_number( fp );
                ch->mana        = fread_number( fp );
                ch->max_mana        = fread_number( fp );
                ch->move        = fread_number( fp );
                ch->max_move        = fread_number( fp );
                fMatch = true;
                break;
            }

            if ( !strcmp( word, "HpManaMovePerm" ) || !strcmp(word,"HMVP"))
            {
                ch->perm_hit        = fread_number( fp );
                ch->perm_mana   = fread_number( fp );
                ch->perm_move   = fread_number( fp );
                fMatch = true;
                break;
            }

            break;

        case 'I':
            KEY( "InvisLevel",        ch->invis_level,        fread_number( fp ) );
            KEY( "Inco",        ch->incog_level,        fread_number( fp ) );
            KEY( "Invi",        ch->invis_level,        fread_number( fp ) );
            
            if (!strcmp( word, "Id" )) {
                ch->setID( fread_number64( fp ) );
                fMatch = true;
                break;
            }

            break;

        case 'L':
            KEY( "LastLevel",        ch->last_level, fread_number( fp ) );
            KEY( "LLev",        ch->last_level, fread_number( fp ) );
            KEYSKIP( "LogO" );
            KEYSKIP( "LastTime" );
            break;
        case 'M':
                KEYSKIP( "MaxSkillPoints" );
                break;
        case 'N':
            if ( !strcmp( word, "Name" ) )
            {
                DLString name = fread_dlstring( fp );
                ch->setName( name );
                fMatch = true;
                break;
            }
            break;

        case 'P':
            KEY( "PKFlag",        ch->PK_flag,        fread_number( fp ) );
            KEY( "PKTimeV",        ch->PK_time_v,        fread_number( fp ) );
            KEY( "PKTimeSK",        ch->PK_time_sk,        fread_number( fp ) );
            KEY( "PKTimeT",        ch->PK_time_t,        fread_number( fp ) );
            KEYV( "Position",        ch->position,                fread_number( fp ) );
            KEYV( "Pos",        ch->position,                fread_number( fp ) );
            KEY( "Practice",        ch->practice,                fread_number( fp ) );
            KEY( "Prac",        ch->practice,                fread_number( fp ) );
            
            if (!strcmp( word, "Played" ) || !strcmp( word, "Plyd" )) {
                ch->age.setTruePlayed( fread_number( fp ) );
                fMatch = true;
                break;
            }
            if (!strcmp( word, "Pass" )) {
                DLString pwd = fread_dlstring( fp );
                password_set( ch, pwd );
                fMatch = true;
                break;
            }
            
            if(!strcmp(word, "Prompt") || !strcmp(word, "Prom")) {
                ch->prompt = fread_dlstring(fp);
                fMatch = true;
                break;
            }
            break;
        case 'Q':
            if (!strcmp(word, "QuestPnts")) {
                ch->setQuestPoints(fread_number(fp));
                fMatch = true;
                break;
            }
            break;

        case 'R':
            if ( !strcmp( word, "Room" ) )
            {
                ch->setStartRoom(fread_number( fp ));
                fMatch = true;
                break;
            }
            if (!strcmp( word, "Relig" )) {
                convert_religion( ch, fread_number( fp ) );
                fMatch = true;
                break;
            }

            break;

        case 'S':
            KEY( "SavingThrow",        ch->saving_throw,        fread_number( fp ) );
            KEY( "Save",        ch->saving_throw,        fread_number( fp ) );
            KEY( "Scro",        ch->lines,                fread_number( fp ) );
            KEY( "Shadow",        ch->shadow,                fread_number( fp ) );
            KEY( "Silv",        ch->silver,             fread_number( fp ) );


            if ( !strcmp( word, "Skill1" ) )
            {
                    int sn;
                    int value;
                    int timer;
                    char *temp;

                    value = fread_number( fp );
                    timer = fread_number( fp );
                    temp = fread_word( fp ) ;
                    /* forget */ fread_number( fp );
                    sn = SkillManager::getThis( )->lookup(temp);
                    convert_skill( sn );

                    if ( sn < 0 )
                    {
                        LogStream::sendWarning( ) << "Fread_char: unknown skill " << temp << endl;
                    }
                    else
                    {
                        PCSkillData &sk = ch->getSkillData( sn );
                        
                        sk.learned = value;
                        sk.timer = timer;
                    }
                    fMatch = true;
                    break;
            }

            break;

        case 'T':
            KEY( "Trai",        ch->train,        fread_number( fp ) );
            KEYSKIP( "Trust" );
            KEYSKIP( "Tru" );

            if ( !strcmp( word, "Title" )  || !strcmp( word, "Titl"))
            {
                DLString word = fread_dlstring( fp );
                ch->setTitle( word );
                fMatch = true;
                break;
            }
            else if (!strcmp( word, "TwitName" ) ) {
                fMatch = true;
                break;
            }

            break;

        case 'V':
            KEYSKIP( "Version" );
            KEYSKIP( "Vers" );
            break;

        case 'W':
            KEY( "Wimpy",        ch->wimpy,                fread_number( fp ) );
            KEY( "Wimp",        ch->wimpy,                fread_number( fp ) );
            KEY( "Wizn",        ch->wiznet,        fread_flag( fp ) );
            break;
        }

        if ( !fMatch )
        {
            LogStream::sendWarning( ) << "Fread_char: no match[" << word << "]." << endl;
            fread_to_eol( fp );
        }
    }
}

void fread_char( PCharacter *ch, FILE *fp )
{
    int percent;

    fread_char_raw( ch, fp );

    /* adjust hp mana move up  -- here for speed's sake */
    percent = ( dreamland->getCurrentTime( ) - ch->last_logoff) * 25 / ( 2 * 60 * 60);
    percent = std::min(percent,100);

    if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON) &&  !IS_AFFECTED(ch,AFF_PLAGUE))
    {
        ch->hit        += (ch->max_hit - ch->hit) * percent / 100;
        ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        ch->move    += (ch->max_move - ch->move)* percent / 100;
    }

    if (FeniaManager::wrapperManager)
        FeniaManager::wrapperManager->linkWrapper( ch );
}

/* load a pet from the forgotten reaches */
void fread_pet( PCharacter *ch, FILE *fp )
{
    const char *word;
    NPCharacter *pet;
    bool fMatch;
    int percent;
    DLString value;
    lang_t lang;
    bitstring_t create_flags = FCREATE_NOAFFECTS | FCREATE_NOCOUNT;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "End" : fread_word(fp);
    if (!strcmp(word,"Vnum"))
    {
            int vnum;

            vnum = fread_number(fp);
            if (get_mob_index(vnum) == 0)
        {
                bug("Fread_pet: bad vnum %d.",vnum);
            pet = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
        }
            else
                pet = create_mobile_org(get_mob_index(vnum), create_flags);
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
    }

    SavedMobState saved;

    for ( ; ; )
    {
            word         = feof(fp) ? "End" : fread_word(fp);
            fMatch = false;

            if (fread_saved_mob_key(word, fp, saved, true))
                continue;

            switch (dl_toupper(word[0]))
            {
            case '*':
                fMatch = true;
                fread_to_eol(fp);
                break;

            case 'A':
                KEY( "Act",                pet->act,                fread_flag(fp));
                KEY( "AfBy",        pet->affected_by,        fread_flag(fp));
                KEY( "Alig",        pet->alignment,                fread_number(fp));

                if (!strcmp(word,"ACs"))
                {
                        int i;

                        for (i = 0; i < 4; i++)
                            pet->armor[i] = fread_number(fp);
                        fMatch = true;
                        break;
                }

            if (!strcmp(word,"Affc"))
            {
                Affect *paf = fread_affect( fp );

                pet->affected.push_front(paf);
                fMatch          = true;
                break;
            }

            if (!strcmp(word,"Aff2"))
            {
                Affect *paf = fread_affect( fp, true );

                pet->affected.push_front(paf);
                fMatch          = true;
                break;
            }

                if (!strcmp(word,"AMod"))
                {
                         int stat;

                         for (stat = 0; stat < stat_table.size; stat++)
                             pet->mod_stat[stat] = fread_number(fp);
                         fMatch = true;
                         break;
                }

                if (!strcmp(word,"Attr"))
                {
                     int stat;

                     for (stat = 0; stat < stat_table.size; stat++)
                         pet->perm_stat[stat] = fread_number(fp);
                     fMatch = true;
                     break;
                }
                break;

             case 'C':
                 KEY( "Comm",        pet->comm,                fread_flag(fp));

                if (!strcmp(word, "Clan")) {
                    DLString word = fread_dlstring(fp);
                    Clan *clan = clanManager->findExisting(word);
                    if (clan)
                        pet->setClan(clan->getName());

                    fMatch = true;
                    break;
                }

                 break;

             case 'D':
                 KEY( "Dam",        pet->damroll,                fread_number(fp));
                 KEY( "DamT",       pet->damage[DICE_TYPE],      fread_number(fp));
                 KEY( "DamN",       pet->damage[DICE_NUMBER],    fread_number(fp));
                 KEY( "Detect",        pet->detection,                fread_flag(fp));

                if (!strcmp( word, "Desc" )) {
                    DLString word = fread_dlstring( fp );
                    pet->setDescription( word, LANG_DEFAULT );
                    fMatch = true;
                    break;
                }
                if (!strcmp(word, "Description")) {
                    fread_multistring(fp, value, lang);
                    pet->setDescription(value, lang);
                    fMatch = true;
                    break;
                }

                 break;

             case 'E':
                 if (!strcmp(word, "End")) {
                     apply_saved_pet(pet, saved);
                     pet->leader = ch;
                     pet->master = ch;
                     ch->pet = pet;
                     /* adjust hp mana move up  -- here for speed's sake */
                     percent = (dreamland->getCurrentTime() - ch->last_logoff) * 25 / (2 * 60 * 60);

                     if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON) && !IS_AFFECTED(ch, AFF_PLAGUE)) {
                         percent = std::min(percent, 100);
                         pet->hit += (pet->max_hit - pet->hit) * percent / 100;
                         pet->mana += (pet->max_mana - pet->mana) * percent / 100;
                         pet->move += (pet->max_move - pet->move) * percent / 100;
                     }

                     if (FeniaManager::wrapperManager)
                         FeniaManager::wrapperManager->linkWrapper(pet);

                     LogStream::sendNotice() << "LOAD " << ch->getName() << " pet " << pet->getID() << " [" << pet->pIndexData->vnum << "]" << endl;    
                     return;
                 }
                 KEY("Exp", pet->exp, fread_number(fp));
                 break;

             case 'G':
                 KEY( "Gold",        pet->gold,                fread_number(fp));
                 break;
            
             case 'H':
                 KEY( "Hit",        pet->hitroll,                fread_number(fp));

                 if (!strcmp(word,"HMV"))
                 {
                         pet->hit        = fread_number(fp);
                         pet->max_hit        = fread_number(fp);
                         pet->mana        = fread_number(fp);
                         pet->max_mana        = fread_number(fp);
                         pet->move        = fread_number(fp);
                         pet->max_move        = fread_number(fp);
                         fMatch = true;
                         break;
                 }
                 break;

        case 'I':
            if (!strcmp( word, "Id" )) {
                pet->setID( fread_number64( fp ) );
                fMatch = true;
                break;
            }

            break;
        case 'K':
            if (!strcmp(word, "Keyword")) {
                fread_multistring(fp, value, lang);
                pet->setKeyword(value, lang);
                fMatch = true;
                break;
            }
            break;

        case 'L':
             KEYSKIP( "LogO" );
            if( !strcmp( word, "Levl" ) )
            {
                    pet->setLevel( fread_number( fp ) );
                    fMatch = true;
                    break;
            }
            if (!strcmp( word, "LnD" )) {
                DLString word = fread_dlstring( fp );
                pet->setLongDescr( word, LANG_DEFAULT );
                fMatch = true;
                break;
            }
            if (!strcmp(word, "LongDesc")) {
                fread_multistring(fp, value, lang);
                pet->setLongDescr(value, lang);
                fMatch = true;
                break;
            }
                break;

            case 'N':
                if ( !strcmp( word, "Name" ) )
                {
                    pet->setKeyword(fread_mixed_multistring(fp));
                    fMatch = true;
                    break;
                }
                 break;

            case 'P':
                 KEYV( "Pos",        pet->position,                fread_number(fp));
                 break;

        case 'R':
            if ( !strcmp( word, "Race" ) )
            {
                    pet->setRace( fread_dlstring(fp) );
                    fMatch = true;
                break;
            } else if (!strcmp( word, "Room" )) {
                    int rvnum = fread_number( fp );
                    pet->in_room = get_room_instance( rvnum );
                    if (!pet->in_room) {
                        LogStream::sendError( ) << "fread_pet: invalid room " << rvnum << " for " << ch->getName( ) << endl;
                    } 
                    fMatch = true;
                    break;
            }
                break;

            case 'S' :
                KEY( "Save",        pet->saving_throw,        fread_number(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );

            if (!strcmp( word, "ShD" )) {
                pet->setShortDescr( fread_dlstring(fp), LANG_DEFAULT );
                fMatch = true;
                break;
            }
            if (!strcmp(word, "ShortDesc")) {
                fread_multistring(fp, value, lang);
                pet->setShortDescr(value, lang);
                fMatch = true;
                break;
            }
            if( !strcmp( word, "Sex" ) )
            {
                pet->setSex( fread_number( fp ) );
                fMatch = true;
                break;
            }
                break;

            case 'W':
                if (!strcmp(word, "Wearloc")) {
                    pet->wearloc.fromString(fread_dlstring(fp));
                    fMatch = true;
                    break;
                }
                break;
            }

            if ( !fMatch )
            {
                bug("Fread_pet: no match.",0);
                fread_to_eol(fp);
            }
            
    }
}

/* load a mobile from the forgotten reaches */
NPCharacter * fread_mob( FILE *fp )
{
    const char *word;
    NPCharacter *mob;
    bool fMatch;
    DLString value;
    lang_t lang;
    bitstring_t create_flags = FCREATE_NOAFFECTS | FCREATE_NOCOUNT;

    // first entry had BETTER be the vnum or we barf
    word = feof(fp) ? "End" : fread_word(fp);

    if ( !strcmp(word,"Vnum") )
    {
            int vnum;
    
            vnum = fread_number(fp);
            if ( get_mob_index (vnum ) == 0 )
            {
                    bug("Fread_mob: bad vnum %d.",vnum);
                    mob = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
            }
            else
                    mob = create_mobile_org(get_mob_index(vnum), create_flags);
    }
    else
    {
            bug("Fread_mob: no vnum in file.",0);
            mob = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
    }

    mob->pIndexData->count++;
    mob->affected.deallocate();

    SavedMobState saved;

    try {
        for ( ; ; )
        {
            word         = feof(fp) ? "End" : fread_word(fp);

            fMatch = false;

            if (fread_saved_mob_key(word, fp, saved, false))
                continue;

            switch (dl_toupper(word[0]))
            {
            case '*':
                    fMatch = true;
                    fread_to_eol(fp);
                    break;

            case 'A':
                    KEY( "Act",                mob->act,                fread_flag(fp));
                    KEY( "AfBy",        mob->affected_by,        fread_flag(fp));
                    KEY( "Alig",        mob->alignment,                fread_number(fp));

                    if ( !strcmp(word,"ACs") )
                    {
                            int i;

                            for ( i = 0; i < 4; i++ )
                                    mob->armor[i] = fread_number(fp);
                            fMatch = true;
                            break;
                    }

                    if ( !strcmp(word,"Affc") )
                    {
                            Affect *af = fread_affect( fp );

                            affect_to_char( mob, af );
                            ddeallocate( af );
                            fMatch          = true;
                            break;
                    }

                    if ( !strcmp(word,"Aff2") )
                    {
                            Affect *af = fread_affect( fp, true );

                            affect_to_char( mob, af );
                            ddeallocate( af );
                            fMatch          = true;
                            break;
                    }

                    if (!strcmp(word,"AMod"))
                    {
                            int stat;

                            for (stat = 0; stat < stat_table.size; stat++)
                                    mob->mod_stat[stat] = fread_number(fp);
                            fMatch = true;
                            break;
                    }

                    if (!strcmp(word,"Attr"))
                    {
                            int stat;

                            for (stat = 0; stat < stat_table.size; stat++)
                                    mob->perm_stat[stat] = fread_number(fp);
                            fMatch = true;
                            break;
                    }
                    break;

            case 'C':
                    KEY( "Comm",        mob->comm,                fread_flag(fp));

                    if (!strcmp( word, "Cab" )) {
                        fread_number( fp );
                        fMatch = true;
                        break;
                    }

                    if (!strcmp(word, "Clan")) {
                        Clan *clan = clanManager->findExisting(fread_dlstring(fp));
                        if (clan)
                            mob->setClan(clan->getName());

                        fMatch = true;
                        break;
                    }

                    break;

            case 'D':
                    KEY( "Dam",        mob->damroll,                fread_number(fp));
                    KEY( "DamT",       mob->damage[DICE_TYPE],      fread_number(fp));
                    KEY( "DamN",       mob->damage[DICE_NUMBER],    fread_number(fp));
                    KEY( "Detect",        mob->detection,                fread_flag(fp));

                    if (!strcmp( word, "Desc" )) {
                        mob->setDescription( fread_dlstring(fp), LANG_DEFAULT );
                        fMatch = true;
                        break;
                    }
                    if (!strcmp(word, "Description")) {
                        fread_multistring(fp, value, lang);
                        mob->setDescription(value, lang);
                        fMatch = true;
                        break;
                    }

                    break;

            case 'E':
                    if (!strcmp(word,"End"))
                    {
                            apply_saved_mob(mob, saved);
                            mob->leader = 0;
                            mob->master = 0;

                            MobileBehaviorManager::parse( mob, fp );
                            if (FeniaManager::wrapperManager)
                                FeniaManager::wrapperManager->linkWrapper( mob );
                            return mob;
                    }

                    KEY( "Exp",        mob->exp,                fread_number(fp));
                    break;
            case 'F':
                    KEY("Form", mob->form, fread_flag(fp));

                    break;
            case 'G':
                    KEY( "Gold",        mob->gold,                fread_number(fp));
                    break;

            case 'H':
                    KEY( "Hit",        mob->hitroll,                fread_number(fp));

                    if (!strcmp(word,"HMV"))
                    {
                            mob->hit        = fread_number(fp);
                            mob->max_hit        = fread_number(fp);
                            mob->mana        = fread_number(fp);
                            mob->max_mana        = fread_number(fp);
                            mob->move        = fread_number(fp);
                            mob->max_move        = fread_number(fp);
                            fMatch = true;
                            break;
                    }
                    break;

            case 'I':
                    KEY("Imm", mob->imm_flags, fread_flag(fp));

                    if (!strcmp( word, "Id" )) {
                        mob->setID( fread_number64( fp ) );
                        fMatch = true;
                        break;
                    }

                    break;

            case 'K':
                if (!strcmp(word, "Keyword")) {
                    fread_multistring(fp, value, lang);
                    mob->setKeyword(value, lang);
                    fMatch = true;
                    break;
                }
                break;

            case 'L':
                    if ( !strcmp( word, "Levl" ) )
                    {
                            mob->setLevel( fread_number( fp ) );
                            fMatch = true;
                            break;
                    }

                    if (!strcmp( word, "LnD" )) {
                        mob->setLongDescr( fread_dlstring(fp), LANG_DEFAULT );
                        fMatch = true;
                        break;
                    }
                    if (!strcmp(word, "LongDesc")) {
                        fread_multistring(fp, value, lang);
                        mob->setLongDescr(value, lang);
                        fMatch = true;
                        break;
                    }

                    break;

            case 'N':
                    if ( !strcmp( word, "Name" ) )
                    {
                        mob->setKeyword(fread_mixed_multistring(fp));
                        fMatch = true;
                        break;
                    }
                    break;

            case 'P':
                    KEYV( "Pos",        mob->position,                fread_number(fp));
                    KEY("Part", mob->parts, fread_flag(fp));

                    break;

            case 'R':
                    KEY("Res", mob->res_flags, fread_flag(fp));

                    if (!strcmp( word, "Room" )) {
                        mob->in_room = get_room_instance( fread_number( fp ) );
                        fMatch = true;
                        break;
                    }

                    if ( !strcmp( word, "Race" ) )
                    {
                            mob->setRace( fread_dlstring(fp) );
                            fMatch = true;
                            break;
                    }

                    if ( !strcmp( word, "RZone" ) )
                    {
                            DLString zoneName = fread_dlstring( fp );
                            AreaIndexData *pArea = get_area_index(zoneName);
                            if (pArea) {
                                mob->zone = pArea;
                                fMatch = true;
                                break;
                            }
                    }

                    KEY("RRoom", mob->reset_room, fread_number(fp));
                    break;

            case 'S' :
                    KEY( "Save",        mob->saving_throw,        fread_number(fp));
                    KEY( "Silv",        mob->silver,            fread_number( fp ) );
                    KEY("Size", mob->size, fread_number(fp));

                    if( !strcmp( word, "Sex" ) )
                    {
                            int sex = fread_number( fp );

                            if (sex_table.name( sex ).empty( ))
                                mob->setSex( mob->pIndexData->sex );
                            else
                                mob->setSex( sex );

                            if (mob->getSex( ) == SEX_EITHER)
                                mob->setSex( number_range( 1, 2 ) );

                            fMatch = true;
                            break;
                    }
                    if (!strcmp( word, "ShD" )) {
                        mob->setShortDescr( fread_dlstring(fp), LANG_DEFAULT );
                        fMatch = true;
                        break;
                    }
                    if (!strcmp(word, "ShortDesc")) {
                        fread_multistring(fp, value, lang);
                        mob->setShortDescr(value, lang);
                        fMatch = true;
                        break;
                    }

                    break;

            case 'T':
                    KEY( "Timer",        mob->timer,                fread_number( fp ) );
                    break;

            case 'V':
                    KEY("Vuln", mob->vuln_flags, fread_flag(fp));
                    break;

            case 'W':
                if (!strcmp(word, "Wearloc")) {
                    mob->wearloc.fromString(fread_dlstring(fp));
                    fMatch = true;
                    break;
                }
                break;

            }
            
            if ( !fMatch )
            {
                    bug("Fread_mob: no match.",0);
                    fread_to_eol(fp);
            }
        }

    } catch (const FileFormatException &e) {
        for (auto &af: saved.affects)
            ddeallocate(af);
        extract_mob_baddrop( mob );
        throw e;
    }
    
}

void fread_mlt( PCharacter *ch, FILE *fp ) {
  bool fMatch;
  const char *word;
  int i;

  word   = feof( fp ) ? "End" : fread_word( fp );
  if( strcmp( word, "MLTLv" ) ) {
    bug( "fread_mlt: no remort count.", 0 );
    fread_to_eol( fp );
    return;
  }

  fread_number( fp );
  i = -1;

  for( ; ; ) {
    word   = feof( fp ) ? "End" : fread_word( fp );
    if( word[0] == '#' ) {
      i++;
      word   = feof( fp ) ? "End" : fread_word( fp );
    }
    if( i < 0 ) i++; // на всяк случай
    switch( dl_toupper( word[0] ) ) {
      case '*':
        fMatch = true;
        fread_to_eol( fp );
        break;
      case 'C':
        KEYSKIP( "Class" );
        break;
      case 'E':
        if( !strcmp( word, "End" ) ) {
          return;
        }
        break;
      case 'R':
        KEYSKIP( "Race" );
        break;
      case 'T':
        KEYSKIP( "Time" );
        break;
      if( !fMatch ) {
          bug( "fread_mlt: no match.", 0 );
          fread_to_eol( fp );
      }
    }
  }
}

static void convert_personal( Object *obj )
{
    // Katanas are now born complete from the craft pipeline -- ownership is the
    // owner field plus the proto's 'master samurai' Fenia behavior, so a legacy
    // 'personal' katana needs no per-instance C++ behavior. Leave it as loaded.
    if (obj->pIndexData->vnum == OBJ_VNUM_KATANA_SWORD)
        return;

    AllocateClass::Pointer pointer = Class::allocateClass( "PersonalQuestReward" );
    ObjectBehavior::Pointer behavior = pointer.getDynamicPointer<ObjectBehavior>( );
    obj->behavior.setPointer( *behavior );
    obj->behavior->setObj( obj );

    LogStream::sendNotice( ) 
            << "Personal: obj [" << obj->pIndexData->vnum << "], "
            << "ID " << obj->getID( ) << ", owner " << obj->getOwner( ) << endl;
}

#define OBJ_VNUM_STUB 127

void fread_obj( Character *ch, Room *room, FILE *fp )
{
    Object *obj;
    const char *word;
    int iNest;
    bool fMatch = true;
    bool fNest;
    bool fVnum;
    bool first;
    bool fPersonal;
    int wear_loc = -1;
    int vnum = 0;
    int econRev = 0;
    DLString value;
    lang_t lang;
    AffectList affectsOldStyle;

    fVnum = false;
    obj = 0;
    first = true;  /* used to counter fp offset */
    fPersonal = false;

    word   = feof( fp ) ? "End" : fread_word( fp );

    if( !strcmp( word,"Vnum" ) )
    {
            first = false;  /* fp will be in right place */

            vnum = fread_number( fp );
            OBJ_INDEX_DATA *pObj = get_obj_index(vnum);

            if (!pObj)
            {
                bug( "Fread_obj: bad vnum %d in room %d.", vnum, room ? room->vnum : -1 );
                pObj = get_obj_index(OBJ_VNUM_STUB);
            }

            /*this object was already initialized once*/
            obj = create_object_nocount( pObj, -1);
            // The saved EconRev decides, not the stamp a new object gets.
            obj->econRev = 0;
            
            /*init pIndexData counter, in case of bootup*/
            if (create_obj_dropped)
                obj->pIndexData->count++;
    }

    if ( obj == 0 )
    {
        bug( "Fread_obj: zero object", 0 );
        return;
    }

    fNest                = false;
    fVnum                = true;
    iNest                = 0;

    try {

        for ( ; ; )
        {
            if ( first )
                    first = false;
            else
                    word   = feof( fp ) ? "End" : fread_word( fp );
            fMatch = false;

            switch ( dl_toupper(word[0]) )
            {
            case '*':
                    fMatch = true;
                    fread_to_eol( fp );
                    break;

            case 'A':
                    KEYSKIP( "Altar" );
                    if (!strcmp(word,"Affc"))
                    {
                            Affect *paf = fread_affect( fp );
                            affectsOldStyle.push_front(paf);
                            fMatch          = true;
                            break;
                    }
                    if (!strcmp(word, "Affect"))
                    {
                            Affect *paf = fread_affect( fp );
                            obj->affected.push_front(paf);
                            fMatch          = true;
                            break;
                    }

                    break;

            case 'C':
                    KEY( "Cond",        obj->condition,                fread_number( fp ) );
                    KEY( "Cost",        obj->cost,                fread_number( fp ) );
                    break;

            case 'D':
                    if (!strcmp( word, "Desc" )) {
                        obj->setDescription( fread_dlstring(fp), LANG_DEFAULT );
                        fMatch = true;
                        break;
                    }
                    if (!strcmp(word, "Description")) {
                        fread_multistring(fp, value, lang);
                        obj->setDescription(value, lang);
                        fMatch = true;
                        break;
                    }

                    break;

            case 'E':
                    if ( !strcmp( word, "Enchanted"))
                    {
                            // Obsolete logic for working with item instance affects.
                            fMatch         = true;
                            break;
                    }

                    KEY( "ExtraFlags",        obj->extra_flags,        fread_number( fp ) );
                    KEY( "ExtF",        obj->extra_flags,        fread_number( fp ) );
                    KEY( "EconRev",     econRev,                 fread_number( fp ) );

                    if (!strcmp(word,"ExDe"))
                    {
                        DLString kw = fread_dlstring(fp);
                        DLString value = fread_dlstring(fp);
                        obj->addExtraDescr(kw, value, LANG_DEFAULT);
                        fMatch = true;
                        break;
                    }
                    if (!strcmp(word,"ExtraDesc"))
                    {
                        DLString keyword, description;
                        lang_t lang;
                        fread_multistring(fp, keyword, description, lang);
                        obj->addExtraDescr(keyword, description, lang);
                        fMatch = true;
                        break;
                    }

                    if ( !strcmp( word, "End" ) )
                    {

                        if ( !fNest || !fVnum || obj->pIndexData == 0)
                        {
                            bug( "Fread_obj: incomplete object.", 0 );
                            // ddeallocate() freed the object while it was still linked into
                            // object_list and pIndexData->instances, leaving a dangling
                            // pointer in both lists. Unlink it properly instead, and mirror
                            // the create_obj_dropped count bump above so the limit stays
                            // balanced. A null pIndexData can't go through extract_obj_1 (it
                            // derefs pIndexData->area), so fall back to a raw free. The only
                            // way to reach null here is a doubly-corrupt record (a second bad
                            // Vnum key, ~2267) that no writer produces; that rare corner keeps
                            // the old leak and needs the creation-time proto to fix properly
                            // (separate change).
                            if (obj->pIndexData == 0)
                                ddeallocate( obj );
                            else if (create_obj_dropped) {
                                fread_obj_uncount_expired( obj, room );
                                extract_obj( obj );
                            }
                            else
                                extract_obj_nocount( obj );
                            return;
                        }
                        else if (obj->pIndexData->vnum == OBJ_VNUM_STUB) {
                            bug("Fread_obj: extracting stub item %d in room %d", vnum, room ? room->vnum : -1);
                            ObjectBehaviorManager::parse( obj, fp );                            
                            extract_obj( obj );
                            return;
                        }
                        else
                        {
                            // Object affect conversion: getting rid of 'enchanted' logic when proto affects got duplicated on the instance.
                            if (!affectsOldStyle.empty()) {
                                // Random weapons and katanas don't have any other affects on the proto 
                                // so it's safe to add all instance affects back as they were.
                                bool isKatana = any_of(affectsOldStyle.begin(), affectsOldStyle.end(), 
                                                [&](const Affect* paf){ return paf->type.getName() == gsn_katana->getName(); });
                                bool isRandom = !obj->getProperty("tier").empty();

                                if (isKatana || isRandom) {
                                    notice("Fread_obj: retained %d proper affects for %d [%lld]", affectsOldStyle.size(), obj->pIndexData->vnum, obj->getID());
                                    for (auto &paf: affectsOldStyle)
                                        obj->affected.push_back(paf);
                                } else {
                                    // For all other items, retain only temporary affects as those are harmless 
                                    // and may add important bits such as 'noenchant' that we want removed eventually.
                                    for (auto &paf: affectsOldStyle)
                                        if (paf->duration >= 0)
                                            obj->affected.push_back(paf);
                                }
                            }

                            // Economy migration, weight half: before placement, so the
                            // carrier's carry weight adds the new weight.
                            bool econStale = obj_econ_rev > 0 && econRev < obj_econ_rev
                                             && obj_econ_migrate_fn != 0;
                            bool econApplied = econStale && obj_econ_migrate_fn( obj, econRev, 0 );

                            Object *container = 0;
                            if (iNest > 0 && rgObjNest[iNest])
                                container = rgObjNest[iNest-1];

                            if (container)
                                obj_to_obj(obj, container);
                            else if (ch)
                                obj_to_char(obj, ch);
                            else
                                obj_to_room(obj, room);

                            ObjectBehaviorManager::parse( obj, fp );
                            
                            if (FeniaManager::wrapperManager)
                                FeniaManager::wrapperManager->linkWrapper( obj );

                            // Cost half: needs the wrapper (personal shop psCost).
                            if (econStale && obj_econ_migrate_fn != 0)
                                econApplied = obj_econ_migrate_fn( obj, econRev, 1 ) && econApplied;
                            obj->econRev = econApplied ? obj_econ_rev : econRev;

                            // Notify item load listeners such as weapon randomizer.
                            eventBus->publish(ItemReadEvent(obj));

                            if (fPersonal) 
                                convert_personal( obj );

                            if (wear_loc != -1)
                                obj->wear_loc.assign(wear_loc);

                            fread_obj_uncount_expired( obj, room );
                            return;
                        }
                    }
                    break;

            case 'G':
                    if (!strcmp( word, "Gender" )) {
                        obj->gram_gender.fromString(fread_word( fp ));
                        obj->updateCachedNouns();
                        fMatch = true;
                        break;
                    }

                    break;

            case 'I':
                    KEY( "ItemType",        obj->item_type,                fread_number( fp ) );
                    KEY( "Ityp",        obj->item_type,                fread_number( fp ) );
                    
                    if (!strcmp( word, "Id" )) {
                        obj->setID( fread_number64( fp ) );
                        fMatch = true;
                        break;
                    }

                    break;
            case 'K':
                if (!strcmp(word, "Keyword")) {
                    fread_multistring(fp, value, lang);
                    obj->setKeyword(value, lang);
                    fMatch = true;
                    break;
                }
                break;

            case 'L':
                    KEY( "Level",        obj->level,                fread_number( fp ) );
                    KEY( "Lev",                obj->level,                fread_number( fp ) );
                    if (!strcmp( word, "Liquid" )) {
                        obj->value2(liquidManager->lookup(fread_word(fp)));
                        fMatch = true;
                        break;
                    }
                    break;

            case 'M':
                    if (!strcmp( word, "Material" )) {
                        obj->setMaterial( fread_dlstring(fp) );
                        fMatch = true;
                    }
                    break;

            case 'N':
                    if (!strcmp( word, "Name" )) {
                        obj->setKeyword(fread_mixed_multistring(fp));
                        fMatch = true;
                        break;
                    }

                    if ( !strcmp( word, "Nest" ) )
                    {
                            iNest = fread_number( fp );
                            if ( iNest < 0 || iNest >= MAX_NEST )
                            {
                                    bug( "Fread_obj: bad nest %d.", iNest );
                            }
                            else
                            {
                                    rgObjNest[iNest] = obj;
                                    fNest = true;
                            }
                            fMatch = true;
                    }
                    break;

            case 'O':
                    if( !strcmp( word, "ObjPrg" ) )
                    {
                        DLString otype, oname;

                        otype = fread_word( fp );
                        oname = fread_word( fp );

                        if (otype == "get_prog" && oname == "get_prog_quest_reward")
                            fPersonal = true;

                        fMatch = true;
                        break;
                    }

                    if (!strcmp( word, "Ownr" )) {
                        obj->setOwner( fread_dlstring(fp) );
                        fMatch = true;
                    }

                    break;

            case 'P':
                    KEYV( "Pocket", obj->pocket, fread_dlstring( fp ) );
                    KEYSKIP( "Pit" );

                    if (!strcmp(word, "Props")) {
                        DLString jsonString = fread_dlstring_to_eol(fp);

                        ostringstream errbuf;
                        if (!JsonUtils::validate(jsonString, errbuf)) {
                            throw Exception("fread_obj: invalid JSON for object " + DLString(obj->pIndexData->vnum));   
                        }

                        JsonUtils::fromString(jsonString, obj->props);
                        fMatch = true;
                        break;
                    }

                    break;
            
            case 'Q':
                    KEY( "Quality",        obj->condition,                fread_number( fp ) );
                    break;

            case 'R':
                    KEY( "RRoom",         obj->reset_room,        fread_number( fp ) );
                    KEY( "RMob",          obj->reset_mob,         fread_number64( fp ) );
                    KEY( "RObj",          obj->reset_obj,         fread_number64( fp ) );
                    break;

            case 'S':
                    if (!strcmp( word, "ShD" )) {
                        obj->setShortDescr( fread_dlstring(fp), LANG_DEFAULT );
                        fMatch = true;
                        break;
                    }
                    if (!strcmp(word, "ShortDesc")) {
                        fread_multistring(fp, value, lang);
                        obj->setShortDescr(value, lang);
                        fMatch = true;
                        break;
                    }


                    if ( !strcmp( word, "Spell" ) )
                    {
                            int iValue;
                            int sn;

                            iValue = fread_number( fp );
                            sn     = SkillManager::getThis( )->lookup( fread_word( fp ) );
                            convert_skill( sn );

                            if ( iValue < 0 || iValue > 4 )
                            {
                                    bug( "Fread_obj: bad iValue %d.", iValue );
                            }
                            else if ( sn < 0 )
                            {
                                    bug( "Fread_obj: unknown skill.", 0 );
                            }
                            else
                            {
                                    obj->valueByIndex(iValue, sn);
                            }
                            fMatch = true;
                            break;
                    }
 
                    break;

            case 'T':
                    KEY( "TS",          obj->timestamp,         fread_number64( fp ) );
                    KEY( "Timer",        obj->timer,                fread_number( fp ) );
                    KEY( "Time",        obj->timer,                fread_number( fp ) );
                    break;

            case 'V':
                    if ( !strcmp( word, "Values" ) || !strcmp(word,"Vals"))
                    {
                            obj->value0(fread_number( fp ));
                            obj->value1(fread_number( fp ));
                            obj->value2(fread_number( fp ));
                            obj->value3(fread_number( fp ));
                            if (obj->item_type == ITEM_WEAPON && obj->value0() == 0)
                                    obj->value0(obj->pIndexData->value[0]);
                            convert_obj_values( obj );
                            fMatch                = true;
                            break;
                    }

                    if ( !strcmp( word, "Val" ) )
                    {
                            obj->value0(fread_number( fp ));
                            obj->value1(fread_number( fp ));
                            obj->value2(fread_number( fp ));
                            obj->value3(fread_number( fp ));
                            obj->value4(fread_number( fp ));
                            convert_obj_values( obj );
                            fMatch = true;
                            break;
                    }

                    if ( !strcmp( word, "Vnum" ) )
                    {
                            int vnum;

                            vnum = fread_number( fp );
                            if ( ( obj->pIndexData = get_obj_index( vnum ) ) == 0 )
                                    bug( "Fread_obj: bad vnum %d.", vnum );
                            else
                                    fVnum = true;
                            fMatch = true;
                            break;
                    }
                    break;

            case 'W':
                    KEY( "WearFlags",        obj->wear_flags,        fread_number( fp ) );
                    KEY( "WeaF",        obj->wear_flags,        fread_number( fp ) );
                    KEY( "Wear",        wear_loc,                fread_number( fp ) );
                    KEY( "Weight",        obj->weight,                fread_number( fp ) );
                    KEY( "Wt",                obj->weight,                fread_number( fp ) );

                    if (!strcmp( word, "Wearloc" )) {
                        Wearlocation *wearloc = wearlocationManager->find( fread_word( fp ) );
                        wear_loc = wearloc ? wearloc->getIndex() : -1;
                        fMatch = true;
                        break;
                    }

                    break;
            case 'X':
                    if (!strcmp(word, "X")) {
                        DLString key = fread_word(fp);
                        obj->setProperty(key, fread_dlstring(fp));
                        fMatch = true;
                        break;
                    }
                    break;
            }

            if ( !fMatch )
            {
                    bug( "Fread_obj: no match.", 0 );
                    LogStream::sendNotice() << "word:" << word << ":" << endl;
                    fread_to_eol( fp );
            }
        }

    } catch (const FileFormatException &e) {
        fread_obj_uncount_expired( obj, room );
        extract_obj( obj );
        throw e;
    }
}

