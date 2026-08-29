/* $Id$
 *
 * ruffina, 2004
 */
#include <set>

#include "defaultrace.h"
#include "helpmeta.h"
#include "defaultpcrace.h"
#include "raceflags.h"

#include "class.h"
#include "grammar_entities_impl.h"

#include "skillgroup.h"                                                       
#include "skill.h"
#include "skillmanager.h"
#include "pcharacter.h"
#include "player_utils.h"
#include "wearlocation.h"
#include "alignment.h"
#include "logstream.h"
#include "websocketrpc.h"
#include "profflags.h"

#include "dreamland.h"
#include "l10n.h"
#include "merc.h"
#include "def.h"

using namespace std;

/*-------------------------------------------------------------------
 * RaceHelp 
 *------------------------------------------------------------------*/
static const DLString LABEL_RACE = "race";
const DLString RaceHelp::TYPE = "RaceHelp";
GROUP(ancient_languages);

void RaceHelp::setRace( DefaultRace::Pointer race )
{
    this->race = race;
    
    addAutoKeyword( race->getName( ) );
    addAutoKeyword( race->getMaleName( ).ruscase( '1' ) );
    addAutoKeyword( race->getFemaleName( ).ruscase( '1' ) );
    addAutoKeyword( race->getMltName( ).ruscase( '1' ) );
    if (!race->getUkrainianName( ).empty( ))
        addAutoKeyword( race->getUkrainianName( ).ruscase( '1' ) );
    // The header shows the singular UA name (male/female); getUkrainianName above
    // is the plural, so register the singular forms too or a Ukrainian reader
    // could not type the very name they are shown.
    if (!race->getMaleNameUa( ).empty( ))
        addAutoKeyword( race->getMaleNameUa( ).ruscase( '1' ) );
    if (!race->getFemaleNameUa( ).empty( ) && race->getFemaleNameUa( ) != race->getMaleNameUa( ))
        addAutoKeyword( race->getFemaleNameUa( ).ruscase( '1' ) );
    labels.addTransient(LABEL_RACE);

    helpManager->registrate( Pointer( this ) );
}

void RaceHelp::unsetRace( )
{
    helpManager->unregistrate( Pointer( this ) );
    race.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}

void RaceHelp::save() const
{
    if (race)
        race->save();
}

DLString RaceHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc") {
        if (race)
            buf << race->getMaleName().ruscase('1').upperFirstCharacter();
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    // Default title if not set explicitly.
    if (title.get(RU).empty() && race)
        return "Раса {c" + race->getMltName().ruscase('1') + "{x";

    return HelpArticle::getTitle(label);
}

/**
 * Same title, composed in the viewer's language.
 *
 * Most articles carry no authored <title> -- theirs is assembled here at
 * display time, and that assembly was Russian for every viewer, including the
 * "toc" form the website's category rail is built from.
 */
static DLString race_name_for(::Pointer<DefaultRace> race, lang_t lang, bool plural)
{
    // EN has no separate noun: the registry key IS the English race name.
    if (lang == LANG_EN)
        return race->getName();

    if (lang == LANG_UA) {
        const DLString &ua = plural ? race->getMltNameUa() : race->getMaleNameUa();
        if (!ua.empty())
            return ua.ruscase('1');
    }

    return (plural ? race->getMltName() : race->getMaleName()).ruscase('1');
}

DLString RaceHelp::getTitle(const DLString &label, lang_t lang) const
{
    if (!race || !title.get(RU).empty())
        return HelpArticle::getTitle(label, lang);

    if (label == "title")
        return DLString::emptyString;

    if (label == "toc")
        return race_name_for(race, lang, false).upperFirstCharacter();

    return "{c" + race_name_for(race, lang, false).upperFirstCharacter() + "{x";
}

struct CommaSet : public set<string> {
    void print( ostream &buf ) const {
        bool found = false;
        for (const_iterator i = begin( ); i != end( ); i++) {
            if (found)
                buf << ", ";
            buf << *i;
            found = true;
        }
    }
};
inline ostream& operator << ( ostream& ostr, const CommaSet& cset )
{
    cset.print( ostr );
    return ostr;
}

