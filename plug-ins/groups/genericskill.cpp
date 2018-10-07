/* $Id: genericskill.cpp,v 1.1.2.23.6.15 2010-09-05 13:57:10 rufina Exp $
 *
 * ruffina, 2004
 */
#include "genericskill.h"
#include "genericskillloader.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "skillmanager.h"
#include "skillreference.h"
#include "skillgroup.h"

#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"

#include "mercdb.h"
#include "clan.h"
#include "merc.h"
#include "def.h"

PROF(none);
PROF(vampire);
PROF(universal);

const DLString GenericSkill::CATEGORY = "Профессиональные умения";

GenericSkill::GenericSkill( ) 
                : raceAffect( 0, &affect_flags ),
		  raceBonuses( false ),
		  classes( false ),
		  hidden( false )

{
}

GenericSkill::~GenericSkill( )
{
}

SkillGroupReference & GenericSkill::getGroup( )
{
    return group;
}

/*
 * заполняет массивы с детьми, на основании массива с предками.
 * Вызывается из GenericSkillLoader после загрузки всех generic-skills.
 */
void GenericSkill::resolve( ) 
{
    Classes::iterator c;
    
    for (c = classes.begin( ); c != classes.end( ); c++) 
	try {
	    c->second.parentNames.resolve( c->first, Pointer( this ) );
	} catch (const Exception &e) {
	    LogStream::sendError( ) << e.what( ) << endl;
	}
}

void GenericSkill::unresolve( )
{
    Classes::iterator c;
    
    for (c = classes.begin( ); c != classes.end( ); c++) {
        c->second.parents.clear( );
        c->second.children.clear( );
    }
}

/*
 * виден ли скилл этому чару в принципе, независимо от уровня.
 */
bool GenericSkill::visible( Character *ch ) const
{
    const SkillRaceBonus *rb; 
    const SkillClassInfo *ci;
    
    if (hidden.getValue( ))
	return false;

    if (ch->is_npc( )) {
	switch (mob.visible( ch->getNPC( ), this )) {
	case MPROF_ANY:
	    return true;
	case MPROF_NONE:
	    return false;
	case MPROF_REQUIRED:
	    break;
	}
    }
    
    rb = getRaceBonus( ch );
    if (rb && rb->visible( ))
        return true;
    
    ci = getClassInfo( ch );
    if (ci && ci->visible( ))
        return true;

    return false;
}

/*
 * доступен ли на этом уровне
 */
bool GenericSkill::available( Character *ch ) const
{
    return ch->getRealLevel( ) >= getLevel( ch );
}

/*
 * может ли чар _сейчас_ использовать этот (уже доступный) скилл
 */
bool GenericSkill::usable( Character *ch, bool message = false ) const
{
    bool fUsable;
    const SkillRaceBonus *rb; 

    if (!available( ch ))
	return false;
    
    if (ch->is_npc( ))
	return true;

    rb = getRaceBonus( ch );
    if (rb && !rb->isProfessional( ))
	return true;

    if (ch->getProfession( ) == prof_vampire) {
	if (spell && !IS_VAMPIRE( ch )) {
	    if (message)
		ch->send_to("Для этого необходимо превратиться в вампира!\n\r");

	    return false;
	}
	else
	    return true;
    }
    else if (ch->getProfession( ) == prof_universal) {
	if (availableForAll( ))
	    return true;
	
	if (classes.size( ) == 1)
	    return true;

        const SkillClassInfo *info = getClassInfo( ch );
        if (info && info->isAlwaysAvailable( ))
            return true;

	if (ch->getPC( )->getSubProfession( ) != prof_none) {
	    fUsable = classes.isAvailable( 
			ch->getPC( )->getSubProfession( )->getName( ) );
	}
	else
	    fUsable = false;
	
	if (!fUsable && message && spell && spell->isCasted( ))
	    ch->send_to( "Это заклинание в данный момент недоступно тебе.\r\n" );

	return fUsable;
    }
    else
	return true;

}

bool GenericSkill::availableForAll( ) const
{
    for (int i = 0; i < professionManager->size( ); i++) {
	Profession *prof = professionManager->find( i );

	if (prof->isValid( ) 
		&& prof->isPlayed( )
		&& !classes.isAvailable( prof->getName( ) ))
	    return false;
    }
	
    return true; 
}

