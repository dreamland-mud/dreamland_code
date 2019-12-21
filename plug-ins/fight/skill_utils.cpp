#include "skill_utils.h"
#include "skillreference.h"
#include "skillgroup.h"
#include "spell.h"
#include "affect.h"
#include "pcharacter.h"
#include "calendar_utils.h"
#include "skill.h"
#include "affectflags.h"
#include "merc.h"
#include "mercdb.h"

GROUP(none);
GSN(turlok_fury);

bool temporary_skill_active( const Skill *skill, Character *ch )
{
    if (ch->is_npc())
        return false;
    
    PCSkillData &data = ch->getPC()->getSkillData(skill->getIndex());
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

void print_see_also(const Skill *skill, PCharacter *ch, ostream &buf) 
{
    SkillGroupReference &group = (const_cast<Skill *>(skill))->getGroup( );

    // 'См. также справка|help травы|herbs' - с гипер-ссылкой на справку.
    // '... группаум|glist maladiction|проклятия' - с гипер-ссылкой на команду
    buf << endl << "См. также {W{lRсправка{lEhelp{lx {hh" << skill->getNameFor(ch) << "{x";
    if (group != group_none)
       buf << " и команду {y{hc{lRгруппаум{lEglist{lx " << group->getNameFor(ch) << "{x";
    buf << "." << endl;
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
    int slevel = ch->getModifyLevel();

    if (!ch->is_npc()) {
        slevel += ch->getPC()->mod_level_groups[skill.getGroup()];
        slevel += ch->getPC()->mod_level_skills[skill.getIndex()];
        slevel += ch->getPC()->mod_level_all;
    }

    if (ch->isAffected(gsn_turlok_fury) && chance(50)) {
        slevel += number_range(1, 5);
    }
    
    if (ch->is_immortal() && slevel != ch->getModifyLevel()) 
        ch->printf("Отладка: уровень умения %s %d -> %d.\r\n", 
                    skill.getName().c_str(), ch->getModifyLevel(), slevel);

    return slevel;
}