void RaceHelp::getRawText( Character *ch, ostringstream &in ) const
{
    lang_t vlang = Player::displayLang(ch);
    DLString nameF, nameM;
    if (vlang == LANG_UA) {
        nameF = (!race->getFemaleNameUa( ).empty( ) ? race->getFemaleNameUa( ) : race->getFemaleName( )).ruscase( '1' );
        nameM = (!race->getMaleNameUa( ).empty( ) ? race->getMaleNameUa( ) : race->getMaleName( )).ruscase( '1' );
    } else if (vlang == LANG_EN) {
        nameF = nameM = race->getName( );
    } else {
        nameF = race->getFemaleName( ).ruscase( '1' );
        nameM = race->getMaleName( ).ruscase( '1' );
    }

    // One name, the reader's own (with the other-gender form in parens). Drops
    // the always-appended English name: it only added a second alphabet, and for
    // an English reader nameF == nameM == getName() made it print the name twice
    // ("Race human or human"). The shown name is a registered race-help keyword
    // in every language, so it still round-trips to this article.
    in << l(ch, "Раса") << " {C" << (ch->getSex( ) == SEX_FEMALE ? nameF : nameM) << "{x";
    if (nameF != nameM)
        in << " ({C" << (ch->getSex( ) == SEX_FEMALE ? nameM : nameF) << "{x)";
    in << editButton(ch) << endl;

    const PCRace *r = race.getConstPointer<DefaultRace>()->getPC( );
    bool playable = r && r->isValid( );

    /* What the race IS, as a bullet list at the head of the article. These
     * numbers used to hang below the flavour text in a hand-aligned column,
     * where the padding had to be counted out per label and only ever lined
     * up in Russian.
     *
     * %PAUSE%d like every other metadata block: the help formatter owns `*`,
     * `_`, `=` and `(...)` as its own markup and would eat the bullet. */
    if (playable) {
        in << "%PAUSE%";
        in << help_meta_line(l(ch, "Натура"),
                             align_name_for_range( r->getMinAlign( ), r->getMaxAlign( ), ch )) << endl;

        if (!r->getStats( ).empty( )) {
            ostringstream stats;
            bool found = false;

            for (int i = 0; i < stat_table.size; i++) {
                int stat = r->getStats( )[i];
                if (stat != 0) {
                    if (found)
                        stats << ", ";
                    stats << (stat > 0 ? "+" : "") << stat << l(ch, " к ") << stat_table.message( i, '3', vlang );
                    found = true;
                }
            }
            if (!found)
                stats << l(ch, "без изменений");

            in << help_meta_line(l(ch, "Параметры"), stats.str( )) << endl;
        }

        in << help_meta_line(l(ch, "Размер"), size_table.message( r->getSize( ), '1', vlang )) << endl;

        {
            ostringstream classes;
            bool found = false;

            classes << l(ch, "любой");
            for (int i = 0; i < professionManager->size( ); i++) {
                Profession *prof = professionManager->find( i );

                if (prof->isValid()
                    && prof->isPlayed()
                    && !prof->getFlags().isSet(PROF_NEWLOCK)
                    && const_cast<PCRace *>(r)->getClasses( )[prof->getIndex( )] <= 0)
                {
                    if (!found)
                        classes << l(ch, ", кроме ");
                    else
                        classes << ", ";
                    classes << prof->getNameFor( ch, Grammar::Case('2') );
                    found = true;
                }
            }

            in << help_meta_line(l(ch, "Классы"), classes.str( )) << endl;
        }

        DLString res = imm_flags.messages( r->getRes( ), true, '1', vlang );
        if (!res.empty( )) {
            in << help_meta_line(l(ch, "Устойчивы"), DLString(l(ch, "к ")) + res) << endl;
        }
        DLString vuln = imm_flags.messages( r->getVuln( ), true, '1', vlang );
        if (!vuln.empty( )) {
            in << help_meta_line(l(ch, "Уязвимы"), DLString(l(ch, "к ")) + vuln) << endl;
        }

        DLString aff = r->getAff( ).messages( true, '1', vlang );
        if (!aff.empty( )) {
            in << help_meta_line(l(ch, "Воздействия"), aff) << endl;
        }

        in << "%RESUME%";
    }

    in << endl << text.getForLang(vlang) << endl;

    if (!playable)
        return;

    // Pretend we have a dummy character of this race. Find out
    // all available race aptitude, professional and
    // non-professioal bonuses.
    CommaSet prof100, noprof100, raceApt, lang;

    for (int sn = 0; sn < skillManager->size( ); sn++) {
        Skill *skill = skillManager->find( sn );
        DLString sname = skill->getNameFor( ch );
        PCharacter dummy;
        dummy.setRace( race->getName( ) );
        dummy.setLevel( 100 );

        // Collect skills available even for profession 'none',
        // such as race aptitudes or non-professional bonuses.
        if (skill->visible( &dummy )) {
            if (skill->getLearned( &dummy ) >= 100)
                noprof100.insert( sname );
            else if (skill->hasGroup(group_ancient_languages)) {
                int max = skill->getMaximum(&dummy);
                int adept = skill->getAdept(&dummy);
                if (max > adept) {
                    DLString langName;
                    langName << sname << l(ch, " до ") << max << "%";
                    lang.insert(langName);
                }
            }
            else
                raceApt.insert( sname );
        }

        // Pretend we're every profession in a row and check
        // for professional 100% learned bonus.
        for (int i = 0; i < professionManager->size( ); i++) {
            Profession *prof = professionManager->find( i );
            if (!prof->isPlayed( ))
                continue;
            dummy.setProfession( prof->getName( ) );
            if (skill->visible( &dummy )) {
                if (skill->getLearned( &dummy ) >= 100)
                    if (raceApt.count( sname ) == 0 && noprof100.count( sname ) == 0)
                        prof100.insert( sname );
            }
        }
    }
    
    /* The derived skill lists stay below the flavour text -- they are what the
     * race can DO, not what it is -- but wear the same bullet as the block up
     * top, so the article has one shape and not two. */
    auto skillList = [ch, &in](const char *label, const CommaSet &names) {
        if (names.empty( ))
            return;

        ostringstream buf;
        buf << names;
        in << help_meta_line(label, buf.str( )) << endl;
    };

    in << "%PAUSE%";
    skillList(l(ch, "Уникальные способности"), raceApt);
    skillList(l(ch, "Бонусы на классовые умения"), prof100);
    skillList(l(ch, "Бонусные умения"), noprof100);
    skillList(l(ch, "Знание древних языков"), lang);
    in << "%RESUME%";

    // The %H% keyword and the [reference] below need the formatter, hence the
    // RESUME above rather than one fence around everything.
    in << endl << "Подробнее о значении каждого параметра читай %H% [расовые особенности]." << endl;
}