/*
 * с какого уровня скилл станет доступен этому чару
 * Для мобов: скилы, соответствующие их off_flags, доступны с 1 уровня
 * (например: OFF_DIRT, OFF_KICK)
 */
int GenericSkill::getLevel( Character *ch ) const
{
    const SkillRaceBonus *rb; 

    if (!visible( ch ))
	return 999;
    
    if (ch->is_npc( )) {
	if (mob.visible( ch->getNPC( ), this ) == MPROF_ANY)
	    return 1;
    }

    rb = getRaceBonus( ch );
    if (rb && !rb->isProfessional( ))
	return rb->getLevel( );

    return getClassInfo( ch )->getLevel( );
}

/*
 * Для чаров возвращает процент разученности скила, с учетом всех скилов-предков.
 * Для мобов возвращает dice * level + bonus
 */
int GenericSkill::getLearned( Character *ch ) const
{
    PCharacter *pch;
    int adept;
    
    if (!usable( ch ))
	return 0;

    if (ch->is_npc( )) 
	return mob.getLearned( ch->getNPC( ), this );
    
    pch = ch->getPC( );

    if (isRaceAffect( pch ))
	return pch->getSkillData( getIndex( ) ).learned;

    adept = pch->getProfession( )->getSkillAdept( ) 
	    + pch->getProfession( )->getParentAdept( );
	    
    return learnedAux( pch, adept );
}

/*
 * вспомогательная процедура для getLearned
 * находит минимально разученный скил среди всех предков
 * (без учета скилов, разученных > 75 или совпадающих с расовыми аффектами)
 */
int GenericSkill::learnedAux( PCharacter *pch, int adept ) const
{
    const SkillClassInfo *info; 
    const SkillRaceBonus *rb;
    int percent, min;
    
    if (!available( pch )) {
	LogStream::sendError( ) << "parent skill " << getName( ) << " is not available  for " << pch->getName( ) << endl;
	return 0;
    }
	
    info = getClassInfo( pch );
    min = 100;
    
    if (info) {
	GenericSkillVector::const_iterator i;
	const GenericSkillVector &v = info->parents.getConstVector( pch );

	for (i = v.begin( ); i != v.end( ); i++) {
	    int lrn;
	    
	    lrn = (*i)->learnedAux( pch, adept );

	    if (lrn < adept)
		min = std::min( lrn, min );
	}
    }

    percent = pch->getSkillData( getIndex( ) ).learned;
    
    rb = getRaceBonus( pch );
    
    if (rb) 
	percent = std::max( percent, rb->getBonus( ) );
    
    if (isRaceAffect( pch ))
	return min;
    else
	return std::min( percent, min );
}

/*
 * pure weight of this skill; race bonuses cost nothing 
 */
int GenericSkill::getWeight( Character *ch ) const
{
    const SkillRaceBonus *rb;
    const SkillClassInfo *ci;

    rb = getRaceBonus( ch );
    if (rb)
	return 0;
    
    ci = getClassInfo( ch );
    if (ci)
	return ci->getWeight( );

    return 0;
}

int GenericSkill::getMaximum( Character *ch ) const
{
    const SkillClassInfo *ci;

    if (( ci = getClassInfo( ch ) ))
	return ci->getMaximum( );

    return BasicSkill::getMaximum( ch );
}

/*
 * Суммарная цена скила в skill points-ах, с учетом стоимости
 * всех разученных предков.
 */
int GenericSkill::getTotalWeight( PCharacter *ch ) 
{
    int result;
    
    unmark( ch );
    result = totalWeightAux( ch ) / 10;
    unmark( ch );
    return result;
}

int GenericSkill::totalWeightAux( PCharacter *ch ) 
{
    GenericSkillVector::iterator i;
    SkillClassInfo *ci = getClassInfo( ch );
    int total = ch->skill_points( getIndex( ) ); 
    
    if (!ci)
	return 0;

    if (ci->isMarked( ))
	return 0;

    ci->mark( );

    GenericSkillVector &v = ci->parents.getVector( ch );

    for (i = v.begin( ); i != v.end( ); i++)
	total += (*i)->totalWeightAux( ch );

    return total;
}

/*
 * Сколько будет стоит знание этого скила на 100%
 * (скилы-предки должны быть разучены минимум на 75%)
 */
