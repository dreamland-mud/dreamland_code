/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultprofession.h"
#include "profflags.h"

#include "stringlist.h"
#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "player_utils.h"
#include "skillmanager.h"
#include "skill.h"
#include "skillgroup.h"
#include "alignment.h"
#include "room.h"
#include "pcrace.h"
#include "l10n.h"
#include "merc.h"
#include "def.h"

GROUP(ancient_languages);
GROUP(card_pack);
GROUP(tattoo_master);

static const DLString LABEL_CLASS = "class";

const DLString ClassSkillHelp::TYPE = "ClassSkillHelp";

DLString ClassSkillHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc") {
        if (prof)
            buf << "Умения " << prof->getRusName().ruscase('2');
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    // Default title if not set explicitly.
    if (title.get(RU).empty() && prof)
        return "Умения " + prof->getRusName().ruscase('2');

    return HelpArticle::getTitle(label);
}

void ClassSkillHelp::getRawText( Character *ch, ostringstream &in ) const
{
    static const DLString PROF_SKILL_TYPE = "GenericSkill";

    // True if it's a help json dump and not a player requesting the article.
    bool autodump = ch->desc == 0;

    in << l(ch, "Навыки и заклинания класса") << " {C" << prof->getNameFor( ch, Grammar::Case('1') ) << "{x, {C"
       << prof->getName( ) << "{x: " << editButton(ch) << endl << endl;

    in << text.getForLang( Player::displayLang(ch) );

    PCharacter dummy;
    dummy.setLevel(100);
    dummy.setProfession(prof->getName());
    dummy.alignment = ALIGN_NONE; // ensure align and ethos restrictions won't be active
    dummy.ethos = ETHOS_NULL;

    // Group all skills by the level they become available to this class.
    map<int, StringList> myskills;
    for (int i = 0; i < skillManager->size(); i++) {
        Skill *skill = skillManager->find(i);
        int mylevel = skill->getLevel(&dummy);

        if (mylevel > LEVEL_MORTAL)
            continue;
        
        // Exclude craft and other non-class skills.
        XMLVariableContainer *xmlVar = dynamic_cast<XMLVariableContainer *>(skill);
        if (xmlVar && xmlVar->getType() != PROF_SKILL_TYPE)
            continue;

        DLString skillName;

        // Show hyper-links to skills but only for json dump.
        if (autodump) {
            skillName << "{hh";
            if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
                skillName << skill->getSkillHelp()->getID();
            skillName << skill->getRussianName() << "{hx";

        } else {
            // For player interactive help, highlight available skills in green.
            DLString color = ch && skill->visible(ch) ? "{g" : "{w";
            skillName = color + skill->getNameFor(ch) + "{x";
        }

        myskills[mylevel].push_back(skillName);
    }

    for (auto &pair: myskills) {
        in << l(ch, "Уровень") << " {C" << pair.first << "{x: " << pair.second.join(", ") << endl;
    }
}

void ClassSkillHelp::save() const
{
    if (prof)
        prof->save();
}

void ClassSkillHelp::setProfession( DefaultProfession::Pointer prof )
{
    this->prof = prof;
    
    addAutoKeyword(prof->getName( ) + " skills");
    addAutoKeyword(prof->getName( ) + " spells");
    addAutoKeyword("заклинания " + prof->getRusName().ruscase('2'));
    addAutoKeyword("заклинания " + prof->getMltName().ruscase('2'));    
    addAutoKeyword("умения " + prof->getRusName().ruscase('2'));
    addAutoKeyword("умения " + prof->getMltName().ruscase('2'));
    addAutoKeyword("навыки " + prof->getRusName().ruscase('2'));
    addAutoKeyword("навыки " + prof->getMltName().ruscase('2'));    
    labels.addTransient(prof->getName() + "-skills");

    helpManager->registrate( Pointer( this ) );
}

void ClassSkillHelp::unsetProfession( )
{
    helpManager->unregistrate( Pointer( this ) );
    prof.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}

/*-------------------------------------------------------------------
 * ProfessionHelp 
 *------------------------------------------------------------------*/
const DLString ProfessionHelp::TYPE = "ProfessionHelp";

void ProfessionHelp::save() const
{
    if (prof)
        prof->save();
}

void ProfessionHelp::setProfession( DefaultProfession::Pointer prof )
{
    this->prof = prof;
    
    addAutoKeyword( prof->getName( ) );
    addAutoKeyword( prof->getRusName( ).ruscase( '1' ) );
    addAutoKeyword( prof->getMltName( ).ruscase( '1' ) );
    labels.addTransient(LABEL_CLASS);

    helpManager->registrate( Pointer( this ) );
}

