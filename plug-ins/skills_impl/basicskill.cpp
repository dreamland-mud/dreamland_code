/* $Id: basicskill.cpp,v 1.1.2.8.6.20 2010-09-05 13:57:10 rufina Exp $
 *
 * ruffina, 2005
 */
#include "basicskill.h"
#include "logstream.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "feniamanager.h"

#include "stringlist.h"
#include "dlscheduler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "affect.h"
#include "skillreference.h"
#include "skill_utils.h"
#include "desire.h"
#include "hometown.h"
#include "feniaskillaction.h"
#include "defaultspell.h"
#include "command.h"

#include "fight.h"
#include "affectflags.h"
#include "commandflags.h"
#include "stats_apply.h"
#include "math_utils.h"
#include "act.h"
#include "dreamland.h"
#include "mercdb.h"
#include "merc.h"
#include "material.h"
#include "immunity.h"
#include "def.h"

GSN(learning);
GSN(anathema);
DESIRE(drunk);
BONUS(learning);
HOMETOWN(frigate);

BasicSkill::BasicSkill() 
                : align( 0, &align_table ),
                  ethos( 0, &ethos_table )
{
    
}

void BasicSkill::loaded( )
{
    skillManager->registrate( Pointer( this ) );

    if (spell) 
        spell->setSkill( Pointer( this ) );

    if (affect) 
        affect->setSkill( Pointer( this ) );

    if (command) 
        command->setSkill( Pointer( this ) );

    if (help) 
        help->setSkill( Pointer( this ) );

    if (eventHandler)
        eventHandler->setSkill( Pointer( this ) );

    if (spell)
        FeniaSkillActionHelper::linkWrapper(*spell);

    if (affect)
        FeniaSkillActionHelper::linkWrapper(*affect);

    if (command)
        FeniaSkillActionHelper::linkWrapper(*command);
}

void BasicSkill::unloaded( )
{
    if (command)
        FeniaSkillActionHelper::extractWrapper(*command);

    if (affect)
        FeniaSkillActionHelper::extractWrapper(*affect);

    if (spell)
        FeniaSkillActionHelper::extractWrapper(*spell);

    if (spell)
        spell->unsetSkill( );

    if (affect) 
        affect->unsetSkill( );

    if (command) 
        command->unsetSkill( );

    if (help) 
        help->unsetSkill( );

    if (eventHandler)
        eventHandler->unsetSkill( );

    skillManager->unregistrate( Pointer( this ) );
}

static void mprog_skill( Character *ch, Character *actor, const char *skill, bool success, Character *victim )
{
    FENIA_VOID_CALL( ch, "Skill", "CsiC", actor, skill, success, victim );
    FENIA_NDX_VOID_CALL( ch->getNPC( ), "Skill", "CCsiC", ch, actor, skill, success, victim );
}

static void rprog_skill( Room *room, Character *actor, const char *skill, bool success, Character *victim )
{
    FENIA_VOID_CALL( room, "Skill", "CsiC", actor, skill, success, victim );

    for (Character *rch = room->people; rch; rch = rch->next_in_room)
        mprog_skill( rch, actor, skill, success, victim );
}