int GenericSkill::getMaxWeight( PCharacter *ch ) 
{
    int result;
    ProfessionReference &prof = ch->getProfession( );
    int adept = prof->getSkillAdept( ) + prof->getParentAdept( );
    
    unmark( ch );
    result = maxWeightAux( ch, adept );
    unmark( ch );
    
    if (!isRaceAffect( ch ))
	result += (100 - adept) * getWeight( ch );
    
    return result / 10;
}

int GenericSkill::maxWeightAux( PCharacter *ch, int adept ) 
{
    GenericSkillVector::iterator i;
    SkillClassInfo *ci = getClassInfo( ch );
    int max = 0;
    
    if (!ci)
	return 0;

    if (ci->isMarked( ))
	return 0;
    
    ci->mark( );
    
    GenericSkillVector &v = ci->parents.getVector( ch );

    for (i = v.begin( ); i != v.end( ); i++)
	max += (*i)->maxWeightAux( ch, adept );
    
    if (isRaceAffect( ch ))
	return max;
    else
	return max + adept * getWeight( ch );
}

/*
 * skill rating for player's class (how hard is it to learn)
 */
int GenericSkill::getRating( PCharacter *ch ) const
{
    const SkillClassInfo *ci;

    ci = getClassInfo( ch );
    
    if (ci)
	return ci->getRating( );
	
    return 1;
}

/*
 * может ли чар забыть скилл (автоматически либо с пом. команды 'forget')
 * Нельзя забывать расовые бонусы и те скилы, у которых есть разученные
 * потомки. Если потомок - расовый бонус, то он не учитывается.
 */
bool GenericSkill::canForget( PCharacter *ch ) const
{
    if (!available( ch ))
	return false;

    if (ch->getSkillData( getIndex( ) ).learned <= 1)
	return false;
    
    if (getRaceBonus( ch ))
	return false;

    return forgetAux( ch );
}

bool GenericSkill::forgetAux( PCharacter *ch ) const
{
    GenericSkillVector::const_iterator i;
    const SkillClassInfo *info; 

    if (getRaceBonus( ch ))
	return true;

    info = getClassInfo( ch );
    
    const GenericSkillVector &v = info->children.getConstVector( ch );

    for (i = v.begin( ); i != v.end( ); i++)
	if (ch->getSkillData( (*i)->getIndex( ) ).learned > 1)
	    return false;
	else if (!(*i)->forgetAux( ch ))
	    return false; 

    return true;
}

/*
 * может ли чар практиковать этот скилл
 */
bool GenericSkill::canPractice( PCharacter *ch, std::ostream & buf ) const
{
    if (!available( ch ))
	return false;
    
    if (ch->getProfession( ) == prof_universal && !usable( ch )) {
	buf << "Умение '" << getNameFor( ch ) << "' сейчас недоступно тебе." << endl;
	return false;
    }

    if (ch->skill_points( ) > ch->max_skill_points) {
	buf << "Тебе уже есть, что {RЗАБЫВАТЬ{x!" << endl;
	return false;
    }
    
    return practiceAux( ch, buf );
}

bool GenericSkill::practiceAux( PCharacter *ch, std::ostream & buf ) const
{
    GenericSkillVector::const_iterator i;
    const SkillClassInfo *info; 
    int adept;
    
    if (getRaceBonus( ch ))
	return true;

    info  = getClassInfo( ch );
    adept = getAdept( ch ) /*+ ch->getProfession( )->getParentAdept( )*/;
    
    const GenericSkillVector &v = info->parents.getConstVector( ch );

    for (i = v.begin( ); i != v.end( ); i++) {
	PCSkillData &data = ch->getSkillData( (*i)->getIndex( ) );
	const char *sname = (*i)->getNameFor( ch ).c_str( );

	if (data.forgetting) {
	    buf << "Ты не можешь изучить это умение, ведь ты сейчас "
	        << "пытаешься забыть исскуство '" << sname << "'." << endl;
	    return false;
	}
	
	if (!isRaceAffect( ch ) && data.learned.getValue( ) < adept) {
	    buf << "Ты недостаточно владеешь искусством '" << sname << "'." << endl;
	    return false;
	}

	if (!(*i)->practiceAux( ch, buf ))
	    return false;
    }

    return true;
}

