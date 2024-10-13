#include <string.h>

#include "skill_utils.h"
#include "skillreference.h"
#include "skillgroup.h"
#include "spell.h"
#include "religion.h"
#include "affect.h"
#include "pcharacter.h"
#include "core/object.h"
#include "calendar_utils.h"
#include "skill.h"
#include "affectflags.h"
#include "damageflags.h"
#include "dreamland.h"
#include "act.h"
#include "merc.h"


const char SKILL_HEADER_BG = 'W';
const char SKILL_HEADER_FG = 'Y';
const char *SKILL_INFO_PAD = "  {W*{x ";

GROUP(none);
GROUP(fightmaster);
GROUP(defensive);
GSN(none);
GSN(turlok_fury);
GSN(athena_wisdom);
GSN(scrolls);
GSN(staves);
GSN(wands);
RELIG(none);

bool temporary_skill_active( const Skill *skill, CharacterMemoryInterface *mem )
{
    if (!mem->getPCM())
        return false;
    
    PCSkillData &data = mem->getPCM()->getSkills().get(skill->getIndex());
    return temporary_skill_active(data);
}

bool temporary_skill_active( const PCSkillData &data )
{
    if (!data.isTemporary())
        return false;

    long today = day_of_epoch(time_info);
    long start = data.start.getValue();
    if (start > today)
        return false;

    long end = data.end.getValue();
    if (end == PCSkillData::END_NEVER)
        return true;

    return today <= end;
}

