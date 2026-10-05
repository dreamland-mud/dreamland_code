/* $Id$
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "xmlmobilefactory.h"
#include "grammar_entities_impl.h"
#include "json_utils_ext.h"
#include "skillgroup.h"
#include "religion.h"
#include "behavior.h"
#include "race.h"
#include "string_utils.h"
#include "mobbody.h"
#include "stringlist.h"
#include "merc.h"
#include "def.h"

int XMLMobileFactory::ignoredNumbers = 0;

/* <reviewed> names -> mask over Body::BitSet; only the five gated sets count. */
static int reviewed_mask(const DLString &names)
{
    int mask = 0;
    StringList words(names);
    for (auto &w: words)
        for (int k = Body::BS_AFF; k < Body::BS_MAX; k++)
            if (w == Body::bitSetNames[k])
                mask |= 1 << k;
    return mask;
}

static DLString reviewed_names(int mask)
{
    DLString buf;
    for (int k = Body::BS_AFF; k < Body::BS_MAX; k++)
        if (mask & (1 << k)) {
            if (!buf.empty())
                buf << " ";
            buf << Body::bitSetNames[k];
        }
    return buf;
}

XMLMobileFactory::XMLMobileFactory( ) : 
                          act(&act_flags),
                          aff(&affect_flags),
                          dam_type(DAMW_NONE, &weapon_flags),
                          off(&off_flags),
                          imm(&imm_flags),
                          res(&res_flags),
                          vuln(&vuln_flags),
                          start_pos(POS_STANDING, &position_table),
                          default_pos(POS_STANDING, &position_table),
                          sex(SEX_NEUTRAL, &sex_table),
                          form(&form_flags),
                          parts(&part_flags),
                          size(NO_FLAG, &size_table),
                          detection(&detect_flags),
                          practicer(skillGroupManager),
                          religion(religionManager),
                          affects(skillManager),
                          behaviors(behaviorManager)
{
}

// True if p (pointing at '(') opens a (keyword) group: an ASCII letter inside, or a
// colour code {X followed by a letter. Bounds-safe -- the old inline test read
// *(p+3) on a string ending "({". KOI8 Cyrillic bytes are not alpha in the C locale,
// so a Cyrillic "(prose)" is deliberately NOT a keyword group and keeps its parens.
static bool is_keyword_paren(const char *p)
{
    if (*p != '(')
        return false;
    if (isalpha(*(p+1)))
        return true;
    if (*(p+1) == '{' && *(p+2) != '\0' && isalpha(*(p+3)))
        return true;
    return false;
}

DLString format_longdescr(const DLString &longdescr)
{
    const char *descr = longdescr.c_str();
    // Remove (keywords) and 1 preceding space.
    ostringstream buf;
    DLString hint;
    bool skipChar = false;

    for (const char *d = descr; *d; d++) {
        // Eat the space before a ( ONLY when it opens a keyword group we strip;
        // otherwise (e.g. Cyrillic prose "слово (текст)") the space must stay.
        if (*d == ' ' && is_keyword_paren(d+1))
            continue;
        // Start ignoring everything after a ( bracket.
        if (is_keyword_paren(d)) {
            skipChar = true;
            continue;
        }
        // Stop ignoring once bracket is closed.
        if (*d == ')' && skipChar) {
            skipChar = false;
            continue;
        }
        // Skip everything while inside the brackets.
        if (skipChar) {
            hint << *d;
            continue;
        }

        // Normal output outside of brackets. 
        buf << *d;
    }

    DLString result = String::stripEOL(buf.str());
    result.stripRightWhiteSpace();
    return result;
}

void
XMLMobileFactory::init(const mob_index_data *mob)
{
    const mob_index_data *m = mob;
    bitstring_t add, del;

    keyword = mob->keyword;
    short_descr = mob->short_descr;
    long_descr = mob->long_descr;
    description = mob->description;
    
    smell = mob->smell;

    race.setValue(mob->race);
    m->bodyDiff(MOBSET_ACT, add, del);
    act.setAddDel(add, del);
    m->bodyDiff(MOBSET_AFF, add, del);
    aff.setAddDel(add, del);
    alignment.setValue(mob->alignment);
    group.setValue(mob->group);
    level.setValue(mob->level);
    hitroll.setValue(mob->hitroll);
    hit.set(mob->hit[DICE_NUMBER], mob->hit[DICE_TYPE], mob->hit[DICE_BONUS]);
    mana.set(mob->mana[DICE_NUMBER], mob->mana[DICE_TYPE], mob->mana[DICE_BONUS]);
    damage.set(mob->damage[DICE_NUMBER], mob->damage[DICE_TYPE], mob->damage[DICE_BONUS]);
    dam_type.setValue(mob->dam_type);
    
    ac.pierce = mob->ac[AC_PIERCE] / 10;
    ac.bash = mob->ac[AC_BASH] / 10;
    ac.slash = mob->ac[AC_SLASH] / 10;
    ac.exotic = mob->ac[AC_EXOTIC] / 10;

    m->bodyDiff(MOBSET_OFF, add, del);
    off.setAddDel(add, del);
    m->bodyDiff(MOBSET_IMM, add, del);
    imm.setAddDel(add, del);
    m->bodyDiff(MOBSET_RES, add, del);
    res.setAddDel(add, del);
    m->bodyDiff(MOBSET_VULN, add, del);
    vuln.setAddDel(add, del);

    start_pos.setValue(mob->start_pos);
    default_pos.setValue(mob->default_pos);

    sex.setValue(mob->sex);
    wealth.setValue(mob->wealth);
    m->bodyDiff(MOBSET_FORM, add, del);
    form.setAddDel(add, del);
    m->bodyDiff(MOBSET_PARTS, add, del);
    parts.setAddDel(add, del);

    size.setValue(mob->size);
    material.setValue(mob->material);

    m->bodyDiff(MOBSET_DET, add, del);
    detection.setAddDel(add, del);

    const char *c = 0;
    
    if(mob->spec_fun.func)
        c = spec_name(mob->spec_fun.func);

    if(!c)
        c = mob->spec_fun.name.c_str();

    if(!c)
        c = "";

    spec.setValue(c);
    practicer.set(mob->practicer);
    religion.set(mob->religion);
    affects.set(mob->affects);
    behaviors.set(mob->behaviors);
    
    if (mob->gram_number != Grammar::Number::SINGULAR)
        gram_number.setValue(mob->gram_number.toString());

    clan.assign(mob->clan);

    if(!mob->behavior.isEmpty( ))
        behavior.setNode(mob->behavior->getFirstNode( ));

    JsonUtils::copy(props, mob->props);

    // props.olc.*Confirmed are retired (decision 12): never written again.
    Json::Value &olc = props["olc"];
    if (olc.isObject()) {
        for (auto &key: olc.getMemberNames())
            if (key.size() > 9 && key.compare(key.size() - 9, 9, "Confirmed") == 0)
                olc.removeMember(key);
        if (olc.empty())
            props.removeMember("olc");
    } else if (olc.isNull())
        props.removeMember("olc");

    tier.setValue(mob->tierName);
    tierStyle.setValue(mob->tierStyle);
    reviewed.setValue(reviewed_names(mob->reviewed));

    // Tier-derived numbers are never written back (plan §3.6 item 6).
    hit.omit = mana.omit = damage.omit = ac.omit = mob->numbersDerived;
    if (mob->numbersDerived) {
        hitroll.setValue(0);
        wealth.setValue(0);
    }
}