bool GenericSkill::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose ) 
{
    if (!mob) {
	if (verbose)
	    ch->println( "Тебе не с кем практиковаться здесь." );
	return false;
    }
    
    if (mob->pIndexData->practicer.isSet( (int)getGroup( ) ))
	return true;

    if (verbose)
	ch->pecho( "%1$^C1 не может научить тебя искусству '%2$s'.\n"
	       "Для большей информации используй: {y{hc{lRумение %2$s{lEslook %2$s{x, {y{lRгруппаумен {Dгруппа{y{lEglist {Dгруппа{x.",
	       mob, getNameFor( ch ).c_str( ) );
    return false;
}

/*
 * Печатает разную инфу: группу, цену в s.p., дерево предков, список потомков etc
 * Используется в showskill.
 */
void GenericSkill::show( PCharacter *ch, std::ostream & buf ) 
{
    float sp;
    int total, max;
    bool rus = ch->getConfig( )->ruskills;

    buf << (spell && spell->isCasted( ) ? "Заклинание" : "Умение")
        << " '{W" << getName( ) << "{x'"
	<< " '{W" << getRussianName( ) << "{x', "
	<< "входит в группу '{hg{W" 
	<< (rus ? getGroup( )->getRussianName( ) : getGroup( )->getName( )) 
	<< "{x'"
	<< endl;
    
    if (!visible( ch )) 
	return;
    
    if (ch->getProfession( ) == prof_universal) {
	int csize = classes.size( );

	if (csize > 1) {
	    DLString cl;

	    buf << "Доступно професси" << (csize == 2 ? "и" : "ям") << " ";
	    
	    for (Classes::iterator i = classes.begin( ); i != classes.end( ); i++) 
		if (i->first != prof_universal->getName( )) {
                    Profession *prof = professionManager->find( i->first );
		    cl += prof->getNameFor( ch ) + ", ";
                }

	    buf << cl.substr( 0, cl.size( ) - 2 ) << endl;
	}
    }
    
    
    sp = (float) getWeight( ch ) / 10;
    total = getTotalWeight( ch );
    max = getMaxWeight( ch );
#if 0    
    if (sp || total || max) 
	buf << "Цена {W" << sp << "{x sp, "
	    << "на всю ветку потрачено {W" << total << "{x sp, "
	    << "мастерское владение стоит {W" << max << "{x sp" 
	    << endl;
#endif    
    SkillClassInfo *ci = getClassInfo( ch );
    
    if (ci) {
	GenericSkillVector::const_iterator i;
	const GenericSkillVector &v = ci->children.getConstVector( ch );
	
	if (!v.empty( )) {
	    buf << "Позволяет выучить: ";
	    
	    for (i = v.begin( ); i != v.end( ); ) {
		buf << "{W" << (*i)->getNameFor( ch ) << "{x";

		if (++i != v.end( ))
		    buf << ", ";
	    }

	    buf << endl;
	}
    }
    
    buf << endl;
    unmark( ch );
    showParents( ch, buf, "|" );
    unmark( ch );
    buf << endl;
}

/* 
 * Печатает дерево предков для скила
 */
void 
GenericSkill::showParents( PCharacter *ch, std::ostream & buf, DLString pad ) 
{
    float sp;
    SkillClassInfo *ci = getClassInfo( ch );
    PCSkillData &data = ch->getSkillData( getIndex( ) );
    int percent = data.learned;
    
    if (!ci)
	return;

    if (ci->isMarked( ))
	buf << "{D";
    else if (!usable( ch ))
	buf << "{R";
    else if (data.forgetting) 
	buf << "{Y";
    else if (getEffective( ch ) >= getMaximum( ch ))
	buf << "{G";
    else 
	buf << "{g";

    buf << getNameFor( ch ) << "{x (";

    if (percent == 1)
	buf << "{R";
    else if (percent >= getMaximum( ch ))
	buf << "{C";
    else if (percent >= getAdept( ch ))
	buf << "{c";
    else 
	buf << "{x";
    
    buf << percent << "%{x";
    
    sp = (float) getWeight( ch ) / 10;
#if 0    
    if (sp > 0)
	buf << "*" << sp;
#endif	
    buf << ", уровень {W" << getLevel( ch ) << "{x)" << endl;
    
    ci->mark( );

    GenericSkillVector &v = ci->parents.getVector( ch );

    for (unsigned int i = 0; i < v.size( ); i++) {
	DLString pa = pad.substr( 0, pad.length( ) - 1 );

	buf << "{y" << pad << endl << pa << "+-->" << "{x";
	
	if (i + 1 == v.size( ))
	    v[i]->showParents( ch, buf, pa + "    |" );	
	else
	    v[i]->showParents( ch, buf, pad + "   |" );	
    }
}

