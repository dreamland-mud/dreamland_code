#include "commandtemplate.h"
#include "skillgroup.h"
#include "liquid.h"
#include "religion.h"
#include "pcharacter.h"
#include "player_utils.h"
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
#include "l10n.h"

RELIG(none);

/*-----------------------------------------------------------------
 * 'affect' command
 *----------------------------------------------------------------*/
enum {
    FSHOW_LINES = (A),
    FSHOW_TIME = (B),
    FSHOW_COLOR = (C),
    FSHOW_EMPTY = (D),
};

struct AffectOutput {
    AffectOutput( ) : unitMinutes(false), viewer(0), lang(LANG_DEFAULT) { }
    AffectOutput( Affect *, Character *viewer );

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
    Character *viewer;
    lang_t lang;
};

struct ShadowAffectOutput : public AffectOutput {
    ShadowAffectOutput( int, Character *viewer );
};

ShadowAffectOutput::ShadowAffectOutput( int shadow, Character *viewer )
{
    this->viewer = viewer;
    lang = Player::displayLang( viewer );
    duration = shadow * 4;
    name = _("вторая тень").getMessage( lang );
    type = -1;
    unitMinutes = false;
}

AffectOutput::AffectOutput( Affect *paf, Character *viewer )
{
    this->viewer = viewer;
    lang = Player::displayLang( viewer );
    type = paf->type;
    name = paf->type->getNameFor( viewer );
    duration = paf->duration;
    unitMinutes = true;
}