mob_index_data *
XMLMobileFactory::compat( )
{
    MOB_INDEX_DATA *mob = new MOB_INDEX_DATA;

    compat(mob);

    return mob;
}

void
XMLMobileFactory::compat(mob_index_data *mob)
{
    Race *mobrace = raceManager->find(race.getValue( ).c_str( ));

    if (!player_name.empty())
        mob->keyword.fromMixedString(player_name);
    else
        mob->keyword = keyword;

    mob->short_descr = short_descr;
    mob->long_descr = long_descr;
    mob->long_descr[RU] = format_longdescr(long_descr[RU]);
    mob->description = description;
    mob->smell = smell;

    mob->race = mobrace->getName();
    mob->alignment = alignment.getValue( );
    mob->group = group.getValue( );
    mob->level = level.getValue( );
    mob->hitroll = hitroll.getValue( );
    mob->hit[DICE_NUMBER] = hit.number;
    mob->hit[DICE_TYPE] = hit.type;
    mob->hit[DICE_BONUS] = hit.bonus;
    mob->mana[DICE_NUMBER] = mana.number;
    mob->mana[DICE_TYPE] = mana.type;
    mob->mana[DICE_BONUS] = mana.bonus;
    mob->damage[DICE_NUMBER] = damage.number;
    mob->damage[DICE_TYPE] = damage.type;
    mob->damage[DICE_BONUS] = damage.bonus;
    mob->dam_type = dam_type.getValue( );
    
    mob->ac[AC_PIERCE] = ac.pierce * 10;
    mob->ac[AC_BASH] = ac.bash * 10;
    mob->ac[AC_SLASH] = ac.slash * 10;
    mob->ac[AC_EXOTIC] = ac.exotic * 10;


    mob->start_pos = start_pos.getValue( );
    mob->default_pos = default_pos.getValue( );

    mob->sex = sex.getValue( );
    mob->wealth = wealth.getValue( );

    mob->size = size.getValue( );
    mob->material = material;


    if(!spec.getValue( ).empty( )) {
        mob->spec_fun.name = spec.getValue( );
        mob->spec_fun.func = spec_lookup(mob->spec_fun.name.c_str( ));

        if(!mob->spec_fun.func) {
            LogStream::sendError() 
                << "special " << mob->spec_fun.name
                << " not found." << endl;
        }
    }

    mob->practicer.set(practicer);
    mob->religion.set(religion);
    mob->affects.set(affects);
    mob->behaviors.set(behaviors);
    
    if(!gram_number.getValue( ).empty( ))
        mob->gram_number = Grammar::Number(gram_number.getValue( ).c_str( ));
 
    mob->clan = clan;
    
    if(behavior.getNode( )) {
        mob->behavior.construct( );
        XMLNode::Pointer p = behavior.getNode( );
        mob->behavior->appendChild(p);
    }

    JsonUtils::copy(mob->props, props);

    // Mob reform: authored diffs, tier and reviewed sets. The body and the
    // numbers are built by the caller once the vnum is known
    // (resolveBody + deriveNumbers).
    const XMLFlagsDiff *sets[MOBSET_MAX] = { &act, &off, &aff, &detection, &imm, &res, &vuln, &form, &parts };
    for (int s = 0; s < MOBSET_MAX; s++) {
        mob->bodyAdd[s] = sets[s]->add;
        mob->bodyDel[s] = sets[s]->del;
    }
    mob->tierName = tier.getValue();
    mob->tierStyle = tierStyle.getValue();
    mob->reviewed = reviewed_mask(reviewed.getValue());

    // Authored numbers lose to the tier curve once mob_tiers.json is loaded
    // (decision 8). Counted here, logged once per area by the loader.
    if (MobBody::tiers().loaded
        && (hit.number || hit.type || hit.bonus || mana.number || mana.type || mana.bonus
            || damage.number || damage.type || damage.bonus || hitroll.getValue() || wealth.getValue()
            || ac.pierce || ac.bash || ac.slash || ac.exotic))
        ignoredNumbers++;
}