void ProfessionHelp::unsetProfession( )
{
    helpManager->unregistrate( Pointer( this ) );
    prof.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}

DLString ProfessionHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc") {
        if (prof)
            buf << "Класс '" << prof->getRusName().ruscase('1') << "'";
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    // Default title if not set explicitly.
    if (title.get(RU).empty() && prof)
        return "Класс {c" + prof->getRusName().ruscase('1') + "{x";

    return HelpArticle::getTitle(label);
}

// Per-viewer plural (multiple) race name: UA name for a UA viewer (RU fallback),
// latin for EN, declinable RU otherwise.
static DLString raceMltNameFor( Race *race, Character *ch, char gcase )
{
    lang_t lang = Player::displayLang( ch );
    if (lang == LANG_UA && !race->getMltNameUa( ).empty( ))
        return race->getMltNameUa( ).ruscase( gcase );
    if (lang == LANG_EN)
        return race->getName( );
    return race->getMltName( ).ruscase( gcase );
}

void ProfessionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << l(ch, "Класс") << " {C" << prof->getNameFor( ch, Grammar::Case('1') ) << "{x " << l(ch, "или") << " {C"
       << prof->getName( ) << "{x"
       << editButton(ch) << endl << endl;

    in << text.getForLang( Player::displayLang(ch) ) << endl;

    in << "{cНатура{x    : " << align_name_for_range( prof->getMinAlign( ), prof->getMaxAlign( ), ch ) << endl;

    if (prof->getEthos( ).equalsToBitNumber( ETHOS_LAWFUL ))
        in << "{c" << l(ch, "Этос") << "{x      : " << l(ch, "законопослушный") << endl;

    if (prof->getSex( ).equalsToBitNumber( SEX_FEMALE ))
        in << "{c" << l(ch, "Пол") << "{x       : " << l(ch, "женский") << endl;
    else if (prof->getSex( ).equalsToBitNumber( SEX_MALE ))
        in << "{c" << l(ch, "Пол") << "{x       : " << l(ch, "мужской") << endl;

    bool found = false;

    in << "{c" << l(ch, "Параметры") << "{x : ";
    for (int i = 0; i < stat_table.size - 1; i++) {
        int stat = prof->getStat( i );
        if (stat != 0) {
            if (found)
                in << ", ";
            in << (stat > 0 ? "+" : "") << stat << l(ch, " к ") << stat_table.message( i, '3', Player::displayLang(ch) );
            found = true;
        }
    }
    if (!found)
        in << l(ch, "без изменений");
    in << endl;

    in << "{c" << l(ch, "Доп. опыт") << "{x : " << prof->getPoints( ) << endl;

    StringList noraces, races;
    for (int i = 0; i < raceManager->size(); i++) {
        Race *race = raceManager->find(i);
        if (race->isPC()) {
            if (race->getPC()->getClasses()[prof->getIndex()] <= 0)
                noraces.push_back(raceMltNameFor(race, ch, '2'));
            else
                races.push_back(raceMltNameFor(race, ch, '1'));
        }
    }
    in << "{c" << l(ch, "Расы") << "      {x: ";
    if (noraces.empty())
        in << l(ch, "все");
    else if (races.size() < noraces.size() || noraces.size() > 5)
        in << l(ch, "только ") << races.join(", ");
    else if (!races.empty())
        in << l(ch, "все, кроме ") << noraces.join(", ");
    else
        in << l(ch, "никто");
    in << endl;

    in << "{c" << l(ch, "Бонус к уровню вещей") << "{x: ";
    found = false;
    for (int i = 0; i < item_table.size; i++) {
        int m = prof->getWearModifier( i );
        if (m != 0) {
            if (found)
                in << ", ";
            in << (m > 0 ? "+" : "") << m << l(ch, " к ") << item_table.message( i, '3', Player::displayLang(ch) );
            found = true;
        }
    }

    in << endl;
    in << endl << l(ch, "Подробнее обо всех параметрах читай в %H% {hh28таблица классов{x. ");
    if (prof->skillHelp && prof->skillHelp->getID() > 0)
        in << "%SA% %H% [(" << prof->getName() << " skills,умения " 
           << prof->getRusName().ruscase('2') << ")].";
    in << endl;
}

/*-------------------------------------------------------------------
 * ProfessionTitles
 *------------------------------------------------------------------*/
const DLString ProfessionTitlesByLevel::TYPE = "ProfessionTitlesByLevel";

