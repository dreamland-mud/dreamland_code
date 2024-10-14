#include "commandtemplate.h"
#include "skillgroup.h"
#include "liquid.h"
#include "religion.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "affect.h"
#include "autoflags.h"
#include "affectflags.h"
#include "profflags.h"
#include "fight.h"
#include "mudtags.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

RELIG(none);

/*-----------------------------------------------------------------
 * 'affect' command
 *----------------------------------------------------------------*/
enum {
    FSHOW_LINES = (A),
    FSHOW_TIME = (B),
    FSHOW_COLOR = (C),
    FSHOW_EMPTY = (D),
    FSHOW_RUSSIAN = (E),
};

struct AffectOutput {
    AffectOutput( ) { }
    AffectOutput( Affect *, int flags );
    
    void format_affect( Affect * );
    DLString format_affect_location( Affect * );
    DLString format_affect_bitvector( Affect * );
    DLString format_affect_global( Affect * );
    void show_affect( ostringstream &, int );

    int type;
    int duration;
    DLString name;
    list<DLString> lines;
    bool unitMinutes;
    bool russian;
};

struct ShadowAffectOutput : public AffectOutput {
    ShadowAffectOutput( int, int flags );
};

ShadowAffectOutput::ShadowAffectOutput( int shadow, int flags )
{
    duration = shadow * 4; 
    name = "вторая тень";
    type = -1;
    unitMinutes = false;
}

AffectOutput::AffectOutput( Affect *paf, int flags )
{
    type = paf->type;
    russian = IS_SET(flags, FSHOW_RUSSIAN);
    name = russian ? paf->type->getRussianName( ) : paf->type->getName( );
    duration = paf->duration;
    unitMinutes = true;
}

void AffectOutput::show_affect( ostringstream &buf, int flags )
{
    ostringstream f;
    DLString fmtResult;
    
    if (IS_SET(flags, FSHOW_RUSSIAN))
        f << "{Y%1$-23s{x";
    else
        f << "{Y%1$-18s{x";
    
    if (IS_SET(flags, FSHOW_LINES|FSHOW_TIME)) 
        f << "{y:";
    
    if (IS_SET(flags, FSHOW_LINES))
        for (list<DLString>::iterator l = lines.begin( ); l != lines.end( ); l++) {
            if (l != lines.begin( )) {                
                f << "," << endl;                
                if (IS_SET(flags, FSHOW_RUSSIAN))
                    f << "                        ";
                else
                    f << "                   ";
            }

            f << "{y " << *l;
        }
    
    if (IS_SET(flags, FSHOW_TIME)) {
        if (duration < 0)
            f << " {cпостоянно";
        else
            f << " {yв течение {m%2$d{y "
              << (unitMinutes ? "час%2$Iа|ов|ов" : "секун%2$Iды|д|д");
    }
    
    fmtResult = fmt( 0, f.str( ).c_str( ), name.c_str( ), duration );

    buf << fmtResult << "{x" << endl;
}

void AffectOutput::format_affect( Affect *paf )
{
    DLString l;

    if (!( l = format_affect_location( paf ) ).empty( ))
        lines.push_back( l );

    if (!( l = format_affect_bitvector( paf ) ).empty( ))
        lines.push_back( l );

    if (!( l = format_affect_global( paf ) ).empty( ))
        lines.push_back( l );
}

DLString AffectOutput::format_affect_location( Affect *paf )
{
    DLString buf;
    
    if (paf->location != APPLY_NONE) 
        switch (paf->location) {
        case APPLY_HEAL_GAIN:
        case APPLY_MANA_GAIN:
            if (paf->modifier > 100)
                buf << fmt( 0, "улучшает {m%1$s{y на {m%2$d%%{y",
                               apply_flags.message( paf->location ).c_str( ),
                               paf->modifier - 100 );
            else if (paf->modifier < 100 && paf->modifier > 0)
                buf << fmt( 0, "ухудшает {m%1$s{y на {m%2$d%%{y",
                               apply_flags.message( paf->location ).c_str( ),
                               100 - paf->modifier );
            break;

        case APPLY_BITVECTOR:
            /* do nothing */
            break;

        default:
            if (paf->global.empty()) {
                buf << "изменяет {m" << apply_flags.message( paf->location ) << "{y "
                    << "на {m" << paf->modifier << "{y";
            }
            break;
        }

    return buf;
}

