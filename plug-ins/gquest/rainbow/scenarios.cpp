/* $Id: scenarios.cpp,v 1.1.2.5.6.5 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
 */
#include <map>

#include "scenarios.h"
#include "rainbow.h"
#include "rainbowinfo.h"
#include "gqchannel.h"

#include "skillmanager.h"
#include "core/object.h"
#include "room.h"
#include "roomutils.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "pcmemoryinterface.h"
#include "affect.h"

#include "msgformatter.h"
#include "act.h"
#include "loadsave.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "merc.h"
#include "descriptor.h"
#include "dl_ctype.h"
#include "def.h"
#include "l10n.h"

/*--------------------------------------------------------------------------
 * Rainbow Scenario
 *-------------------------------------------------------------------------*/

bool RainbowScenario::checkArea( AreaIndexData *area ) const
{
    if (IS_SET(area->area_flag, AREA_WIZLOCK|AREA_NOQUEST|AREA_HOMETOWN|AREA_NOGATE))
        return false;
    if (area->low_range > 20)
        return false;

    return true;
}

bool RainbowScenario::checkMobile( NPCharacter *ch ) const
{
    if (!IS_SET(ch->form, FORM_BIPED))
        return false;
    if (IS_SET(ch->imm_flags, IMM_SUMMON|IMM_CHARM))
        return false;
    if ((ch->behavior && ch->behavior->hasDestiny( )) || ch->fighting)
        return false;
    if (IS_CHARMED(ch)) 
        return false;
    if (IS_SET(ch->act, ACT_AGGRESSIVE))
        return false;
    if (IS_AFFECTED(ch, AFF_BLIND))
        return false;

    return true;
}

bool RainbowScenario::checkRoom( Room *room ) const    
{
    if (IS_SET(room->room_flags, ROOM_NO_QUEST))
        return false;

    if (!room->isCommon( ))
        return false;

    for (int d = 0; d < DIR_SOMEWHERE; d++)
        if (room->exit[d] 
            && room->exit[d]->u1.to_room
            && room->exit[d]->u1.to_room->exit[dirs[d].rev]
            && room->exit[d]->u1.to_room->exit[dirs[d].rev]->u1.to_room == room)
            return true;

    return false;
}

void RainbowScenario::onQuestInit( ) const
{
    Descriptor *d;
    Character *ch;
    
    for ( d = descriptor_list; d; d = d->next ) {
        if (d->connected != CON_PLAYING)
            continue;
        if (!(ch = d->character))
            continue;
        if (ch->is_npc( ))
            continue;
        if (!canHearInitMsg( ch->getPC( ) ))
            continue;

        ch->pecho( getInitMsg( )  );
    }
}

/*--------------------------------------------------------------------------
 * Rainbow Default Scenario 
 *-------------------------------------------------------------------------*/

void RainbowDefaultScenario::canStart( ) const 
{
    if (weather_info.sky != SKY_RAINING)
        throw GQCannotStartException( "wrong weather" );
        
    if (weather_info.sunlight != SUN_LIGHT)
        throw GQCannotStartException( "wrong hour" );
}

bool RainbowDefaultScenario::checkRoom( Room *room ) const
{
    if (IS_SET(room->room_flags, ROOM_INDOORS|ROOM_DARK))
        return false;

    return RainbowScenario::checkRoom( room );
}

void RainbowDefaultScenario::printCount( int cnt, ostringstream& buf, Character *ch ) const
{
    buf << fmt( ch, _("У тебя уже есть {Y%1$d{y разноцветн%1$Iый|ых|ых кусоч%1$Iек|ка|ков. "), cnt );
}

void RainbowDefaultScenario::printTime( ostringstream& buf, Character *ch ) const
{
    buf << fmt( ch, _("Остается ") );
    RainbowGQuest::getThis( )->printRemainedTime( buf, ch );
    buf << fmt( ch, _(", чтобы собрать всю радугу.") ) << endl;
}

MultiMessage RainbowDefaultScenario::getWinnerMsgOther( PCMemoryInterface *pci ) const
{
    MultiMessage frame = _("{Y%1$s{y зажигает {Yр{Rа{Mд{Gу{Bг{Cу{x {yнад Миром!");
    DLString en = frame.getMessage( LANG_EN ); en.replaces( "%1$s", pci->getNameP( '1', LANG_EN ) );
    DLString ru = frame.getMessage( LANG_RU ); ru.replaces( "%1$s", pci->getNameP( '1', LANG_RU ) );
    DLString ua = frame.getMessage( LANG_UA ); ua.replaces( "%1$s", pci->getNameP( '1', LANG_UA ) );
    return MultiMessage( en, ru, ua );
}

bool RainbowDefaultScenario::canHearInitMsg( PCharacter *ch ) const
{
    return RoomUtils::isOutside(ch);
}