/* ------------------------------------------------------------------
 * DefaultRace
 *------------------------------------------------------------------*/
DefaultRace::DefaultRace( ) : 
                det( 0, &detect_flags ),
                act( 0, &act_flags ),
                aff( 0, &affect_flags ),
                off( 0, &off_flags ),
                imm( 0, &imm_flags ),
                res( 0, &res_flags ),
                vuln( 0, &vuln_flags ),
                form( 0, &form_flags ),
                parts( 0, &part_flags ),
                stats( &stat_table ),
                size( SIZE_MEDIUM, &size_table ),
                wearloc( wearlocationManager ),
                affects( skillManager ),
                hunts( raceManager ),
                donates( raceManager )
{
}

const DLString & DefaultRace::getName( ) const
{
    return Race::getName( );
}

bool DefaultRace::isValid( ) const
{
    return true;
}

void DefaultRace::setName( const DLString &name )
{
    this->name = name;
}

bool DefaultRace::matchesStrict( const DLString &str ) const
{
    return Race::matchesStrict( str ); /* TODO */
}

bool DefaultRace::matchesUnstrict( const DLString &str ) const
{
    return Race::matchesUnstrict( str ); /* TODO */
}


void DefaultRace::loaded( )
{
    raceManager->registrate( Pointer( this ) );

    if (help)
        help->setRace( Pointer( this ) );
}