DLString AffectOutput::format_affect_bitvector( Affect *paf )
{
    ostringstream buf;
    const FlagTable *table = paf->bitvector.getTable();
    bitstring_t b = paf->bitvector;
    bool removes = paf->location == APPLY_BITVECTOR && paf->modifier < 0;

    if (table && b) {
        const char *word = 0;
        char gcase = '1';
        DLString ending;

        if (table == &affect_flags) {
            word = removes ? "{1{Rотнимает{2" : "добавляет";
            gcase = '4';
        } else if (table == &imm_flags) {
            word = removes ? "{1{Rотнимает{2 иммунитет к" : "иммунитет к";
        } else if (table == &res_flags) {
            word = removes ? "{1{Rотнимает{2 сопротивляемость к" : "сопротивляемость к";
        } else if (table == &vuln_flags) {
            word = removes ? "отнимает уязвимость к" : "уязвимость к";
        } else if (table == &detect_flags) {
            word = (IS_SET(b, ADET_WEB|ADET_FEAR) ?  
                    (removes ? "отнимает" : "добавляет") : 
                    (removes ? "отнимает обнаружение" : "обнаружение"));
            gcase = (IS_SET(b, ADET_WEB|ADET_FEAR) ? '4': '2');
            
        } else if (table == &form_flags) {
            word = removes ? "отнимает форму" : "добавляет форму";
        } else if (table == &part_flags) {
            word = removes ? "отнимает часть тела" : "добавляет часть тела";
        }
        
        if (word)
            buf << word << " {m" << table->messages( b, true, gcase ) << "{y" << ending;
    }

    return buf.str();
}

DLString AffectOutput::format_affect_global( Affect *paf )
{
    DLString buf;
    const GlobalRegistryBase *registry = paf->global.getRegistry();
    int mod = paf->modifier;

    if (registry) {
        if (registry == skillManager) {
            buf << (mod >= 0 ? "повышает" : "понижает") << " "
                << (paf->location == APPLY_LEARNED ? "владение умением" : "уровень умения")
                << " {m" << paf->global.toRussianString().quote() 
                << "{y на {m" << (int)abs(mod) << "{y";
        } else if (registry == skillGroupManager) {
            buf << (mod >= 0 ? "повышает" : "понижает") << " "
                << (paf->location == APPLY_LEARNED ? "владение группой умений" : "уровень умений группы")
                << " {m" << paf->global.toRussianString().quote() 
                << "{y на {m" << (int)abs(mod) << "{y";
        } else if (registry == liquidManager) {
            buf << "добавляет запах {m" << paf->global.toRussianString('2', ",").colourStrip() << "{x";
        } else if (registry == wearlocationManager) {
            StringList cut;

            for (auto wearlocIndex: paf->global.toArray()) {
                Wearlocation *wearloc = wearlocationManager->find(wearlocIndex);
                cut.push_back(wearloc->getRibName().ruscase('2'));
            }

            buf << "лишает {1{m" << cut.join("{2, {1{m");
        } else {
            buf << "неизвестный вектор";
        }
    }

    return buf;
}

static bool __aff_sort_time__( const AffectOutput &a, const AffectOutput &b )
{
    if (a.unitMinutes == b.unitMinutes)
        return a.duration < b.duration;
    else
        return a.unitMinutes ? b.duration : a.duration;
}

static bool __aff_sort_name__( const AffectOutput &a, const AffectOutput &b )
{
    return a.name < b.name;
}

struct PermanentAffects {
    PermanentAffects(Character *ch) {
        this->ch = ch;
        my_res = ch->res_flags;
        my_vuln = ch->vuln_flags;
        my_imm = ch->imm_flags;
        my_aff = ch->affected_by;
        my_det = ch->detection;
        my_hgain = ch->heal_gain;
        my_mgain = ch->mana_gain;
        my_beats = ch->mod_beats;
    }

    void printAll() const {
        print("У тебя иммунитет к", my_imm, imm_flags, '2');
        print("Ты обладаешь сопротивляемостью к", my_res, imm_flags, '3');
        print("Ты уязвим%1$Gо||а к", my_vuln, imm_flags, '3');
        print("Ты способ%1$Gно|ен|на обнаружить", my_det, detect_flags, '4');
        print("Ты под воздействием", my_aff, affect_flags, '2');

        if (ch->getProfession()->getFlags().isSet(PROF_DIVINE) && ch->getReligion() == god_none)
            ch->pecho("У тебя {rштраф{x на все молитвы, пока ты не выберешь себе {hh1религию{x.");

        if (my_hgain <= -100)
            ch->pecho("Твоё здоровье и шаги не восстанавливаются!");
        else if (my_hgain != 0)
            ch->pecho("Твоё здоровье и шаги восстанавливаются на {%s%d%%{x %s.",
                    my_hgain > 0 ? "C" : "r",
                    abs(my_hgain) , my_hgain > 0 ? "быстрее" : "медленнее");

        if (my_mgain <= -100)
            ch->pecho("Твоя мана не восстанавливается!");
        else if (my_mgain != 0)
            ch->pecho("Твоя мана восстанавливается на {%s%d%%{x %s.",
                       my_mgain > 0 ? "C" : "r",
                       abs(my_mgain), my_mgain > 0 ? "быстрее" : "медленнее");

        if (my_beats != 0)
            ch->pecho("Задержки от всех умений у тебя на {%s%d%%{x %s.",
                       my_beats > 0 ? "r" : "C",
                       abs(my_beats), my_beats > 0 ? "длиннее" : "короче");
    }

    bool isSet() const {
        return my_res || my_vuln || my_imm || my_aff || my_det;
    }

private:
    void print(const char *messagePrefix, const int &my_flags, const FlagTable &my_table, char gcase) const {
        if (my_flags != 0) {
            DLString message = DLString(messagePrefix) + " " + my_table.messages(my_flags, true, gcase) + ".";
            ch->pecho(message.c_str(), ch);
        }
    }

