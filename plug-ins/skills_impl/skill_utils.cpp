#include "skill_utils.h"
#include "pcharacter.h"
#include "calendar_utils.h"
#include "skill.h"
#include "merc.h"
#include "mercdb.h"

bool temporary_skill_active( const Skill *skill, Character *ch )
{
    if (ch->is_npc())
        return false;
    
    PCSkillData &data = ch->getPC()->getSkillData(skill->getIndex());
    return temporary_skill_active(data);
}

bool temporary_skill_active( const PCSkillData &data )
{
    if (!data.temporary)
        return false;

    long today = day_of_epoch(time_info);
    long start = data.start.getValue();
    long end = data.end.getValue();
    return (start <= today && today <= end);
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