void AffectOutput::show_affect( ostringstream &buf, int flags )
{
    ostringstream f;
    DLString fmtResult;
    
    if (lang == LANG_EN)
        f << "{Y%1$-18s{x";
    else
        f << "{Y%1$-23s{x";

    if (IS_SET(flags, FSHOW_LINES|FSHOW_TIME))
        f << "{y:";

    if (IS_SET(flags, FSHOW_LINES))
        for (list<DLString>::iterator l = lines.begin( ); l != lines.end( ); l++) {
            if (l != lines.begin( )) {
                f << "," << endl;
                if (lang == LANG_EN)
                    f << "                   ";
                else
                    f << "                        ";
            }

            f << "{y " << *l;
        }

    if (IS_SET(flags, FSHOW_TIME)) {
        if (duration < 0)
            f << " {c" << _("постоянно").getMessage( lang );
        else
            f << " {y" << _("в течение {m%2$d{y ").getMessage( lang )
              << (unitMinutes ? _("час%2$Iа|ов|ов").getMessage( lang ) : _("секун%2$Iды|д|д").getMessage( lang ));
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
                buf << fmt( 0, _("улучшает {m%1$s{y на {m%2$d%%{y").getMessage( lang ).c_str( ),
                               apply_flags.message( paf->location, '1', lang ).c_str( ),
                               paf->modifier - 100 );
            else if (paf->modifier < 100 && paf->modifier > 0)
                buf << fmt( 0, _("ухудшает {m%1$s{y на {m%2$d%%{y").getMessage( lang ).c_str( ),
                               apply_flags.message( paf->location, '1', lang ).c_str( ),
                               100 - paf->modifier );
            break;

        case APPLY_BITVECTOR:
            /* do nothing */
            break;

        default:
            if (paf->global.empty()) {
                buf << fmt( 0, _("изменяет {m%1$s{y на {m%2$d{y").getMessage( lang ).c_str( ),
                               apply_flags.message( paf->location, '1', lang ).c_str( ),
                               paf->modifier );
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
        DLString word;
        char gcase = '1';
        DLString ending;

        if (table == &affect_flags) {
            word = (removes ? _("{1{Rотнимает{2") : _("добавляет")).getMessage( lang );
            gcase = '4';
        } else if (table == &imm_flags) {
            word = (removes ? _("{1{Rотнимает{2 иммунитет к") : _("иммунитет к")).getMessage( lang );
        } else if (table == &res_flags) {
            word = (removes ? _("{1{Rотнимает{2 сопротивляемость к") : _("сопротивляемость к")).getMessage( lang );
        } else if (table == &vuln_flags) {
            word = (removes ? _("отнимает уязвимость к") : _("уязвимость к")).getMessage( lang );
        } else if (table == &detect_flags) {
            if (IS_SET(b, ADET_WEB|ADET_FEAR)) {
                word = (removes ? _("отнимает") : _("добавляет")).getMessage( lang );
                gcase = '4';
            } else {
                word = (removes ? _("отнимает обнаружение") : _("обнаружение")).getMessage( lang );
                gcase = '2';
            }
        } else if (table == &form_flags) {
            word = (removes ? _("отнимает форму") : _("добавляет форму")).getMessage( lang );
        } else if (table == &part_flags) {
            word = (removes ? _("отнимает часть тела") : _("добавляет часть тела")).getMessage( lang );
        }

        if (!word.empty())
            buf << word << " {m" << table->messages( b, true, gcase, lang ) << "{y" << ending;
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
            buf << (mod >= 0 ? _("повышает") : _("понижает")).getMessage( lang ) << " "
                << (paf->location == APPLY_LEARNED ? _("владение умением") : _("уровень умения")).getMessage( lang )
                << " {m" << paf->global.toString(lang).quote()
                << "{y " << _("на").getMessage( lang ) << " {m" << (int)abs(mod) << "{y";
        } else if (registry == skillGroupManager) {
            buf << (mod >= 0 ? _("повышает") : _("понижает")).getMessage( lang ) << " "
                << (paf->location == APPLY_LEARNED ? _("владение группой умений") : _("уровень умений группы")).getMessage( lang )
                << " {m" << paf->global.toString(lang).quote()
                << "{y " << _("на").getMessage( lang ) << " {m" << (int)abs(mod) << "{y";
        } else if (registry == liquidManager) {
            buf << _("добавляет запах").getMessage( lang ) << " {m" << paf->global.toString(lang, '2', ",").colourStrip() << "{x";
        } else if (registry == wearlocationManager) {
            StringList cut;

            for (auto wearlocIndex: paf->global.toArray()) {
                Wearlocation *wearloc = wearlocationManager->find(wearlocIndex);
                cut.push_back(wearloc->getRibName().ruscase('2'));
            }

            buf << _("лишает").getMessage( lang ) << " {1{m" << cut.join("{2, {1{m");
        } else {
            buf << _("неизвестный вектор").getMessage( lang );
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
        print(_("У тебя иммунитет к"), my_imm, imm_flags, '2');
        print(_("Ты обладаешь сопротивляемостью к"), my_res, imm_flags, '3');
        print(_("Ты уязвим%1$Gо||а к"), my_vuln, imm_flags, '3');
        print(_("Ты способ%1$Gно|ен|на обнаружить"), my_det, detect_flags, '4');
        print(_("Ты под воздействием"), my_aff, affect_flags, '2');

        if (ch->getProfession()->getFlags().isSet(PROF_DIVINE) && ch->getReligion() == god_none)
            ch->pecho(_("У тебя {rштраф{x на все молитвы, пока ты не выберешь себе {hh1религию{x."));

        if (my_hgain <= -100)
            ch->pecho(_("Твоё здоровье и шаги не восстанавливаются!"));
        else if (my_hgain != 0)
            ch->pecho(_("Твоё здоровье и шаги восстанавливаются на {%s%d%%{x %s."),
                    my_hgain > 0 ? "C" : "r",
                    abs(my_hgain), (my_hgain > 0 ? _("быстрее") : _("медленнее")).getMessage(Player::displayLang(ch)).c_str());

        if (my_mgain <= -100)
            ch->pecho(_("Твоя мана не восстанавливается!"));
        else if (my_mgain != 0)
            ch->pecho(_("Твоя мана восстанавливается на {%s%d%%{x %s."),
                       my_mgain > 0 ? "C" : "r",
                       abs(my_mgain), (my_mgain > 0 ? _("быстрее") : _("медленнее")).getMessage(Player::displayLang(ch)).c_str());

        if (my_beats != 0)
            ch->pecho(_("Задержки от всех умений у тебя на {%s%d%%{x %s."),
                       my_beats > 0 ? "r" : "C",
                       abs(my_beats), (my_beats > 0 ? _("длиннее") : _("короче")).getMessage(Player::displayLang(ch)).c_str());
    }

    bool isSet() const {
        return my_res || my_vuln || my_imm || my_aff || my_det;
    }

private:
    void print(const MultiMessage &prefix, const int &my_flags, const FlagTable &my_table, char gcase) const {
        if (my_flags == 0)
            return;

        lang_t lang = Player::displayLang(ch);
        StringList names = my_table.toStringList(my_flags, gcase, lang);
        DLString message = prefix.getMessage(lang) + " " + names.wrap("{Y", "{x").join(", ") + ".";
        ch->pecho(message.c_str(), ch);
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

    // The display language follows the charmer when the char is charmed.
    Character *viewer = IS_CHARMED(ch) ? ch->master : ch;

    // Keep track of res, vuln, hp/mana gain.
    PermanentAffects permAff(ch);

    for (auto &paf: ch->affected) {
        if (output.empty( ) || output.back( ).type != paf->type)
            output.push_back( AffectOutput( paf, viewer ) );

        output.back( ).format_affect( paf );
    }

    if (HAS_SHADOW(ch))
        output.push_front( ShadowAffectOutput( ch->getPC( )->shadow, viewer ) );

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
        lang_t lang = Player::displayLang(viewer);
        bool rus = (lang != LANG_EN);
        const DLString joiner = " {yна{m ";
        StringList learnedSkills;
        StringList learnedGroups = pch->mod_skill_groups.toStringList(rus, joiner);
        StringList levelSkills = pch->mod_level_skills.toStringList(rus, joiner);
        StringList levelGroups = pch->mod_level_groups.toStringList(rus, joiner);

        for ( int sn = 0; sn < skillManager->size( ); sn++) {
            Skill *skill = skillManager->find(sn);
            if (skill->available(pch) && pch->mod_skills[sn] != 0) {
                DLString line = skill->getNameFor(viewer);
                line << joiner << pch->mod_skills[sn];
                learnedSkills.push_back( line );
            }
        }
        if (!learnedSkills.empty())
            buf << _("{yИзменено владение умениями: ").getMessage(lang) << learnedSkills.wrap("{m", "%{y").join(", ") << "." << endl;
        if (!learnedGroups.empty())
            buf << _("{yИзменено владение группами умений: ").getMessage(lang) << learnedGroups.wrap("{m", "%{y").join(", ") << "." << endl;
        if (pch->mod_skill_all != 0)
            buf << _("{yВладение всеми умениями изменено на {m").getMessage(lang) << pch->mod_skill_all << "%{y.{x" << endl;
        if (!levelSkills.empty())
            buf << _("{yИзменен уровень умений: ").getMessage(lang) << levelSkills.wrap("{m", "{y").join(", ") << "." << endl;
        if (!levelGroups.empty())
            buf << _("{yИзменен уровень группы умений: ").getMessage(lang) << levelGroups.wrap("{m", "{y").join(", ") << "." << endl;
        if (pch->mod_level_all != 0)
            buf << _("{yУровень всех умений изменен на {m").getMessage(lang) << pch->mod_level_all << "{y.{x" << endl;
        if (pch->mod_level_spell != 0)
            buf << _("{yУровень всех заклинаний изменен на {m").getMessage(lang) << pch->mod_level_spell << "{y.{x" << endl;
    }

    if (IS_CHARMED(ch)) {
        if (buf.str( ).empty( )) 
            oldact(_("$C1 не находится под действием каких-либо аффектов."), ch->master, 0, ch, TO_CHAR);
        else 
            oldact(_("$C1 находится под действием следующих аффектов:"), ch->master, 0, ch, TO_CHAR);
        buf << "{x";
        ch->master->send_to( buf );
        return;
    }

    // Output permanent bits on top.
    permAff.printAll();

    if (buf.str( ).empty( )) {
        if (IS_SET(flags, FSHOW_EMPTY) && !permAff.isSet())
            ch->pecho( _("Ты не находишься под действием каких-либо аффектов.") );
    } 
    else {
        if (permAff.isSet())
            ch->pecho("");
        ch->pecho( _("Ты находишься под действием следующих аффектов:") );
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
