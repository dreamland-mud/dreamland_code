/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultprofession.h"
#include "profflags.h"

#include "stringlist.h"
#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "skillmanager.h"
#include "skill.h"
#include "skillgroup.h"
#include "alignment.h"
#include "room.h"
#include "pcrace.h"
#include "merc.h"
#include "def.h"

PROF(universal);
GROUP(ancient_languages);
GROUP(card_pack);
GROUP(tattoo_master);

static const DLString LABEL_CLASS = "class";

const DLString ClassSkillHelp::TYPE = "ClassSkillHelp";

DLString ClassSkillHelp::getTitle(const DLString &label) const
{
    if (prof)
        return "умения " + prof->getRusName().ruscase('2');

    return HelpArticle::getTitle(label);
}

void ClassSkillHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Умения и заклинания класса {C" << prof->getRusName( ).ruscase('1') << "{x, {C"
       << prof->getName( ) << "{x: " << editButton(ch) << endl << endl;
        
    in << *this;

    PCharacter dummy;
    dummy.setLevel(100);
    dummy.setProfession(prof->getName());

    // Group all skills by the level they become available to this class.
    map<int, StringList> myskills;
    for (int i = 0; i < skillManager->size(); i++) {
        Skill *skill = skillManager->find(i);
        int mylevel = skill->getLevel(&dummy);

        if (mylevel > LEVEL_MORTAL)
            continue;
        
        if (skill->getGroup() == group_ancient_languages 
            || skill->getGroup() == group_card_pack
            || skill->getGroup() == group_tattoo_master)
            continue;

        DLString color = ch && skill->visible(ch) ? "{g" : "{w";
        myskills[mylevel].push_back(color + skill->getNameFor(ch) + "{x");
    }

    // Output skills highlighting those available to this player in green.
    for (auto &pair: myskills) {
        in << "Уровень {C" << pair.first << "{x: " << pair.second.join(", ") << endl;
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
    addAutoKeyword("умения " + prof->getRusName().ruscase('2'));
    addAutoKeyword("умения " + prof->getMltName().ruscase('2'));
    labels.addTransient(LABEL_CLASS);

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
    if (prof)
        return prof->getRusName().ruscase('1') + ", " + prof->getName();
    return HelpArticle::getTitle(label);
}

void ProfessionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Профессия {C" << prof->getRusName( ).ruscase( '1' ) << "{x или {C"
       << prof->getName( ) << "{x" 
       << editButton(ch) << endl << endl;
        
    in << *this << endl;

    in << "{cХарактер{x  : " << align_name_for_range( prof->getMinAlign( ), prof->getMaxAlign( ) ) << endl;

    if (prof->getEthos( ).equalsToBitNumber( ETHOS_LAWFUL ))
        in << "{cЭтос{x      : " << "законопослушный" << endl;

    if (prof->getSex( ).equalsToBitNumber( SEX_FEMALE ))
        in << "{cПол{x       : " << "женский" << endl;
    else if (prof->getSex( ).equalsToBitNumber( SEX_MALE ))
        in << "{cПол{x       : " << "мужской" << endl;

    bool found = false;

    in << "{cПараметры{x : ";
    for (int i = 0; i < stat_table.size - 1; i++) {
        int stat = prof->getStat( i );
        if (stat != 0) {
            if (found) 
                in << ", ";
            in << (stat > 0 ? "+" : "") << stat << " к " << stat_table.message( i, '3' );
            found = true;
        }
    }
    if (!found)
        in << "без изменений";
    in << endl;

    in << "{cДоп. опыт{x : " << prof->getPoints( ) << endl;

    StringList noraces, races;
    for (int i = 0; i < raceManager->size(); i++) {
        Race *race = raceManager->find(i);
        if (race->isPC()) {
            if (race->getPC()->getClasses()[prof->getIndex()] <= 0)
                noraces.push_back(race->getMltName().ruscase('2'));
            else
                races.push_back(race->getMltName().ruscase('1'));
        }
    }
    in << "{cРасы      {x: ";
    if (noraces.empty())
        in << "все";
    else if (races.size() < noraces.size() || noraces.size() > 5)
        in << "только " << races.join(", ");
    else if (!races.empty())
        in << "все, кроме " << noraces.join(", ");
    else
        in << "никто";
    in << endl;
    
    in << "{cБонус к уровню вещей{x: ";
    if (prof->getIndex( ) == prof_universal) {
        in << " (зависит от выбранной профессии)";
    } else {
        found = false;
        for (int i = 0; i < item_table.size; i++) {
            int m = prof->getWearModifier( i );
            if (m != 0) {
                if (found)
                    in << ", ";
                in << (m > 0 ? "+" : "") << m << " к " << item_table.message( i, '3' );
                found = true;
            }
        }
    }

    in << endl;
    in << endl << "Подробнее обо всех параметрах читай в %H% [(class stats,профессия характеристики)]. ";
    if (prof->skillHelp && prof->skillHelp->getID() > 0)
        in << "%SA% %H% [(" << prof->getName() << " skills,умения " 
           << prof->getRusName().ruscase('2') << ")].";
    in << endl;
}

/*-------------------------------------------------------------------
 * ProfessionTitles
 *------------------------------------------------------------------*/
const DLString ProfessionTitlesByLevel::TYPE = "ProfessionTitlesByLevel";

const DLString & ProfessionTitlesByLevel::build( const PCMemoryInterface *pcm ) const
{
    unsigned int level = pcm->getLevel( );

    if (level >= size( ))
        return DLString::emptyString;
        
    const ProfessionTitlePair &pair = (*this)[level]; 

    return (pcm->getSex( ) == SEX_FEMALE 
                ? pair.female.getValue( ) : pair.male.getValue( ));
}

const DLString & ProfessionTitlesByConstant::build( const PCMemoryInterface *pcm ) const
{
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
int DefaultProfession::getWeapon( ) const
{
    return weapon.getValue( );
}
int DefaultProfession::getSkillAdept( ) const
{
    return skillAdept.getValue( );
}
int DefaultProfession::getParentAdept( ) const
{
    return parentAdept.getValue( );
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
Flags DefaultProfession::getFlags( Character * ) const
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

const DLString & DefaultProfession::getTitle( const PCMemoryInterface *pcm ) const
{
    return titles->build( pcm );
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

GlobalBitvector DefaultProfession::toVector( Character * ) const
{
    GlobalBitvector bv( professionManager );

    bv.set( getIndex( ) );
    return bv;
}

DLString DefaultProfession::getNameFor( Character *ch, const Grammar::Case &c ) const
{
    if (ch && ch->getConfig( )->rucommands)
        return getRusName( ).ruscase( c );
    else
        return getName( );
}

DLString DefaultProfession::getWhoNameFor( Character *ch ) const
{
    if (ch && ch->getConfig( )->rucommands)
        return whoNameRus;
    else
        return whoName;
}

