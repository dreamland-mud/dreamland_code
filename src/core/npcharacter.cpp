/* $Id: npcharacter.cpp,v 1.1.2.15.6.19 2009/09/17 18:08:56 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "logstream.h"
#include "class.h"
#include "noun.h"
#include "grammar_entities_impl.h"
#include "ru_pronouns.h"
#include "string_utils.h"
#include "fenia/register-impl.h"

#include "wrapperbase.h"
#include "feniamanager.h"

#include "mobilebehavior.h"
#include "skill.h"
#include "skillmanager.h"
#include "skillreference.h"
#include "spelltarget.h"

#include "object.h"
#include "room.h"
#include "affect.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "race.h"
#include "merc.h"
#include "dreamland.h"
#include "def.h"

GSN(mirror);
GSN(charm_person);
PROF(none);

NPCharacter::NPCharacter( ) :
                reset_room(0),
                behavior( MobileBehavior::NODE_NAME )
{
    init( );
}

NPCharacter::~NPCharacter( )
{
}

/****************************************************************************
 * recycle 
 ****************************************************************************/
void NPCharacter::init( )
{
    Character::init( );

    pIndexData = 0;
    zone = 0;

    keyword.clearValues();
    description.clearValues();
    long_descr.clearValues();
    short_descr.clearValues();

    cachedNouns.clear( );
        
    group = 0;
    off_flags = 0;
    for (int i = 0; i < 3; i++ ) 
        damage[i] = 0;
    start_pos = POS_STANDING;
    default_pos = POS_STANDING;
    
    profession.assign( prof_none );
    spec_fun.clear( );
    behavior.clear( );
    reset_room = 0;
    switchedFrom = 0;
}

/**************************************************************************
 * gate to pc/npc info 
 **************************************************************************/
PCharacter *NPCharacter::getPC()
{
    return switchedFrom;
}

NPCharacter *NPCharacter::getNPC()
{
    return this;
}

const PCharacter *NPCharacter::getPC() const
{
    return switchedFrom;
}

const NPCharacter *NPCharacter::getNPC() const
{
    return this;
}

bool NPCharacter::is_npc( ) const
{
    return true;
}

/****************************************************************************
 * additional set/get methods 
 ****************************************************************************/

short NPCharacter::getModifyLevel( ) const 
{
    return level.getValue( );
}

/************************************************************************
 * set-get methods for string fields
 ************************************************************************/

short NPCharacter::getSex( ) const 
{
    return sex.getValue( );
}

void NPCharacter::setSex( short sex ) 
{
    this->sex.setValue( sex );
    updateCachedNouns();
}

void NPCharacter::setKeyword(const DLString& kw, lang_t lang)
{
    keyword[lang] = kw;
}

void NPCharacter::setKeyword(const XMLMultiString &keyword)
{
    this->keyword = keyword;
}

void NPCharacter::setKeyword(const DLString &str)
{
    keyword.fromMixedString(str);
}

const DLString& NPCharacter::getKeyword(lang_t lang) const
{
    return String::firstNonEmpty(keyword, pIndexData->keyword, lang);
}

const XMLMultiString& NPCharacter::getKeyword() const
{
    return keyword.emptyValues() ? pIndexData->keyword : keyword;;
}

void NPCharacter::setDescription( const DLString& d, lang_t lang )
{
    description[lang] = d;
}

const DLString & NPCharacter::getDescription( lang_t lang ) const
{
    return String::firstNonEmpty(description, pIndexData->description, lang);
}

const XMLMultiString & NPCharacter::getDescription( ) const
{
    return description.emptyValues() ? pIndexData->description : description;;
}

void NPCharacter::setDescription( const XMLMultiString &description )
{
    this->description = description;
} 

const DLString & NPCharacter::getShortDescr( lang_t lang ) const
{
    return String::firstNonEmpty(short_descr, pIndexData->short_descr, lang);
}

const DLString & NPCharacter::getLongDescr( lang_t lang ) const
{
    return String::firstNonEmpty(long_descr, pIndexData->long_descr, lang);
}

const DLString& NPCharacter::getRealKeyword(lang_t lang) const
{
    return keyword.get(lang);
}

const DLString & NPCharacter::getRealShortDescr( lang_t lang ) const
{
    return short_descr.get(lang);
}

const DLString & NPCharacter::getRealLongDescr( lang_t lang ) const
{
    return long_descr.get(lang);
}

const DLString & NPCharacter::getRealDescription( lang_t lang ) const
{
    return description.get(lang);
}