/*
 * Возвращает структуру SkillClassInfo для класса этого чара.
 * Для мобов ищет тот класс, в котором скил доступен на самом низком левеле.
 * Мобы могут быть "многоклассовыми", в соотв-и со своими act-flags.
 */
const SkillClassInfo *
GenericSkill::getClassInfo( Character *ch ) const
{
    vector<int> proffi = ch->getProfession( )->toVector( ch ).toArray( );
    int minLevel = LEVEL_IMMORTAL;
    const SkillClassInfo *bestClass = 0;
    
    for (unsigned int i = 0; i < proffi.size( ); i++) {
	Classes::const_iterator iter = classes.find( 
		    professionManager->find( proffi[i] )->getName( ) );

	if (iter != classes.end( ) && iter->second.getLevel( ) < minLevel) {
	    minLevel = iter->second.getLevel( );
	    bestClass = &iter->second;
	}
    }
    
    return bestClass;
}

SkillClassInfo *
GenericSkill::getClassInfo( PCharacter *ch ) 
{
    Classes::iterator iter = classes.find( ch->getProfession( )->getName( ) );

    if (iter == classes.end( ) || iter->second.getClanAntiBonus( ch ))
	return NULL;
    else 
	return &iter->second;
}

SkillClassInfo * 
GenericSkill::getClassInfo( const DLString &className )
{
    GenericSkill::Classes::iterator c;

    c = classes.find( className );

    if (c == classes.end( ))
	throw Exception( "Skill " + getName( ) + " declared as parent, "
	                 "doesnt have entry for " + className + "'" );

    return &c->second;
}

/*
 * возвращает инфо о расовом бонусе для чара (if any)
 */
const SkillRaceBonus *
GenericSkill::getRaceBonus( Character *ch ) const
{
    RaceBonuses::const_iterator i = raceBonuses.find( ch->getRace( )->getName( ) );

    return (i == raceBonuses.end( ) ? NULL : &(i->second));
}

/*
 * соответствует ли этот скил какому-либо расовому аффекту для чара?
 * (пример: sneak - AFF_SNEAK)
 */
bool GenericSkill::isRaceAffect( Character *ch ) const
{
    return ch->getRace( )->getAff( ).isSet( raceAffect.getValue( ) );
}

/*
 * установка/снятие/проверка меток на дереве предков
 * (используется при обходе в глубину)
 */
void GenericSkill::unmark( PCharacter *ch )
{
    Classes::iterator c;
    
    for (c = classes.begin( ); c != classes.end( ); c++) {
	GenericSkillVector::iterator i;
	SkillClassInfo *info = &c->second;
	GenericSkillVector &v = info->parents.getVector( ch );

	info->unmark( );
	
	for (i = v.begin( ); i != v.end( ); i++)
	    (*i)->unmark( ch );
    }
}

/*--------------------------------------------------------------------------
 * SkillRaceBonus
 *--------------------------------------------------------------------------*/
bool SkillRaceBonus::visible( ) const
{
    return !isProfessional( ) 
           && getLevel( ) < LEVEL_IMMORTAL;
}

/*--------------------------------------------------------------------------
 * SkillClassInfo
 *--------------------------------------------------------------------------*/
SkillClassInfo::SkillClassInfo( )
                 : maximum( 100 ), clanAntiBonuses( false ), always( false )
{
}

/*
 * возвращает инфо о клановых запретах на использования скила 
 * для данной профессии
 */
const SkillClanAntiBonus *
SkillClassInfo::getClanAntiBonus( Character *ch ) const
{
    ClanAntiBonuses::const_iterator i;

    i = clanAntiBonuses.find( ch->getClan( )->getName( ) );

    return (i == clanAntiBonuses.end( ) ? NULL : &(i->second));
}

bool SkillClassInfo::visible( ) const
{
    return getLevel( ) < LEVEL_IMMORTAL 
            && getRating( ) > 0;
}

/*--------------------------------------------------------------------------
 * SkillRelatives
 *--------------------------------------------------------------------------*/