void RainbowDefaultScenario::onQuestFinish( PCharacter *ch ) const
{
    Affect af;

    af.bitvector.setTable(&res_flags);
    af.type = SkillManager::getThis( )->lookup( "rainbow shield" );
    af.duration = 180;
    af.level = 106;
    af.bitvector.setValue(RES_SUMMON|RES_CHARM|RES_SPELL|RES_WEAPON|RES_BASH
                   |RES_PIERCE|RES_SLASH|RES_FIRE|RES_COLD|RES_LIGHTNING
                   |RES_ACID|RES_NEGATIVE|RES_HOLY|RES_ENERGY|RES_MENTAL
                   |RES_LIGHT|RES_WOOD|RES_SILVER|RES_IRON|RES_MITHRIL);
    
    af.modifier = 0;
    affect_join(ch, &af);
    
    oldact(_("Ты поднимаешь к небу ладони, полные разноцветных осколков."), ch, 0, 0, TO_CHAR);
    oldact(_("$c1 поднимает ладони к небу."), ch, 0, 0, TO_ROOM);
    oldact(_("\r\n{YР{Rа{Mд{Gу{Bж{Cн{Rы{Mй{W столп света ударяет в {Cнебо!{x"), ch, 0, 0, TO_ALL);
    oldact(_("\r\nТебя окружают {Yр{Rа{Mз{Gн{Bо{Cц{Rв{Mе{Gт{Cн{Yы{Rе{x вихри!"), ch, 0, 0, TO_CHAR);
    oldact(_("\r\n{YР{Rа{Mз{Gн{Bо{Cц{Rв{Mе{Gт{Cн{Yы{Rе{x вихри окружают $c4!"), ch, 0, 0, TO_ROOM);
}  

void RainbowDefaultScenario::onGivePiece( PCharacter *hero, NPCharacter *mob ) const
{
    oldact(_("$c1 достает откуда-то цветной булыжник и отламывает от него кусочек."), mob, 0, hero, TO_ROOM);
}

/* Load a piece that is still in the old on-disk shape.
 *
 * Old:  <node>жаб|а|и|і|у|ою|і зелен|а|ої|ій|у|ою|ій</node>
 * New:  <node><name l="ru">...</name><name l="en">...</name>...</node>
 *
 * Every gquest file on disk is still the old shape, and this is not a migration
 * that can be got wrong quietly: GlobalQuestManager::save() serialises the whole
 * GlobalQuestInfo back out of memory, and the running server does that a couple
 * of times a day (105 rewrites of gquests/ in the last 60 days). A load that
 * produced an empty name would not merely blank the seven pieces on screen, it
 * would write the blanks to disk within hours and there would be nothing left to
 * recover. So read the node body into the Russian slot when no <name> child
 * turned up, and let the next save rewrite it in the new shape.
 */
void PieceDescription::fromXML( const XMLNode::Pointer &node )
{
    XMLContainer::fromXML( node );

    if (!name.emptyValues( ))
        return;

    XMLNode::Pointer body = node->getFirstNode( );

    if (body)
        name[RU] = body->getCData( );
}

/* Put an adjective in front of a noun phrase that may begin with an article.
 *
 * Russian and Ukrainian have no articles, so there the answer is the plain
 * prepend this scenario always did: "красный" + "кусочек радуги". English puts
 * the article first, and prepending there gives "red a piece of rainbow", so the
 * adjective has to slide in BEHIND the article instead.
 *
 * The indefinite article is then re-picked from the adjective, because it is the
 * adjective that now follows it: "a piece" has to become "an orange piece". The
 * vowel-letter test is the usual approximation -- it is wrong for the likes of
 * "a unique" and "an hour", and exact for the seven rainbow colours, which are
 * the only adjectives that reach here. Colours are stripped first so the test
 * lands on the letter and not on the '{' of a colour tag.
 *
 * A phrase with no leading article falls through to the plain prepend, which is
 * also what happens if it starts with markup -- same output as before. */
static DLString prepend_adjective( const DLString &adj, const DLString &base )
{
    static const char *ARTICLES[] = { "a ", "an ", "the ", 0 };

    for (int i = 0; ARTICLES[i]; i++) {
        DLString article( ARTICLES[i] );

        if (base.size( ) <= article.size( ) || base.compare( 0, article.size( ), article ) != 0)
            continue;

        DLString rest( base.substr( article.size( ) ) );

        if (article != "the ") {
            DLString bare = adj.colourStrip( );
            char c = bare.empty( ) ? ' ' : dl_tolower( bare.at( 0 ) );

            article = (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') ? "an " : "a ";
        }

        return article + adj + " " + rest;
    }

    return adj + " " + base;
}

void RainbowDefaultScenario::dressItem( Object *obj, int number ) const
{
    const XMLMultiString &pieceName = pieces[number].name;

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        // Only dress a language that has both halves. Object::getShortDescr(lang)
        // is the strict getter -- it checks the instance, then the prototype, and
        // returns empty rather than falling back to Russian. Writing every slot
        // blindly would leave an English player holding "green frog " with the
        // noun missing, which is worse than the empty slot we have today.
        const DLString &piece = pieceName.get( lang );
        if (piece.empty( ))
            continue;

        const DLString &base = obj->getShortDescr( lang );
        if (base.empty( ))
            continue;

        obj->setShortDescr( prepend_adjective( piece, base ), lang );
    }
}

