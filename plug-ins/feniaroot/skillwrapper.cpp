#include "skillwrapper.h"

#include "grammar_entities_impl.h"
#include "skill.h"
#include "skillcommand.h"
#include "skillgroup.h"
#include "skillreference.h"
#include "profession.h"
#include "room.h"
#include "pcharacter.h"
#include "clan.h"
#include "spelltarget.h"
#include "fight.h"
#include "loadsave.h"
#include "fenia/exceptions.h"
#include "nativeext.h"
#include "regcontainer.h"
#include "reglist.h"
#include "wrappermanager.h"
#include "subr.h"
#include "wrap_utils.h"
#include "xmlattributerestring.h"
#include "calendar_utils.h"
#include "skill_utils.h"
#include "profflags.h"
#include "damageflags.h"
#include "merc.h"
#include "def.h"

using namespace std;

CLAN(battlerager);

/*----------------------------------------------------------------------
 * Skill
 *----------------------------------------------------------------------*/
NMI_INIT(SkillWrapper, "skill, умение или заклинание");

SkillWrapper::SkillWrapper( const DLString &n )
                  : name( n )
{
}

Skill * SkillWrapper::getTarget() const
{
    Skill *skill = skillManager->find(name);
    if (!skill)
        throw Scripting::Exception(name + ": skill no longer exists");
    return skill;
}