void SkillRelatives::clear( )
{
    evil.clear( );
    good.clear( );
    neutral.clear( );
}

GenericSkillVector & SkillRelatives::getVector( PCharacter *ch ) 
{
    return const_cast<GenericSkillVector &>( getConstVector( ch ) );
}

const GenericSkillVector & SkillRelatives::getConstVector( PCharacter *ch ) const
{
    if (IS_GOOD(ch))
	return good;
    else if (IS_EVIL(ch))
	return evil;
    else
	return neutral;
}

/*--------------------------------------------------------------------------
 * XMLSkillParents 
 *--------------------------------------------------------------------------*/

bool XMLSkillParents::nodeFromXML( const XMLNode::Pointer& child )
{
    XMLNode::Pointer node;
    DLString align, name;
    
    if (child->getName( ) != XMLNode::ATTRIBUTE_NODE) 
	return false;
    
    node = child->getFirstNode( );

    if (!node || node->getName( ).empty( ))
	return false;
    
    name = node->getName( );
    align = child->getAttribute( "align" );

    if (align.empty( ) || align == "any" || align == "all") {
	evil.push_back( name );
	good.push_back( name );
	neutral.push_back( name );
    }
    else if (align == "evil")
	evil.push_back( name );
    else if (align == "good") 
	good.push_back( name );
    else if (align == "neutral")
	neutral.push_back( name );
    else
	return false;
	
    return true;
}

bool XMLSkillParents::toXML( XMLNode::Pointer& parent ) const
{
    Names::const_iterator i;
    
    if (evil.empty( ) && good.empty( ) && neutral.empty( ))
	return false;

    parent->setType( XMLNode::XML_NODE );
    
    if (evil == good && good == neutral) {
	for (i = evil.begin( ); i != evil.end( ); i++) 
	    appendChild( *i, parent, "" );
    }
    else {
	for (i = evil.begin( ); i != evil.end( ); i++) 
	    appendChild( *i, parent, "evil" );

	for (i = neutral.begin( ); i != neutral.end( ); i++) 
	    appendChild( *i, parent, "neutral" );

	for (i = good.begin( ); i != good.end( ); i++) 
	    appendChild( *i, parent, "good" );
    }

    return true;
}

void XMLSkillParents::appendChild( const DLString& name, XMLNode::Pointer& parent, const DLString& align ) const
{
    XMLNode::Pointer child( NEW ), text( NEW );
    
    text->setCData( name );
    text->setType( XMLNode::XML_TEXT );
    child->appendChild( text );

    child->setName( XMLNode::ATTRIBUTE_NODE );
    child->setType( XMLNode::XML_NODE );

    if (!align.empty( ))
	child->insertAttribute( "align", align );

    parent->appendChild( child );
}

void XMLSkillParents::resolve( const DLString &className, GenericSkill::Pointer mySkill )
{
    Names::iterator i;
    GenericSkill::Pointer parent;

    for (i = evil.begin( ); i != evil.end( ); i++) {
	parent = getParentSkill( *i, mySkill );
	mySkill->getClassInfo( className )->parents.evil.push_back( parent );
	parent->getClassInfo( className )->children.evil.push_back( mySkill );
    }
    for (i = good.begin( ); i != good.end( ); i++) {
	parent = getParentSkill( *i, mySkill );
	mySkill->getClassInfo( className )->parents.good.push_back( parent );
	parent->getClassInfo( className )->children.good.push_back( mySkill );
    }
    for (i = neutral.begin( ); i != neutral.end( ); i++) {
	parent = getParentSkill( *i, mySkill );
	mySkill->getClassInfo( className )->parents.neutral.push_back( parent );
	parent->getClassInfo( className )->children.neutral.push_back( mySkill );
    }
}

GenericSkill::Pointer XMLSkillParents::getParentSkill( 
	    const DLString& skillName, GenericSkill::Pointer mySkill ) 
{
    Skill::Pointer skill;
    GenericSkill::Pointer gskill;
    
    skill = SkillManager::getThis( )->find( skillName );

    if (!skill)
	throw Exception( "Unknown parent " + skillName + " for skill " + mySkill->getName( ) );
	
    gskill = skill.getDynamicPointer<GenericSkill>( );

    if (!gskill)
	throw Exception( "Skill " + mySkill->getName( ) + " depends on non-generic skill " + skill->getName( ) );

    return gskill;
}