/*--------------------------------------------------------------------------
 * Rainbow Sins Scenario 
 *-------------------------------------------------------------------------*/

void RainbowSinsScenario::canStart( ) const 
{
    GlobalQuestInfo::PlayerList players;
    RainbowGQuestInfo::getThis( )->findParticipants( players );
    int evils = 0;

    for (GlobalQuestInfo::PlayerList::const_iterator p = players.begin( ); p != players.end( ); p++)
        if (IS_EVIL(*p))
            evils++;

    if (evils < 1)
        throw GQCannotStartException( "not enough evil in the world" );

    if (chance(90))
        throw GQCannotStartException( "won't start" );
}

void RainbowSinsScenario::printCount( int cnt, ostringstream& buf, Character *ch ) const
{
    buf << fmt( ch, _("Тебе удалось собрать {Y%1$d{y смертн%1$Iый|ых|ых грех%1$I|а|ов. "), cnt );
}

void RainbowSinsScenario::printTime( ostringstream& buf, Character *ch ) const
{
    buf << fmt( ch, _("До закрытия вакансии остается ") );
    RainbowGQuest::getThis( )->printRemainedTime( buf, ch );
    buf << "." << endl;
}

MultiMessage RainbowSinsScenario::getWinnerMsgOther( PCMemoryInterface *pci ) const
{
    // Accusative in RU/UA ("приняли КОГО"); EN just takes the plain name.
    MultiMessage frame = _("{Y%1$s{y приняли на адскую должность!");
    DLString en = frame.getMessage( LANG_EN ); en.replaces( "%1$s", pci->getNameP( '4', LANG_EN ) );
    DLString ru = frame.getMessage( LANG_RU ); ru.replaces( "%1$s", pci->getNameP( '4', LANG_RU ) );
    DLString ua = frame.getMessage( LANG_UA ); ua.replaces( "%1$s", pci->getNameP( '4', LANG_UA ) );
    return MultiMessage( en, ru, ua );
}

bool RainbowSinsScenario::canHearInitMsg( PCharacter *ch ) const
{
    return RoomUtils::isOutside(ch);
}

void RainbowSinsScenario::onQuestFinish( PCharacter *ch ) const
{
    Affect af;

    af.bitvector.setTable(&res_flags);
    af.type = SkillManager::getThis( )->lookup( "demonic mantle" );
    af.duration = 180;
    af.level = 106;
    af.bitvector.setValue(RES_SUMMON|RES_CHARM|RES_SPELL|RES_WEAPON|RES_BASH
                   |RES_PIERCE|RES_SLASH|RES_FIRE|RES_COLD|RES_LIGHTNING
                   |RES_ACID|RES_NEGATIVE|RES_HOLY|RES_ENERGY|RES_MENTAL
                   |RES_LIGHT|RES_WOOD|RES_SILVER|RES_IRON|RES_MITHRIL);
    
    af.modifier = 0;
    affect_join(ch, &af);
   
    ch->pecho(_("\r\nИз дымки появляется секретарша Ада и произносит '{rТы приня%Gто|т|та!{x'."), ch);
    oldact(_("\r\nИз дымки появляется секретарша Ада и что-то говорит $c3."), ch, 0, 0, TO_ROOM );

    oldact(_("\r\nТебя окутывает демоническая мантия!"), ch, 0, 0, TO_CHAR);
    oldact(_("\r\nДемоническая мантия окутывает $c4!"), ch, 0, 0, TO_ROOM);
}  

void RainbowSinsScenario::onGivePiece( PCharacter *hero, NPCharacter *mob ) const
{
    oldact(_("$c1 понимающе ухмыляется, узнав в $C6 достойного преемника."), mob, 0, hero, TO_NOTVICT);
    oldact(_("$c1 понимающе ухмыляется, узнав в тебе достойного преемника."), mob, 0, hero, TO_VICT);
}

bool RainbowSinsScenario::checkMobile( NPCharacter *ch ) const
{
    if (!RainbowScenario::checkMobile( ch ))
        return false;

    if (IS_GOOD(ch))
        return false;

    return true;
}

void RainbowSinsScenario::dressItem( Object *obj, int number ) const
{
    const XMLMultiString &pieceName = pieces[number].name;

    // Replaces the name outright, so no object text is needed -- but still only
    // the languages the piece actually has. An empty slot can fall back at
    // display time; a slot filled with Russian cannot.
    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        const DLString &piece = pieceName.get( lang );

        if (!piece.empty( ))
            obj->setShortDescr( piece, lang );
    }
}