void 
BasicSkill::improve( Character *ch, bool success, Character *victim, int dam_type, int dam_flags ) const 
{
    PCharacter *pch;
    int chance, xp;
    const Skill *thiz = static_cast<const Skill *>(this);
    
    if (ch->is_npc( ))
        return;

    pch = ch->getPC( );
    PCSkillData &data = pch->getSkillData( getIndex( ) );
    
    /* skill is beyond reach */
    if (!usable( pch, false )) 
        return;     

    if (data.learned <= 1)
        return;
    
    rprog_skill( ch->in_room, ch, getName( ).c_str( ), success, victim );

    if (data.isTemporary())
        return;

    data.timer = 0;

    if (pch->getRealLevel( ) > 19  && !IS_SET( pch->act, PLR_CONFIRMED ))
        return;

    if (pch->getRealLevel() > 19  && pch->getHometown() == home_frigate)
        return;
    
    /* no improve in safe rooms */
    if (IS_SET(pch->in_room->room_flags, ROOM_SAFE))
        return;
    
    /* no improve on immune mobiles */
    if (victim) {
        if (dam_type != -1 && immune_check( victim, dam_type, dam_flags ) == RESIST_IMMUNE)
            return;
    }
    
    if (data.learned >= getMaximum( pch ))
        return;

    /* check to see if the character has a chance to learn */
    chance = 10 * get_int_app( pch ).learn;
    chance /= max( 1, hard.getValue( ) ) * getRating( pch ) * 4;
    chance += pch->getRealLevel( );
    
    /* little victim - small chance */
    if (victim) {
        int diff = victim->getRealLevel( ) - pch->getModifyLevel( );
        
        if (diff < -10)
            chance += 20 * (diff + 10);
    }

    //humans always have bonus learning. does not stack with other bonuses
    bool fBonus = (bonus_learning->isActive(pch, time_info) || ch->getRace()->getName() == "human");

    if (fBonus)
        chance *= 2;
    if (number_range(1, 1000) > chance)
        return;
   
    /* now that the character has a CHANCE to learn, see if they really have */        
    if (success) {
        int learned = data.learned;
        if (fBonus)
            learned = min(learned, 60);
        chance = URANGE(5, 100 - learned, 95);
        
        if (number_percent( ) >= chance)
            return;

        pch->pecho("{GТеперь ты гораздо лучше владеешь искусством '%K'!{x", thiz);
            
        data.learned++;
    }
    else {
        int wis_mod = get_wis_app( ch ).learn;
        
        if (wis_mod <= 0)
            return;

        chance = URANGE(5, data.learned / 2, wis_mod * 15);
            
        if (number_percent( ) >= chance)
            return;

        pch->pecho("{GТы учишься на своих ошибках, и твое умение '%K' совершенствуется.{x", thiz);
        
        data.learned += number_range( 1, wis_mod );
        data.learned = min( (int)data.learned, getMaximum( pch ) );
    }

    pch->updateSkills( );
    xp = 2 * getRating( pch );

    if (pch->isAffected(gsn_learning ))
        xp += data.learned / 4;

    if (data.learned >= getMaximum( pch )) {
        pch->pecho("{WТеперь ты {Cмастерски{W владеешь искусством {C%K{W!{x", thiz);
        
        xp += 98 * getRating( pch );
    }
    
    pch->gainExp( xp );
}

int BasicSkill::getAdept( PCharacter *ch ) const
{
    int adept;

    adept  = ch->getProfession( )->getSkillAdept( );
    adept -= ch->getRemorts( ).size( ) * 3;
    
    return max( 50, adept );
}

int BasicSkill::getMaximum( Character *ch ) const
{
    return 100;
}

void BasicSkill::practice( PCharacter *ch ) const
{
    int delta;
    PCSkillData &data = ch->getSkillData( getIndex( ) );
    int &learned = data.learned;
    
    delta  = get_int_app( ch ).learn;
    delta /= max( getRating( ch ), 1 );
    delta  = max( 1, delta * 3 / 4 );
    
    learned = URANGE( 1, learned + delta, getAdept( ch ) );

    data.timer = 0;
}

int BasicSkill::getEffective( Character *ch ) const
{
    int result = getLearned( ch );

    // Don't apply bonuses/antibonuses to unavailable skills.
    // Available but not yet learned skills (1%) can receive a bonus.
    if (result < 1)
        return result;

    // Influence by worn items and affects.
    if (!ch->is_npc())
        result += skill_learned_from_affects(this, ch->getPC());

    // Daze state makes everything worse.
    if (ch->daze > 0) {
        if (getSpell( ))
            result /= 2;
        else
            result = 2 * result / 3;
    }
    
    // Being drunk would makes everything worse.
    if (!ch->is_npc( )) 
        if (desire_drunk->isActive( ch->getPC( ) ))
            result = 9 * result / 10;

    result = URANGE(0, result, 100);
    
    return result;
}


const DLString & BasicSkill::getName( ) const
{
    return Skill::getName( );
}
void BasicSkill::setName( const DLString &name ) 
{
    this->name = name;
}
const DLString & BasicSkill::getRussianName( ) const
{
    return nameRus;
}
AffectHandlerPointer BasicSkill::getAffect( ) const 
{
    return affect;
}
SpellPointer BasicSkill::getSpell( ) const 
{
    return spell;
}
SkillCommandPointer BasicSkill::getCommand( ) const 
{
    if (command.isEmpty( ))
        return Skill::getCommand( );
    else
        return command;
}
HelpArticlePointer BasicSkill::getSkillHelp( ) const
{
    return help;
}

int BasicSkill::getBeats(Character *ch) const
{
    return ch ? percentage(beats, ch->mod_beats) : beats.getValue();
}

int BasicSkill::getMana( ) const
{
    return mana.getValue( );
}
const RussianString &BasicSkill::getDammsg( ) const
{
    return dammsg;
}
int BasicSkill::getRating( PCharacter * ) const
{
    return 1;
}