const XMLMultiString& NPCharacter::getRealKeyword() const
{
    return keyword;
}

const XMLMultiString& NPCharacter::getRealShortDescr() const
{
    return short_descr;
}

const XMLMultiString& NPCharacter::getRealLongDescr() const
{
    return long_descr;
}

const XMLMultiString& NPCharacter::getRealDescription() const
{
    return description;
}

void NPCharacter::setShortDescr( const DLString& d, lang_t lang )
{
    short_descr[lang] = d;
    updateCachedNoun(lang);
}

void NPCharacter::setLongDescr( const DLString& d, lang_t lang )
{
    long_descr[lang] = d;
}c

/*****************************************************************************
 * name and sex formatting
 *****************************************************************************/
using namespace Grammar;

Noun::Pointer NPCharacter::toNoun( const DLObject *forWhom, int flags ) const
{
    const Character *wch = dynamic_cast<const Character *>(forWhom);
    
    if (IS_SET(flags, FMT_INVIS)) {
        if (wch && !wch->can_see( this ))
            return somebody;
    }
    
    // TODO take lang from forWhom
    return cachedNouns.find(LANG_DEFAULT)->second;
}

void NPCharacter::updateCachedNouns()
{
    StringList forms;

    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        updateCachedNoun(lang);

        /* set form with all shortdescr cases and keywords */
        {
            DLString allCases, kw;

            // Summoned creatures may contain master's name in their short descr, hence
            // their name is handled differently, with the last part of the short descr removed.
            if (behavior && behavior->hasSpecialName()) {
                StringList names(getShortDescr(lang));
                if (!names.empty()) {
                    names.pop_back();
                    DLString nameNoMaster = names.join(" ");
                    allCases = russian_case_all_forms(nameNoMaster);
                }

            } else {
                allCases = cachedNouns[lang]->decline('7').colourStrip();
            }
    
            if (!allCases.empty())
                forms.push_back(allCases);

            kw = pIndexData->keyword.find(lang)->second;
            if (!kw.empty())
                forms.push_back(kw);
        }
    }

    cachedAllForms = forms.join(" ");
}

void NPCharacter::updateCachedNoun(lang_t lang)
{
    MultiGender mg(getSex(), pIndexData->gram_number);
    const DLString &fullForm = getShortDescr(lang);

    if (cachedNouns.find(lang) == cachedNouns.end()) {
        cachedNouns[lang] = InflectedString::Pointer(NEW, fullForm, mg);
    }
    else {
        cachedNouns[lang]->setFullForm(fullForm);
        cachedNouns[lang]->setGender(mg);
    }
}

const char * NPCharacter::getNameC( ) const
{
    return getNameP('1').c_str();
}

const DLString & NPCharacter::getNameP( char gram_case ) const
{   
    // Special case when we want to see all names at once, to search among them.
    if (gram_case == '7') {
        return cachedAllForms;
    }

    // TODO toNoun should take lang as argument
    return toNoun()->decline(gram_case);
}

/****************************************************************************
 * trust and immortality 
 ****************************************************************************/
int NPCharacter::get_trust( ) const
{
    if ( switchedFrom )
        return switchedFrom->get_trust();
    
    return std::min( (int)getRealLevel( ), LEVEL_HERO );
}

bool NPCharacter::isCoder( ) const
{
    return false;
}

bool NPCharacter::is_immortal( ) const
{
    return false;
}

/****************************************************************************
 * character stats 
 ****************************************************************************/
int NPCharacter::getCurrStat( int stat ) 
{
    int value;

    // Mob's perm_stat is set only once when the mob is created, all other modifiers are applied on-the-fly.
    // TODO: we might allow mob's stat to go above the max threshold in some cases, but existing battle formulae
    // need to be reviewed first.
    value = perm_stat[stat];
    value += getRace()->getStats()[stat];
    value += mod_stat[stat];
    value += getProfession( )->getStat( stat, this ); 
    return URANGE( MIN_STAT, value, MAX_STAT );
}

/****************************************************************************
 * misc 
 ****************************************************************************/
bool NPCharacter::is_vampire( ) const
{
    return false;
}
bool NPCharacter::is_mirror( ) const
{
    return isAffected( gsn_mirror );
}

/****************************************************************************
 * switched player configuration 
 ****************************************************************************/
PlayerConfig NPCharacter::getConfig( ) const
{
    if (switchedFrom)
        return switchedFrom->getConfig( );
    else
        return PlayerConfig();
}