DLString spell_utterance(Skill *skill)
{
    char buf  [MAX_STRING_LENGTH];
    char spellName [MAX_STRING_LENGTH];
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
        const char *        old;
        const char *        _new;
    };

    static const struct syl_type syl_table[] =
    {
        { " ",                " "                },
        { "ar",                "abra"                },
        { "au",                "kada"                },
        { "bless",        "fido"                },
        { "blind",        "nose"                },
        { "bur",        "mosa"                },
        { "cu",                "judi"                },
        { "de",                "oculo"                },
        { "en",                "unso"                },
        { "light",        "dies"                },
        { "lo",                "hi"                    },
        { "mor",        "zak"                },
        { "move",        "sido"                },
        { "ness",        "lacri"                },
        { "ning",        "illa"                },
        { "per",        "duda"                },
        { "ra",                "gru"                },
        { "fresh",        "ima"                },
        { "re",                "candus"        },
        { "son",        "sabru"                },
        { "tect",        "infra"                },
        { "tri",        "cula"                },
        { "ven",        "nofo"                },
        { "ust",        "lon"           },
        { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
        { "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
        { "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
        { "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
        { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
        { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
        { "y", "l" }, { "z", "k" },
        { "", "" }
    };

    buf[0]        = '\0';
    strcpy( spellName, skill->getName( ).c_str( ) );

    for ( pName = spellName; *pName != '\0'; pName += length ) {
        for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ ) {
            if ( !str_prefix( syl_table[iSyl].old, pName ) ) {
                strcat( buf, syl_table[iSyl]._new );
                break;
            }
        }

        if ( length == 0 )
            length = 1;
    }

    return buf;
}

Skill * skillref_to_pointer(const XMLSkillReference &ref)
{
    Skill *skill = const_cast<Skill *>(const_cast<XMLSkillReference *>(&ref)->getElement());
    return skill;
}

char skill_learned_colour(const Skill *skill, PCharacter *ch)
{
    int percent = ch->getSkillData(
                        skill->getIndex()).learned;

    if (percent == 1)
        return 'R';
    if (percent >= skill->getMaximum( ch ))
        return 'C';
    if (percent > skill->getAdept( ch ))
        return 'c';
    return 'x';
}

DLString print_names_for(const Skill *skill, Character *ch)
{
    bool rus = ch->getConfig( ).ruskills;
    const char *format = "'{%c%N1{%c' или '{%c%N1{%c'";

    if (rus)
        return fmt(0, format, 
            SKILL_HEADER_FG, skill->getRussianName().c_str(), SKILL_HEADER_BG, 
            SKILL_HEADER_FG, skill->getName().c_str(), SKILL_HEADER_BG);
    else
        return fmt(0, format, 
            SKILL_HEADER_FG, skill->getName().c_str(), SKILL_HEADER_BG, 
            SKILL_HEADER_FG, skill->getRussianName().c_str(), SKILL_HEADER_BG);
}

DLString print_what(const Skill *skill)
{
    ostringstream buf;

    buf << "{" << SKILL_HEADER_BG
        << skill_what(skill).ruscase('1').upperFirstCharacter();

    return buf.str();
}

DLString print_group_for(const Skill *skill, Character *ch)
{
    vector<int> groups = const_cast<Skill *>(skill)->getGroups().toArray();    
    if (groups.empty())
        return DLString::emptyString;

    ostringstream buf;
    buf << "{" << SKILL_HEADER_BG << ", входит в групп"
        << (groups.size() == 1 ? "у" : "ы")
        << " ";
        
    for (unsigned int g = 0; g < groups.size(); g++) {
        if (g > 0)
            buf << ", ";

        buf << "'{hg{" << SKILL_HEADER_FG 
            << skillGroupManager->find(groups[g])->getNameFor(ch) << "{hx"
            << "{" << SKILL_HEADER_BG << "'{x";
    }

    return buf.str();
}

bool skill_is_spell(const Skill *skill)
{
    return skill->getSpell( ) && skill->getSpell( )->isCasted( );
}
 
DLString skill_what(const Skill *skill)
{
    return skill_is_spell(skill) ? "заклинани|е|я|ю|е|ем|и" : "умени|е|я|ю|е|ем|и";
} 

DLString skill_what_plural(const Skill *skill)
{
    return skill_is_spell(skill) ? "заклинания" : "умения";
}
 
int skill_learned_from_affects(const Skill *skill, PCharacter *ch)
{    
    int learned = 0;

    for (auto group: const_cast<Skill *>(skill)->getGroups().toArray())
        learned += ch->mod_skill_groups[group];

    learned += ch->mod_skills[skill->getIndex()];
    learned += ch->mod_skill_all;
    return learned;
}

DLString skill_effective_bonus(const Skill *skill, PCharacter *ch)
{
    int learned = ch->getSkillData(skill->getIndex()).learned;
    int eff = skill->getEffective(ch);

    if (eff == learned)
        return DLString::emptyString;

    ostringstream buf;
    buf << ", работает на {C" << eff << "%{x";
    return buf.str();
}

int skill_level(Skill &skill, Character *ch)
{
    return skill_level_bonus(skill, ch) + ch->getModifyLevel();
}

int skill_level_bonus(Skill &skill, Character *ch)
{
    int slevel = 0;

    if (skill.getIndex() == gsn_none)
        return slevel;

    if (!ch->is_npc()) {
        for (auto group: skill.getGroups().toArray())
            slevel += ch->getPC()->mod_level_groups[group];

        slevel += ch->getPC()->mod_level_skills[skill.getIndex()];
        slevel += ch->getPC()->mod_level_all;

        if (skill.getSpell() && skill.getSpell()->isCasted())
            slevel += ch->getPC()->mod_level_spell;
    }

    if (skill.hasGroup(group_fightmaster) && ch->isAffected(gsn_turlok_fury) && chance(50)) {
        slevel += number_range(1, 5);
    }

    if (skill.hasGroup(group_defensive) && ch->isAffected(gsn_athena_wisdom) && chance(50)) {
        slevel += number_range(1, 5);
    }
    
    return slevel;
}

// Returns which skill governs the usage of a given item, e.g. 'scrolls' for ITEM_SCROLL.
static Skill & skill_for_item_type(Object *obj)
{
    switch (obj->item_type) {
        case ITEM_SCROLL: return *gsn_scrolls;
        case ITEM_WAND:   return *gsn_wands;
        case ITEM_STAFF:  return *gsn_staves;
        default:          return *gsn_none;
    }
}

short get_wear_level( Character *ch, Object *obj ) 
{
    int wear_mod, level_diff, itemtype_bonus;
    
    wear_mod = ch->getProfession( )->getWearModifier( obj->item_type );
    level_diff = ch->getModifyLevel( ) - ch->getRealLevel( );
    itemtype_bonus = skill_level_bonus(
        skill_for_item_type(obj), ch
    );
            
    return max( 1, obj->level - wear_mod - level_diff - itemtype_bonus );
}

int skill_lookup(const DLString &constName, Character *ch)
{
    int i;
    const Skill *skill;
    DLString name = constName.toLower().quote().getOneArgument(); // support skill name like 'cure p' with explicit quotes

    if (name.empty( ))
        return -1;
    
    // Strict lookup: always return an exact match available for ch.
    for (i = 0; i < skillManager->size(); i++) {
        skill = skillManager->find(i);
        if (skill->getName() == name || skill->getRussianName() == name) {
            if (!ch || skill->available(ch))
                return i;
        }
    }

    // Unstrict lookup by partial name , e.g. 'cur po' for 'cure poison'.
    StringList argList(name);

    for (i = 0; i < skillManager->size(); i++) {
        skill = skillManager->find(i);

        if (StringList(skill->getName()).superListOf(argList)
             || StringList(skill->getRussianName()).superListOf(argList))
        {
            if (!ch || skill->available(ch))
                return i;
        }
    }

    // Nothing found for ch, try again without specifying target character.
    if (ch)
        return skill_lookup(constName, 0);

    return -1;
}
