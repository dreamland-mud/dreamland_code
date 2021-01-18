#include "skill_utils.h"
#include "skillreference.h"
#include "skillgroup.h"
#include "spell.h"
#include "affect.h"
#include "pcharacter.h"
#include "calendar_utils.h"
#include "skill.h"
#include "affectflags.h"
#include "damageflags.h"
#include "dreamland.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"

const char SKILL_HEADER_BG = 'W';
const char SKILL_HEADER_FG = 'Y';
const char *SKILL_INFO_PAD = "  {W*{x ";

GROUP(none);
GROUP(fightmaster);
GROUP(defensive);
GSN(turlok_fury);
GSN(athena_wisdom);

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
    const char *format = "'{%c%s{%c' или '{%c%s{%c'";

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
    ostringstream buf;
    SkillGroupReference &group = const_cast<Skill *>(skill)->getGroup();

    if (group != group_none)
        buf << "{" << SKILL_HEADER_BG << ", входит в группу "
            << "'{hg{" << SKILL_HEADER_FG << group->getNameFor(ch) << "{hx"
            << "{" << SKILL_HEADER_BG << "'{x";

    return buf.str();
}

DLString print_wait_and_mana(const Skill *skill, Character *ch)
{
    const char *pad = SKILL_INFO_PAD;
    bool empty = true; // Contains any meaningful output besides padding and new lines?
    ostringstream buf;

    buf << pad;

    int beat = skill->getBeats() / dreamland->getPulsePerSecond();
    if (beat > 0) {
        buf << fmt(0, "Задержка при выполнении {W%1$d{x секунд%1$Iу|ы|. ", beat);
        empty = false;
    }

    Spell::Pointer spell = skill->getSpell();
    int mana = (ch && spell && spell->isCasted( )) ? spell->getManaCost(ch) : skill->getMana();
    if (mana > 0) {
        buf << fmt(0, "Расход маны {W%d{x. ", mana);
        empty = false;
    }

    if (spell && spell->isCasted()) { 
        buf << "Тип заклинания" << " {W" << spell_types.message(spell->getSpellType()) << "{x.";
        empty = false;
    }

    if (!empty)
        buf << endl;
    
    if (spell && spell->isCasted() && spell->getTarget() != 0) {
        buf << pad << "Целью служит {W" << target_table.messages(spell->getTarget(), true) << "{x. " << endl;
        empty = false;
    }

    // TODO: expose spell position and show it here.
    // TODO: show if it's ranged or not.

    if (empty) 
        return DLString::emptyString;

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
    int sn = skill->getIndex();
    int gsn = const_cast<Skill *>(skill)->getGroup()->getIndex();

    return ch->mod_skills[sn] + ch->mod_skill_groups[gsn];
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

    if (!ch->is_npc()) {
        slevel += ch->getPC()->mod_level_groups[skill.getGroup()];
        slevel += ch->getPC()->mod_level_skills[skill.getIndex()];
        slevel += ch->getPC()->mod_level_all;

        if (skill.getSpell() && skill.getSpell()->isCasted())
            slevel += ch->getPC()->mod_level_spell;
    }

    if (skill.getGroup() == group_fightmaster && ch->isAffected(gsn_turlok_fury) && chance(50)) {
        slevel += number_range(1, 5);
    }

    if (skill.getGroup() == group_defensive && ch->isAffected(gsn_athena_wisdom) && chance(50)) {
        slevel += number_range(1, 5);
    }
    
    return slevel;
}