NMI_INVOKE( SkillWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<SkillWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( SkillWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}

NMI_GET( SkillWrapper, nameRus, "русское название" ) 
{
    return getTarget()->getRussianName( );
}

NMI_GET( SkillWrapper, index, "порядковый номер (для value у волшебных предметов)" ) 
{ 
    return getTarget()->getIndex();
}

NMI_GET( SkillWrapper, spell, "заклинание для этого умения (.Spell) или null" ) 
{ 
    if (getTarget()->getSpell())
        return WrapperManager::getThis( )->getWrapper(getTarget()->getSpell().getPointer());
    else
        return Register();
}

NMI_GET( SkillWrapper, affectHandler, "обработчик аффекта для этого умения (.AffectHandler) или null" ) 
{ 
    if (getTarget()->getAffect())
        return WrapperManager::getThis( )->getWrapper(getTarget()->getAffect().getPointer());
    else
        return Register();
}

NMI_GET(SkillWrapper, spellTarget, "флаги целей заклинания (.tables.target_table)")
{
    Spell::Pointer spell = getTarget()->getSpell();
    return spell ? spell->getTarget() : 0;
}

NMI_GET(SkillWrapper, spellType, "вид заклинания (.tables.spell_types)")
{
    Spell::Pointer spell = getTarget()->getSpell();
    return spell ? spell->getSpellType() : 0;
}

NMI_GET(SkillWrapper, helpId, "ID статьи справки или 0")
{
    int help_id = 0;
    Skill *skill = getTarget();

    if (skill->getSkillHelp())
        help_id = skill->getSkillHelp()->getID();

    return max(0, help_id);
}

NMI_GET(SkillWrapper, groups, "список названий групп умения")
{
    RegList::Pointer rc(NEW);

    for (auto &group: getTarget()->getGroups().toArray()) {
        DLString groupName = skillGroupManager->find(group)->getName();
        rc->push_back(Register(groupName));
    }

    return wrap(rc);
}

NMI_GET(SkillWrapper, group, "название первой (часто и единственной) группы умения")
{
    vector<int> gsns = getTarget()->getGroups().toArray();
    DLString groupName = gsns.empty() ? "none" : skillGroupManager->find(gsns.front())->getName();
    return Register(groupName);
}

NMI_GET(SkillWrapper, category, "категория умения (.tables.skill_category_flags)")
{
    return getTarget()->getCategory();
}

NMI_INVOKE(SkillWrapper, nameFor, "(ch): название умения с учетом языковых настроек персонажа")
{
    Character *ch = args2character(args);
    return getTarget()->getNameFor(ch);
}

NMI_INVOKE(SkillWrapper, beats, "(ch): длина задержки в пульсах для персонажа с учетом бонусов")
{
    Character *ch = args2character(args);
    return getTarget()->getBeats(ch);
}

NMI_INVOKE(SkillWrapper, mana, "(ch): цена этого умения в мане для персонажа ch")
{
    Character *ch = args2character(args);
    return getTarget()->getMana(ch);
}

NMI_INVOKE(SkillWrapper, moves, "(ch): цена этого умения в шагах для персонажа ch [пока что одинакова для всех]")
{
    Character *ch = args2character(args);
    return getTarget()->getMoves(ch);
}

NMI_INVOKE(SkillWrapper, level, "(ch): уровень умения для персонажа с учетом бонусов")
{
    Character *ch = args2character(args);
    return skill_level(*getTarget(), ch);
}

NMI_INVOKE( SkillWrapper, usable, "(ch): доступно ли умение для использования прямо сейчас персонажу ch" )
{
    Character *ch = args2character(args);
    return getTarget()->usable( ch, false );
}

NMI_INVOKE( SkillWrapper, visible, "(ch): видно ли это умение ch, независимо от уровня, включая временные скилы" )
{
    Character *ch = args2character(args);
    return getTarget()->visible( ch );
}

NMI_INVOKE( SkillWrapper, adept, "(ch): вернуть максимальное значение, до которого можно практиковаться" )
{
    PCharacter *ch = args2player(args); 
    return getTarget()->getAdept(ch);
}

NMI_INVOKE( SkillWrapper, maximum, "(ch): вернуть максимальное значение раскачки умения для персонажа" )
{
    PCharacter *ch = args2player(args); 
    return getTarget()->getMaximum(ch);
}

NMI_INVOKE( SkillWrapper, rating, "(ch): сложность прокачки этого умения для персонажа, 1 по умолчанию, >1 для более сложных" )
{
    PCharacter *ch = args2player(args); 
    Skill *skill = getTarget();
    BasicSkill *basicSkill = dynamic_cast<BasicSkill *>(skill);

    if (basicSkill)
        return basicSkill->getRating(ch);
    else
        return 1;
}

NMI_INVOKE( SkillWrapper, origin, "(ch): как умение досталось персонажу (.tables.skill_origin_table)" )
{
    PCharacter *ch = args2player(args); 
    int sn = getTarget()->getIndex();
    PCSkillData &data = ch->getSkillData(sn);

    return data.origin.getValue();
}

NMI_INVOKE( SkillWrapper, learned, "(ch[,percent]): вернуть разученность или установить ее в percent" )
{
    PCharacter *ch = args2player(args); 
    int sn = getTarget()->getIndex();

    if (args.size() > 1) {
        int value = args.back( ).toNumber( );
        
        if (value < 0)
            throw Scripting::IllegalArgumentException( );
        
        ch->getSkillData(sn).learned = value;
        return Register( );
    }

    return Register(ch->getSkillData(sn).learned);
}

NMI_INVOKE( SkillWrapper, practice, "(ch): разучить умение, потратив 1 практику" )
{
    PCharacter *ch = args2player(args); 
    Skill *skill = getTarget();
    skill->practice(ch);
    return Register();
}

NMI_INVOKE( SkillWrapper, effective, "(ch): узнать процент раскачки у персонажа" )
{
    Character *ch = args2character(args);
    return Register( getTarget()->getEffective(ch) );
}

NMI_INVOKE( SkillWrapper, improve, "(ch,success[,victim]): попытаться улучшить знание умения на успехе/неудаче (true/false), применен на жертву" )
{
    Character *ch = argnum2character(args, 1);
    int success = argnum2number(args, 2);
    Character *victim = args.size() > 2 ? argnum2character(args, 3) : NULL;
     
    getTarget()->improve( ch, success, victim );
    return Register( );
}

NMI_INVOKE( SkillWrapper, giveTemporary, "(ch[,learned[,days[,origin]]]): присвоить временное умение персонажу, разученное на learned % (или на 75%), работающее days дней (или вечно), помеченное как origin (или fenia). Вернет true, если присвоено успешно.")
{
    Character *ach = argnum2character(args, 1);
    if (ach->is_npc())
        return false;

    PCharacter *ch = ach->getPC();
    int learned = args.size() > 1 ? argnum2number(args, 2) : ch->getProfession()->getSkillAdept();
    long today = day_of_epoch(time_info);
    long end;
    int origin;

    if (args.size() <= 2)
        end = PCSkillData::END_NEVER;
    else {
        end = argnum2number(args, 3);
        if (end < 0)
            end = PCSkillData::END_NEVER;
        else
            end = today + end;
    }

    if (args.size() >= 4)
        origin = argnum2flag(args, 4, skill_origin_table);
    else
        origin = SKILL_FENIA;

    if (learned <= 0)
        throw Scripting::Exception("learned param cannot be negative");

    // Do nothing for already available permanent or temporary skills.
    Skill *skill = getTarget();
    if (skill->available(ch))
        return Register(false);
    
    // Do nothing for spells and battlerager clan.
    if (skill->getSpell() && skill->getSpell()->isCasted() && ch->getClan() == clan_battlerager)
        return Register(false);

    // Create and save temporary skill data.
    PCSkillData &data = ch->getSkillData(skill->getIndex());
    data.origin = origin;
    data.start = today;
    data.end = end;
    data.learned = learned;

    return skill->available(ch); // check again if became available, as align restrictions may prevent it
}

NMI_INVOKE( SkillWrapper, removeTemporary, "(ch[,origin]): очистить временное умение у персонажа, помеченное как origin (.tables.skill_origin_table). Вернет true, если было что очищать.")
{
    Character *ach = argnum2character(args, 1);
    if (ach->is_npc())
        return false;

    PCharacter *ch = ach->getPC();
    Skill *skill = getTarget();
    PCSkillData &data = ch->getSkillData(skill->getIndex());
    int origin;

    if (args.size() >= 2)
        origin = argnum2flag(args, 2, skill_origin_table);
    else
        origin = SKILL_FENIA;

    if (!data.isTemporary())
        return Register(false);
    if (data.origin != origin)
        return Register(false);

    bool rc = skill->available(ch); // only return true if the skill was available in the first place
    data.clear();

    return rc;
}

NMI_INVOKE(SkillWrapper, apply, "(ch,vict|obj|room|arg[,level]): выполнить умение без проверок и сообщений")
{
    Skill *skill = getTarget();
    Character *ch = argnum2character(args, 1);
    bool rc;

    if (skill->getSpell()) {
        SpellTarget::Pointer target = argnum2target(args, 2);
        int level = argnum2number(args, 3);
        rc = skill->getSpell()->apply(ch, target, level);
    }
    else {
        Character *victim = args.size() > 1 ? argnum2character(args, 2) : 0;
        int level = args.size() > 2 ? argnum2number(args, 3) : 0;
        rc = skill->getCommand()->apply(ch, victim, level);
    }

    return Register(rc);
}

NMI_INVOKE(SkillWrapper, dressItem, "(obj,ch[,key]): рестрингнуть предмет согласно аттрибутам персонажа")
{
    Object *item = argnum2item(args, 1);
    Character *ch = argnum2character(args, 2);
    DLString key;
    if (args.size() > 2)
        key = argnum2string(args, 3);

    dress_created_item(getTarget()->getIndex(), item, ch, key);
    return Register();
}


/*----------------------------------------------------------------------
 * SkillGroup
 *----------------------------------------------------------------------*/
NMI_INIT(SkillGroupWrapper, "skill group, группа умений");

SkillGroupWrapper::SkillGroupWrapper( const DLString &n )
                  : name( n )
{
}

Register SkillGroupWrapper::wrap(const DLString &name)
{
    SkillGroup *group = skillGroupManager->findExisting(name);
    if (!group)
        throw Scripting::Exception(name + ": skill group not found");
        
    return Register::handler<SkillGroupWrapper>(group->getName());
}

SkillGroup * SkillGroupWrapper::getTarget() const
{
    SkillGroup *group = skillGroupManager->find(name);
    if (!group)
        throw Scripting::Exception(name + ": skill group no longer exists");
    return group;
}

NMI_INVOKE( SkillGroupWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<SkillGroupWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( SkillGroupWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}

NMI_GET( SkillGroupWrapper, nameRus, "русское название" ) 
{
    return getTarget()->getRussianName( );
}