const DLString & ProfessionTitlesByLevel::build( const PCMemoryInterface *pcm, lang_t lang ) const
{
    unsigned int level = pcm->getLevel( );

    if (level >= size( ))
        return DLString::emptyString;

    const ProfessionTitlePair &pair = (*this)[level];

    bool female = (pcm->getSex( ) == SEX_FEMALE);

    // Prefer the requested language; an empty localized field falls back to the
    // RU male/female below, so partially-translated tables never show blanks.
    if (lang == LANG_UA) {
        const DLString &ua = female ? pair.femaleUa.getValue( ) : pair.maleUa.getValue( );
        if (!ua.empty( ))
            return ua;
    }
    else if (lang == LANG_EN) {
        const DLString &en = female ? pair.femaleEn.getValue( ) : pair.maleEn.getValue( );
        if (!en.empty( ))
            return en;
    }

    return female ? pair.female.getValue( ) : pair.male.getValue( );
}

const DLString & ProfessionTitlesByConstant::build( const PCMemoryInterface *pcm, lang_t lang ) const
{
    if (lang == LANG_UA && !titleUa.getValue( ).empty( ))
        return titleUa.getValue( );
    if (lang == LANG_EN && !titleEn.getValue( ).empty( ))
        return titleEn.getValue( );
    return title.getValue( );
}

/*-------------------------------------------------------------------
 * DefaultProfession
 *------------------------------------------------------------------*/
DefaultProfession::DefaultProfession( )
                : stats( &stat_table ),
                  wearModifiers( &item_table ),
                  flags( 0, &prof_flags ),
                  align( 0, &align_table ),
                  ethos( 0, &ethos_table ),
                  sex( 0, &sex_table )
{
}


void DefaultProfession::loaded( )
{
    professionManager->registrate( Pointer( this ) );

    if (help)
        help->setProfession( Pointer( this ) );

    if (skillHelp)
        skillHelp->setProfession(Pointer(this));
}

void DefaultProfession::unloaded( )
{
    if (help)
        help->unsetProfession( );

    if (skillHelp)
        skillHelp->unsetProfession();

    professionManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultProfession::getRusName( ) const
{
    return rusName.getValue( );
}
const DLString & DefaultProfession::getMltName( ) const
{
    return mltName.getValue( );
}
const DLString & DefaultProfession::getUaName( ) const
{
    return uaName.getValue( );
}
int DefaultProfession::getWeapon( ) const
{
    return weapon.getValue( );
}
int DefaultProfession::getSkillAdept( ) const
{
    return skillAdept.getValue( );
}
int DefaultProfession::getThac00( Character * ) const
{
    return thac00.getValue( );
}
int DefaultProfession::getThac32( Character * ) const
{
    return thac32.getValue( );
}
int DefaultProfession::getHpRate( ) const
{
    return hpRate.getValue( );
}
int DefaultProfession::getManaRate( ) const
{
    return manaRate.getValue( );
}
Flags DefaultProfession::getFlags( CharacterMemoryInterface * ) const
{
    return flags;
}
int DefaultProfession::getPoints( ) const
{
    return points.getValue( );
}

int DefaultProfession::getStat( bitnumber_t s, Character * ) const 
{
    return stats[s];
}

const DLString & DefaultProfession::getTitle( const PCMemoryInterface *pcm, lang_t lang ) const
{
    return titles->build( pcm, lang );
}

const Flags & DefaultProfession::getSex( ) const
{
    return sex;
}
const Flags & DefaultProfession::getEthos( ) const
{
    return ethos;
}
const Flags & DefaultProfession::getAlign( ) const
{
    return align;
}
int DefaultProfession::getMinAlign( ) const
{
    return minAlign.getValue( );
}
int DefaultProfession::getMaxAlign( ) const
{
    return maxAlign.getValue( );
}
int DefaultProfession::getWearModifier( int type ) const
{
    return wearModifiers[type];
}

bool DefaultProfession::isPlayed( ) const
{
    return true;
}

GlobalBitvector DefaultProfession::toVector( CharacterMemoryInterface * ) const
{
    GlobalBitvector bv( professionManager );

    bv.set( getIndex( ) );
    return bv;
}

DLString DefaultProfession::getNameFor( Character *ch, const Grammar::Case &c ) const
{
    // No viewer -> canonical English name, as before.
    lang_t lang = ch ? Player::displayLang( ch ) : LANG_EN;

    if (lang == LANG_UA && !getUaName( ).empty( ))
        return getUaName( ).ruscase( c );

    if (lang == LANG_EN)
        return getName( );

    // LANG_RU, or a UA viewer for a class with no UA name yet -> Russian.
    return getRusName( ).ruscase( c );
}

DLString DefaultProfession::getWhoNameFor( Character *ch ) const
{
    // EN viewers (and no-viewer) get the latin who-tag; UA reuses the RU 3-char
    // tag (CLASSES.md).
    lang_t lang = ch ? Player::displayLang( ch ) : LANG_EN;
    if (lang == LANG_EN)
        return whoName;
    return whoNameRus;
}