    Character *ch;
    int my_res;
    int my_vuln;
    int my_imm;
    int my_aff;
    int my_det;
    int my_hgain;
    int my_mgain;   
    int my_beats;
};

CMDRUNP( affects )
{
    ostringstream buf;
    list<AffectOutput> output;
    list<AffectOutput>::iterator o;
    int flags = FSHOW_LINES|FSHOW_TIME|FSHOW_COLOR|FSHOW_EMPTY;

    if ((IS_CHARMED(ch) ? ch->master : ch)->getConfig().ruskills)
        SET_BIT(flags, FSHOW_RUSSIAN);
   
    // Keep track of res, vuln, hp/mana gain. 
    PermanentAffects permAff(ch);
 
    for (auto &paf: ch->affected) {
        if (output.empty( ) || output.back( ).type != paf->type) 
            output.push_back( AffectOutput( paf, flags ) );
        
        output.back( ).format_affect( paf );
    }
    
    if (HAS_SHADOW(ch))
        output.push_front( ShadowAffectOutput( ch->getPC( )->shadow, flags ) );

    DLString words = argument;
    for (auto &word: words.split(" ")) {
        if (arg_is( word, "time" ))
            output.sort( __aff_sort_time__ );
        
        if (arg_is( word, "name" ))
            output.sort( __aff_sort_name__ );

        if (arg_is( word, "descend" ))
            output.reverse( );

        if (arg_is( word, "short" ))
            REMOVE_BIT(flags, FSHOW_LINES);

        if (arg_is( word, "nocolor" ))
            REMOVE_BIT(flags, FSHOW_COLOR);

        if (arg_is( word, "noempty" ))
            REMOVE_BIT(flags, FSHOW_EMPTY);
    }

    for (o = output.begin( ); o != output.end( ); o++) 
        o->show_affect( buf, flags );

    // Output skill and level bonuses in the end.
    if (!ch->is_npc()) {
        PCharacter *pch = ch->getPC();
        bool rus = IS_SET(flags, FSHOW_RUSSIAN);
        const DLString joiner = " {yна{m ";
        StringList learnedSkills;
        StringList learnedGroups = pch->mod_skill_groups.toStringList(rus, joiner);
        StringList levelSkills = pch->mod_level_skills.toStringList(rus, joiner);
        StringList levelGroups = pch->mod_level_groups.toStringList(rus, joiner);

        for ( int sn = 0; sn < skillManager->size( ); sn++) {
            Skill *skill = skillManager->find(sn);
            if (skill->available(pch) && pch->mod_skills[sn] != 0) {
                DLString line = rus ? skill->getRussianName() : skill->getName();
                line << joiner << pch->mod_skills[sn];
                learnedSkills.push_back( line );
            }
        }
        if (!learnedSkills.empty())
            buf << "{yИзменено владение умениями: " << learnedSkills.wrap("{m", "%{y").join(", ") << "." << endl;
        if (!learnedGroups.empty())
            buf << "{yИзменено владение группами умений: " << learnedGroups.wrap("{m", "%{y").join(", ") << "." << endl;
        if (pch->mod_skill_all != 0)
            buf << "{yВладение всеми умениями изменено на {m" << pch->mod_skill_all << "%{y.{x" << endl;
        if (!levelSkills.empty())
            buf << "{yИзменен уровень умений: " << levelSkills.wrap("{m", "{y").join(", ") << "." << endl;
        if (!levelGroups.empty())
            buf << "{yИзменен уровень группы умений: " << levelGroups.wrap("{m", "{y").join(", ") << "." << endl;
        if (pch->mod_level_all != 0)
            buf << "{yУровень всех умений изменен на {m" << pch->mod_level_all << "{y.{x" << endl;
        if (pch->mod_level_spell != 0)
            buf << "{yУровень всех заклинаний изменен на {m" << pch->mod_level_spell << "{y.{x" << endl;
    }

    if (IS_CHARMED(ch)) {
        if (buf.str( ).empty( )) 
            oldact("$C1 не находится под действием каких-либо аффектов.", ch->master, 0, ch, TO_CHAR);
        else 
            oldact("$C1 находится под действием следующих аффектов:", ch->master, 0, ch, TO_CHAR);
        buf << "{x";
        ch->master->send_to( buf );
        return;
    }

    // Output permanent bits on top.
    permAff.printAll();

    if (buf.str( ).empty( )) {
        if (IS_SET(flags, FSHOW_EMPTY) && !permAff.isSet())
            ch->pecho( "Ты не находишься под действием каких-либо аффектов." );
    } 
    else {
        if (permAff.isSet())
            ch->pecho("");
        ch->pecho( "Ты находишься под действием следующих аффектов:" );
        buf << "{x";

        if (!IS_SET(flags, FSHOW_COLOR)) {
            ostringstream showbuf;
            mudtags_convert( buf.str( ).c_str( ), showbuf, TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR, ch );
            ch->send_to( showbuf );
        }
        else
            ch->send_to( buf );
    }
}