void DefaultRace::unloaded( )
{
    if (help)
        help->unsetRace( );

    raceManager->unregistrate( Pointer( this ) );
}

const Flags & DefaultRace::getDet( ) const
{
    return det;
}
const Flags & DefaultRace::getAct( ) const
{
    return act;
}
const Flags & DefaultRace::getAff( ) const
{
    return aff;
}
const Flags & DefaultRace::getOff( ) const
{
    return off;
}
const Flags & DefaultRace::getImm( ) const
{
    return imm;
}
const Flags & DefaultRace::getRes( ) const
{
    return res;
}
const Flags & DefaultRace::getVuln( ) const
{
    return vuln;
}
const Flags & DefaultRace::getForm( ) const
{
    return form;
}
const Flags & DefaultRace::getParts( ) const
{
    return parts;
}

const EnumerationArray & DefaultRace::getStats( ) const 
{
    return stats;
}

const GlobalBitvector & DefaultRace::getWearloc( ) const
{
    return wearloc;
}

const GlobalBitvector & DefaultRace::getAffects( ) const
{
    return affects;
}

const Enumeration & DefaultRace::getSize( ) const
{
    return size;
}

const DLString & DefaultRace::getMaleName( ) const
{
    return nameMale.getValue( );
}
const DLString & DefaultRace::getNeuterName( ) const
{
    return nameNeuter.empty( ) ? nameMale : nameNeuter;
}
const DLString & DefaultRace::getFemaleName( ) const
{
    return nameFemale.getValue( );
}
const DLString & DefaultRace::getMltName( ) const
{
    return nameMlt.getValue( );
}
const DLString & DefaultRace::getMaleNameUa( ) const
{
    return nameMaleUa.getValue( );
}
const DLString & DefaultRace::getFemaleNameUa( ) const
{
    return nameFemaleUa.getValue( );
}
const DLString & DefaultRace::getMltNameUa( ) const
{
    return nameMltUa.getValue( );
}
// Surface the UA name to GlobalRegistryElement::matchesStrict/Unstrict (empty for
// races without UA-name data yet -> safe no-op, falls back to EN/RU matching).
const DLString & DefaultRace::getUkrainianName( ) const
{
    return getMltNameUa( );
}
DLString DefaultRace::getNameFor( Character *looker, Character *me ) const
{
    lang_t lang = looker ? Player::displayLang( looker ) : LANG_EN;

    // RU and UA show a gendered name; UA prefers its own field, falling back to RU.
    if (lang != LANG_EN && me) {
        bool ua = (lang == LANG_UA);
        if (me->toNoun()->getNumber() == Number::PLURAL)
            return (ua && !getMltNameUa().empty()) ? getMltNameUa() : getMltName();
        if (me->getSex( ) == SEX_MALE)
            return (ua && !getMaleNameUa().empty()) ? getMaleNameUa() : getMaleName();
        if (me->getSex( ) == SEX_FEMALE)
            return (ua && !getFemaleNameUa().empty()) ? getFemaleNameUa() : getFemaleName();
        if (me->getSex( ) == SEX_NEUTRAL)
            return (ua && !getMaleNameUa().empty()) ? getMaleNameUa() : getNeuterName();
    }

    return getName( );
}

Flags DefaultRace::getAttitude( const Race &race ) const
{
    Flags att( 0, &race_flags );

    if (hunts.isSet (race))
        att.setBit( RACE_HUNTS );

    if (donates.isSet (race))
        att.setBit( RACE_DONATES );

    if (getForm( ).isSet( FORM_CANINE ) && race.getForm( ).isSet( FORM_FELINE ))
        att.setBit( RACE_HATES );

    if (getForm( ).isSet( FORM_FELINE ) && race.getForm( ).isSet( FORM_CANINE ))
        att.setBit( RACE_HATES );

    return att;
}