bool BasicSkill::isPassive() const
{
    if (spell && spell->isCasted())
        return false;

    Command::Pointer cmd = command.getDynamicPointer<Command>();
    if (cmd && !cmd->getExtra().isSet(CMD_NO_INTERPRET))
        return false;

    return true;
}


bool BasicSkill::accessFromString(const DLString &newValue, ostringstream &errBuf)
{
    errBuf << "Этому умению невозможно задать классовые или другие ограничения." << endl;
    return false;
}

DLString BasicSkill::accessToString() const
{
    return "n/a";
}

map<DLString, int> BasicSkill::parseAccessTokens(const DLString &newValue, const GlobalRegistryBase *registry, ostringstream &errBuf) const
{
    // Split string "cleric 10, warrior 33, witch 15" by comma, into a list of tokens.
    StringList tokens;
    tokens.split(newValue, ", ");
    map<DLString, int> entriesWithLevels, empty;

    // Empty restrictions: all entries will be flushed in the calling method.
    if (tokens.empty())
        return empty;

    for (auto &t: tokens) {
        // Split "cleric 10" into cleric/10 pair, validate and remember in a map.
        DLString token(t);
        DLString entryName  = token.getOneArgument();
        DLString levelName = token.getOneArgument();

        if (!registry->hasElement(entryName)) {
            errBuf << "Не могу найти '" << entryName << "', проверьте написание." << endl;
            return empty;
        }

        Integer level;
        if (!Integer::tryParse(level, levelName)) {
            errBuf << "Уровень для " << entryName << " должен быть числом." << endl;
            return empty;
        }

        if (level < 1 || level > MAX_LEVEL) {
            errBuf << "Уровень для " << entryName << " должен быть не меньше 0 и не больше максимального уровня." << endl;
            return empty;
        }

        entriesWithLevels[entryName] = level;
    }

    return entriesWithLevels;
}

DLString BasicSkill::printLevelBonus(Character *ch) const
{
    ostringstream buf;

    int bonus = skill_level_bonus(const_cast<Skill &>(*(Skill*)this), ch);

    if (bonus > 0)
        buf << SKILL_INFO_PAD << fmt(0, "У тебя бонус {C%1$d{x уров%1$Iень|ня|ней на это умение.", bonus) << endl;
    else if (bonus < 0)
        buf << SKILL_INFO_PAD << fmt(0, "У тебя штраф {r%1$d{x уров%1$Iень|ня|ней на это умение.", bonus) << endl;

    return buf.str();
}

DLString BasicSkill::printWaitAndMana(Character *ch) const
{
    const char *pad = SKILL_INFO_PAD;
    bool empty = true; // Contains any meaningful output besides padding and new lines?
    ostringstream buf;

    buf << pad;

    int beat = getBeats(ch) / dreamland->getPulsePerSecond();
    if (beat > 0) {
        buf << fmt(0, "Задержка при выполнении {W%1$d{x секунд%1$Iу|ы|. ", beat);
        empty = false;
    }
    
    int mana = (ch && spell && spell->isCasted( )) ? spell->getManaCost(ch) : getMana();
    if (mana > 0) {
        buf << fmt(0, "Расход маны {W%d{x. ", mana);
        empty = false;
    }

    DefaultSpell::Pointer dspell = spell.getDynamicPointer<DefaultSpell>();
    if (dspell && spell->isCasted()) { 
        const char *force_type = "";
        if (dspell->flags.isSet(SPELL_PRAYER)) {
            if (dspell->flags.isSet(SPELL_MAGIC))
                force_type = " магия или молитва";
            else
                force_type = " молитва";
        }
        else if (dspell->flags.isSet(SPELL_MAGIC))
            force_type = " магия";
        
        buf << "Тип заклинания" << " {W" << spell_types.message(spell->getSpellType()) << force_type << "{x.";
        empty = false;
    }

    if (!empty)
        buf << endl;
    
    if (dspell && spell->isCasted() && spell->getTarget() != 0) {
        buf << pad << "Целью служит {W" << target_table.messages(spell->getTarget(), true);
        if (dspell->getMaxRange(ch) > 0)
            buf << " или по направлению (дальнобойное)";
        buf << "{x. " << endl;
        empty = false;
    }

    if (isPassive()) {
        if (!empty)
            buf << pad;
        buf << "Это {Cпассивное умение{x, работает автоматически." << endl;
        empty = false;
    }

    // TODO: expose spell position and show it here.

    if (empty) 
        return DLString::emptyString;

    return buf.str();
}
