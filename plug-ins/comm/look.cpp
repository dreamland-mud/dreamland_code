/* $Id: act_look.cpp,v 1.1.2.12.6.37 2014-09-19 11:34:32 rufina Exp $
 *
 * ruffina, 2005
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#include <map>
#include <list>
#include <sstream>
#include <iomanip>

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "grammar_entities_impl.h"
#include "string_utils.h"
#include "morphology.h"
#include "commandtemplate.h"
#include "command.h"
#include "commandmanager.h"
#include "mobilebehavior.h"
#include "core/behavior/behavior_utils.h"
#include "skill.h"
#include "affecthandler.h"
#include "spelltarget.h"
#include "affect.h"
#include "core/object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcrace.h"
#include "liquid.h"
#include "room.h"
#include "roomutils.h"
#include "itemutils.h"
#include "desire.h"
#include "fight.h"
#include "screenreader.h"
#include "descriptor.h"
#include "webmanip.h"
#include "websocketrpc.h"
#include "comm.h"
#include "weapontier.h"
#include "directions.h"
#include "occupations.h"
#include "player_utils.h"
#include "occupations.h"
#include "terrains.h"
#include "move_utils.h"
#include "keyhole.h"
#include "loadsave.h"
#include "../loadsave/behavior_utils.h"
#include "morphology.h"
#include "act.h"
#include "merc.h"
#include "def.h"

#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

#define CLR_MOB(ch)        (MILD(ch) ? "y" : "Y")
#define CLR_PLAYER(ch)        (MILD(ch) ? "W" : "W")
#define CLR_OBJ(ch)        (MILD(ch) ? "w" : "G")
#define CLR_OBJROOM(ch)        (MILD(ch) ? "g" : "G")
#define CLR_NOEQ(ch)        (MILD(ch) ? "D" : "w")
#define CLR_RNAME(ch)        (MILD(ch) ? "W" : "W")
#define CLR_RVNUM(ch)        (MILD(ch) ? "c" : "C")

DESIRE(bloodlust);
GSN(stardust);
GSN(rainbow_shield);
GSN(demonic_mantle);
GSN(shapeshift);
GSN(manacles);
GSN(jail);
GSN(dark_shroud);
GSN(peek);
GSN(curl);
RELIG(godiva);
PROF(druid);

/*
 * Extern functions needed
 */
bool oprog_can_fetch_corpse_pc( Character *ch, Object *container, Object *obj, bool verbose );
void show_exits_to_char( Character *ch, Room *targetRoom );
void do_look_into( Character *ch, char *arg2 );

/*
 * Local functions.
 */
static DLString format_obj_to_char( Object *obj, Character *ch, bool fShort );
static void show_pockets_to_char( Object *container, Character *ch, ostringstream &buf );
void show_list_to_char( Object *list, Character *ch, bool fShort, 
                        bool fShowNothing, DLString pocket = "", Object *container = NULL );
static void show_char_to_char_0( Character *victim, Character *ch );
void show_char_to_char_1( Character *victim, Character *ch, bool fBrief );
static void show_people_to_char( Character *list, Character *ch, bool fShowMount = true );
bool show_char_equip( Character *ch, Character *victim, ostringstream &buf, bool fShowEmpty );


/*
 * Show long description for objects and mobiles, with 
 * English names in brackets removed for screenreaders.
 */
static DLString format_longdescr_to_char(const DLString &longdescr, Character *ch)
{
    if (!uses_screenreader(ch))
        return longdescr;

    const char *descr = longdescr.c_str();
    
    // Remove (keywords) and 1 preceding space.
    ostringstream buf;
    bool skipChar = false;
    for (const char *d = descr; *d; d++) {
        // Ignore extra space just before the ( bracket.
        if (*d == ' ' && *(d+1) == '(') 
            continue;
        // Start ignoring everything after a ( bracket.
        if (*d == '(' && isalpha(*(d+1))) {
            skipChar = true;
            continue;
        }
        // Stop ignoring once bracket is closed.
        if (*d == ')' && skipChar) {
            skipChar = false;
            continue;
        }
        // Skip everything while inside the brackets.
        if (skipChar)
            continue;
        // Normal output outside of brackets. 
        buf << *d;
    }

    return buf.str();
}

// Translate colour coding (tier, unusual item) into auras when screenreader is on or colours are off.
static void format_screenreader_flags(Object *obj, ostringstream &buf, Character *ch)
{
    if (ch->is_npc())
        return;

    if (!uses_screenreader(ch) && ch->getPC()->getConfig().color)
        return;

    DLString aura = get_tier_aura(obj);
    if (!aura.empty()) {
        buf << aura << " ";
        return;
    }

    DLString myshort = obj->getShortDescr(LANG_DEFAULT);
    if (myshort.find('{') != DLString::npos)
        buf << "(Яркое) ";
}

// Display aura if the item can be looted from PC corpse
static void format_loot_mark(Object *obj, ostringstream &buf, Character *ch)
{
    if (!obj->in_obj || obj->in_obj->item_type != ITEM_CORPSE_PC)
        return;

    if (!oprog_can_fetch_corpse_pc(ch, obj->in_obj, obj, false))
        return;

    if (obj->in_obj->hasOwner( ch ))
        return;

    buf << "({cДобыча{x) "; 
}

static void oprog_show(Object *obj, Character *ch, ostringstream &buf)
{
    if (obj->behavior)
        obj->behavior->show(ch, buf);

    for (auto &paf: obj->affected.findAllWithHandler())
        if (paf->type->getAffect())
            paf->type->getAffect( )->onShow(SpellTarget::Pointer(NEW, obj), paf, ch, buf);    
}

/*
 * Show object on the floor or in inventory/equipment/container...
 */
static DLString format_obj_to_char( Object *obj, Character *ch, bool fShort )
{
    std::ostringstream buf;
    Wearlocation *wearloc = obj->wear_loc.getElement();

    // Hide items without short description inside object lists.
    if (fShort && obj->getShortDescr(LANG_DEFAULT).empty())
            return "";
    
    // Hide items without long description on the floor.
    if (!fShort && obj->getDescription(LANG_DEFAULT).empty())
        return "";
    
#define FMT(cond, buf, ch, lng, color, letter)        \
    if (!(ch)->is_npc() && IS_SET((ch)->getPC()->config, CONFIG_SHORT_OBJFLAG))   \
        buf << color << ((cond) ? letter : ".");      \
    else if ((cond))                                  \
        buf << lng;                                   

    if (wearloc->displayFlags(ch, obj)) {
        format_screenreader_flags(obj, buf, ch);

        format_loot_mark(obj, buf, ch);

        oprog_show(obj, ch, buf);

        FMT( true, buf, ch, "", "{x", "[" );
        
        FMT( IS_OBJ_STAT(obj, ITEM_INVIS), buf, ch,
            "({DНевидимо{x) ", "{D", "Н" );
            
        FMT( CAN_DETECT(ch, DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_EVIL), buf, ch,
            "({RКрасная Аура{x) ", "{R", "З" );
            
        FMT( CAN_DETECT(ch, DETECT_GOOD) && IS_OBJ_STAT(obj,ITEM_BLESS), buf, ch,
            "({CГолубая Аура{x) ", "{C", "Б" );
                
        if (obj->item_type == ITEM_PORTAL) {
            FMT( CAN_DETECT(ch, DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC), buf, ch, 
                "(Магическое) ", "{w", "М" );
        } else {
            FMT( CAN_DETECT(ch, DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC), buf, ch,
                "(Заколдовано) ", "{w", "М" );
        }

        FMT( IS_OBJ_STAT(obj, ITEM_GLOW), buf, ch,
            "({MПылает{x) ", "{M", "П" ); 

        FMT( IS_OBJ_STAT(obj, ITEM_HUM), buf, ch,   
            "({cВибрирует{x) ", "{c", "В" );
    
        FMT( true, buf, ch, "", "{x", "] " );
    }
#undef FMT
    
    bool showHint = !ch->is_npc() && IS_SET(ch->getPC()->config, CONFIG_OBJNAME_HINT);

    if (fShort)
    {
        buf << "{" << CLR_OBJ(ch) << wearloc->displayName(ch, obj) << "{x";

        if (obj->pIndexData->vnum > 5)        /* money, gold, etc */
            if (obj->condition <= 99 )
                buf << " [" << obj->get_cond_alias( ) << "]";

        if (showHint)
            buf << " (" << Syntax::label_en(obj->getKeyword()) << ")";
    }
    else
    {
        if (obj->in_room 
                && RoomUtils::isWater(obj->in_room)
                && !IS_SET(obj->extra_flags, ITEM_WATER_STAND)) 
        {
            DLString msg;
            DLString liq = obj->in_room->getLiquid()->getShortDescr();

            msg << "{" << CLR_OBJ(ch) << "{1" << "%1$^O1" << "{2";

            if (showHint)
                msg << " (" << Syntax::label_en(obj->getKeyword()) << ")";

            msg << " ";

            switch(dice(1,3)) {
            case 1: msg << "тихо круж%1$nится|атся на %2$N6.";break;
            case 2: msg << "плыв%1$nет|ут по %2$N3.";break;
            case 3: msg << "намока%1$nет|ют от %2$N2.";break;
            }

            buf << fmt( ch, msg.c_str( ), obj, liq.c_str( ) );
        }
        else {
            DLString longd = format_longdescr_to_char(obj->getDescription(LANG_DEFAULT), ch);  
            buf << "{" << CLR_OBJROOM(ch) << longd << "{x";
        }
    }

    return buf.str( );
}

/*
 * Show list of all container pockets to a character
 */
static void show_pockets_to_char( Object *container, Character *ch, ostringstream &buf )
{
    DLString name;
    std::map<DLString, int> pockets;
    Object *obj;
    
    if (container->item_type != ITEM_CONTAINER
        || !IS_SET(container->value1(), CONT_WITH_POCKETS))
        return;

    if (IS_SET(container->value1(), CONT_PUT_ON|CONT_PUT_ON2))
        buf << "Отделения: " << endl;
    else if (!container->can_wear( ITEM_TAKE ))
        buf << "Полки: " << endl;
    else
        buf << "Карманы: " << endl;
    
    for (obj = container->contains; obj; obj = obj->next_content) {
        if (obj->pocket.empty( ))
            continue;
        
        pockets[obj->pocket]++;
    }

    if (pockets.empty( )) {
        buf << "      (пустые)" << endl;
    } 
    else {
       for (std::map<DLString, int>::iterator i = pockets.begin( ); i != pockets.end( ); i++) {
           buf << "     ";
           webManipManager->decoratePocket( buf, i->first, container, ch );
           buf << "{x" << endl;
       }
    }

    buf << endl
        << "Основное отделение:" << endl;
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( Object *list, Character *ch, bool fShort, bool fShowNothing, DLString pocket, Object *container )
{
    ostringstream output;
    bool fCombine, fConfigCombine;
    map<DLString, int> dups;
    map<DLString, int>::iterator d;
    std::list<DLString> shortDescriptions;
    std::list<Object *> items;
    Object *obj;

    if ( ch->desc == 0 )
            return;
    
    if (container && pocket.empty( ))
        show_pockets_to_char( container, ch, output );
    
    fConfigCombine = (ch->is_npc() || IS_SET(ch->comm, COMM_COMBINE));

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != 0; obj = obj->next_content )
    {
        DLString strShow;

        if (obj->wear_loc != wear_none)
            continue;

        if (!ch->can_see( obj ))
            continue;
            
        if (pocket.empty( ) && !obj->pocket.empty( ))
            continue;

        if (!pocket.empty( ) && obj->pocket != pocket)
            continue;
        
        strShow = format_obj_to_char( obj, ch, fShort );

        if (strShow.empty( ))
            continue;

        fCombine = false;

        if (fConfigCombine) {
            /*
             * Look for duplicates, case sensitive.
             */
            d = dups.find( strShow );

            if (d != dups.end( )) {
                d->second++;
                fCombine = true;
            }
        }

        /*
         * Couldn't combine, or didn't want to.
         */
        if (!fCombine) {
            dups[strShow] = 1;
            shortDescriptions.push_back( strShow );
            // Remember actual items, to later construct a list of possible manipulations.
            // For combined lists, only the first object with a given description is going to be
            // considered, which is not exactly accurate.
            items.push_back( obj );
        }
    }


    /*
     * Output the formatted list.
     */
    if (shortDescriptions.empty( ))
    {
        if (fShowNothing)
            output << "     Ничего." << endl;
    }
    else {
        std::list<DLString>::iterator sd;
        std::list<Object *>::iterator item;
        int iShow;
        
        for (sd = shortDescriptions.begin( ), item = items.begin( ), iShow = 0; 
             sd != shortDescriptions.end( ); 
             sd++, item++, iShow++) {
            if (iShow >= 200) {
                output << "{" << CLR_OBJ(ch)
                        << "     ...и много чего еще.{x" << endl;
                break;
            }

            d = dups.find( *sd );

            if (fConfigCombine && d->second != 1) {
                output << fmt(0, "(%2d) ", d->second);
            }
            else
                output << "     ";

            webManipManager->decorateItem( output, *sd, *item, ch, pocket, d->second );
            output << endl;
        }
    }

    page_to_char(output.str( ).c_str( ), ch);
}

static void show_room_affects_to_char(Room *room, Character *ch, ostringstream &mainBuf)
{
    ostringstream buf;

    for (auto &paf: room->affected.findAllWithHandler())
        if (paf->type->getAffect())
            paf->type->getAffect( )->onDescr(SpellTarget::Pointer(NEW, ch), paf, buf);

    if (!buf.str().empty())
        mainBuf << endl << buf.str();
}

/*
 * Display PK-flags
 */
void show_char_pk_flags( PCharacter *ch, ostringstream &buf )
{
    struct PKFlag {
        int bit;
        char color;
        const char *descr;
    };
    static const struct PKFlag pk_flag_table [] = {
        { PK_VIOLENT, 'B', "АДРЕНАЛИН" },
        { PK_KILLER,  'R', "УБИЙЦА"  },
        { PK_THIEF,   'R', "ВОРЮГА"   },
        { PK_SLAIN,   'D', "ЖЕРТВА"   },
        { PK_GHOST,   'D', "ПРИЗРАК"   },
    };
    static const int size = sizeof(pk_flag_table) / sizeof(*pk_flag_table);

    for (int i = 0; i < size; i++)
        if (IS_SET(ch->PK_flag, pk_flag_table[i].bit))
            buf << "[{" << pk_flag_table[i].color 
                << pk_flag_table[i].descr << "{x]";
}

static void show_char_blindness(Character* ch, Character* victim, ostringstream& buf)
{
    if (IS_AFFECTED(victim, AFF_BLIND) && IS_AWAKE(victim)) {
        if (victim->fighting == ch)
            buf << "{x...вслепую размахивая во все стороны.{x" << endl;
        else
            buf << fmt(ch, "{x...%1$P1 выгляд%1$nит|ят слеп%1$Gым|ым|ой|ыми и дезориентированн%1$Gым|ым|ой|ыми.{x", victim) << endl;
    }
}

static DLString oprog_show_where( Object *furniture, Character *ch, Character *looker )
{   
    FENIA_STR_CALL( furniture, "ShowWhere", "CC", ch, looker )
    FENIA_NDX_STR_CALL( furniture, "ShowWhere", "OCC", furniture, ch, looker )
    return DLString::emptyString;
}

static void show_char_position( Character *ch, Character *victim, 
                         const char *verb, int atFlag, int onFlag,
                         ostringstream &buf )
{
    int furniture_flag = 0;
    if (!MOUNTED(victim)) {
        buf << verb << " ";

        if (victim->on != 0) {
            furniture_flag = Item::furnitureFlags(victim->on);
            
            DLString rc = oprog_show_where( victim->on, victim, ch );

            if (!rc.empty( ))
                buf << rc;
            else if (IS_SET(furniture_flag, atFlag)) 
                buf << "возле " << victim->on->getShortDescr( '2', LANG_DEFAULT );
            else if (IS_SET(furniture_flag, onFlag))
                buf << "на " << victim->on->getShortDescr( '6', LANG_DEFAULT );
            else
                buf << "в " << victim->on->getShortDescr( '6', LANG_DEFAULT );
        }
        else
            buf << "здесь";

        if (victim->position == POS_SLEEPING && !IS_AFFECTED(victim, AFF_SLEEP)) 
            if (gsn_curl->getEffective( victim ) > 1)
                buf << ", свернувшись клубочком";
    }
    else
        buf << "сидит здесь верхом на " 
            << (ch == victim->mount ? "тебе" : ch->sees( MOUNTED(victim), '6' ));

    buf << "." << endl;
    show_char_blindness( ch, victim, buf );
}

static DLString oprog_show_end( Object *furniture, Character *ch, Character *looker )
{
    FENIA_STR_CALL( furniture, "ShowEnd", "CC", ch, looker ) 
    FENIA_NDX_STR_CALL( furniture, "ShowEnd", "OCC", furniture, ch, looker )
    return DLString::emptyString;
}

static DLString rprog_show_end( Room *room, Character *ch, Character *looker )
{
    DLString rc = behavior_trigger_str(room, "ShowEnd", "RCC", room, ch, looker);
    if (!rc.empty())
        return rc;
        
    FENIA_STR_CALL( room, "ShowEnd", "CC", ch, looker ) 
    return DLString::emptyString;
}

/** Decides if a long description can be displayed, or we need to specify position. 
 *  Long descriptions are only shown if a mob is still it ins initial (starting) position
 *  after area reset. Alternatively, for overridden descriptions, show it when a mob is standing.
 */
static bool can_show_long_descr(NPCharacter *nVict)
{
    if (nVict->position == nVict->start_pos 
            && !nVict->on
            && !nVict->getLongDescr(LANG_DEFAULT).empty()) 
    {
        return true;
    }

    if (nVict->position == POS_STANDING
        && !nVict->on
        && !nVict->getRealLongDescr(LANG_DEFAULT).empty())
    {
        return true;
    }

    return false;
}

/*
 * Show a character in the room ('look' or 'look auto')
 */
static void show_char_to_char_0( Character *victim, Character *ch )
{
    std::basic_ostringstream<char> buf;
    Character *origVict;
    NPCharacter *nVict;
    PCharacter *pVict;
    
    origVict = victim;
    victim = victim->getDoppel( );

    if (victim->is_npc( )) {
        nVict = victim->getNPC( );
        pVict = 0;
    }
    else {
        nVict = 0;
        pVict = victim->getPC( );
    }

    if (nVict && nVict->behavior)
        nVict->behavior->show( ch, buf );
    
    if (pVict) {
        show_char_pk_flags( pVict, buf );

        if (pVict->desc == 0 )
            buf << "[{DБез связи{x]";

        if (IS_SET(pVict->comm, COMM_AFK ))
            buf << "[{CАФК{x]";

        if (pVict->act.isSet(PLR_RITUAL))
            buf << "({bРитуал{x)";

        if (IS_SET(pVict->act, PLR_WANTED))
            buf << "({RРАЗЫСКИВАЕТСЯ{x)";

        if (pVict->isAffected(gsn_manacles ))
            buf << "({DКАНДАЛЫ{x)";

        if (victim->isAffected(gsn_jail ))
            buf << "({DТЮРЬМА{x)";
        
        if (pVict->invis_level >= LEVEL_HERO)
            buf << "(Wizi)";
    }

    if (RIDDEN(victim))
        buf << fmt(ch, "({1{gОседлан%Gо||а{2)", victim);

    if (IS_AFFECTED(victim, AFF_INVISIBLE))
        buf << fmt(ch, "({1{DНевидим%Gо||а{2)", victim);

    if (IS_AFFECTED(victim, AFF_IMP_INVIS))
        buf << fmt(ch, "({1{bОчень невидим%Gо||а{2)", victim);

    if (IS_AFFECTED(victim, AFF_HIDE))
        buf << fmt(ch, "({1{DУкрыт%Gо||а{2)", victim);

    if (IS_AFFECTED(victim, AFF_FADE))
        buf << fmt(ch, "({1{DСпрятан%Gо||а{2)", victim);

    if (IS_AFFECTED(victim, AFF_CAMOUFLAGE))
        buf << fmt(ch, "({1{GЗамаскирован%Gо||а{2)", victim);

    if (IS_AFFECTED(victim, AFF_CHARM))
        buf << fmt(ch, "({1{mОчарован%Gо||а{2)", victim);

    if (CAN_DETECT(ch, DETECT_UNDEAD)) {
        bool npcUndead = victim->is_npc() && 
            (IS_SET(victim->act, ACT_UNDEAD) || IS_SET(victim->form, FORM_UNDEAD));
    
        if (npcUndead || IS_VAMPIRE(victim))
            buf << "({1{rНежить{2)";
    }

    if (IS_AFFECTED(victim, AFF_PASS_DOOR))
        buf << fmt(ch, "({1{wПр{Dо{wзр{Dа{wч%Gно|ен|на{2)", victim);

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
        buf << "({MРозовая Аура{x)";

    if (IS_EVIL(victim) && CAN_DETECT(ch, DETECT_EVIL))
        buf << "({RКрасная Аура{x)";

    if (IS_GOOD(victim) && CAN_DETECT(ch, DETECT_GOOD))
        buf << "({YЗолотая Аура{x)";

    if (IS_AFFECTED(victim, AFF_SANCTUARY))
        buf << "({WБелая Аура{x)";

    if (victim->isAffected(gsn_rainbow_shield))
        buf << "({RР{Yа{Gд{Cу{Bг{Mа{x)";

    if (victim->isAffected(gsn_demonic_mantle))
        buf << "({RМ{Dантия{x)";

    if (victim->isAffected(gsn_dark_shroud))
        buf << "({DАура Тьмы{x)";

    if (victim->isAffected(gsn_stardust))
        buf << "({WЗ{wве{Wзд{wная {WП{wыль{x)";

    if (nVict) 
        if (can_show_long_descr(nVict)) {
            DLString longd = format_longdescr_to_char(nVict->getLongDescr(LANG_DEFAULT), ch);
            buf << "{" << CLR_MOB(ch);
            webManipManager->decorateCharacter(buf, longd, victim, ch);
            buf << "{x";
            show_char_blindness( ch, victim, buf );
            ch->send_to( buf);
            return;
        }
    
    if (nVict) {
        buf << "{" << CLR_MOB(ch) << "{1";;
        webManipManager->decorateCharacter(buf, ch->sees( victim, '1' ).upperFirstCharacter(), victim, ch);
    }
    else {
        if (ch->getConfig( ).holy && origVict != victim)
            buf << "{1" << "{" << CLR_PLAYER(ch) << ch->sees( origVict, '1' ).upperFirstCharacter() << "{2 "
                << "(под личиной " << ch->sees( victim, '2' ) << ") ";
        else {
            buf << "{1" << "{" << CLR_PLAYER(ch);
            webManipManager->decorateCharacter( buf, ch->sees( victim, '1' ).upperFirstCharacter(), victim, ch );
            buf << "{2";
        }

        if (!IS_SET(ch->comm, COMM_BRIEF) 
            && victim->position == POS_STANDING 
            && ch->on == 0)
        {
            buf << Player::title(pVict);
        }
    }

    buf << " {2";

    if (nVict && !ch->is_npc()) {
        if (!ch->getConfig().rucommands || IS_SET(ch->getPC()->config, CONFIG_OBJNAME_HINT))
            buf << "(" << Syntax::label_en(nVict->getKeyword()) << ") ";
    }

    switch (victim->position.getValue( )) {
    case POS_DEAD:     
        buf << "уже {1{RТРУП!!!{2" << endl;
        break;
        
    case POS_MORTAL:   
        buf << "при смерти." << endl;   
        break;
        
    case POS_INCAP:    
        buf << "в беспомощном состоянии." << endl;      
        break;
        
    case POS_STUNNED:  
        buf << "лежит без сознания." << endl; 
        break;
        
    case POS_SLEEPING:
        show_char_position( ch, victim, "спит", SLEEP_AT, SLEEP_ON, buf );
        break;

    case POS_RESTING:
        show_char_position( ch, victim, "отдыхает", REST_AT, REST_ON, buf );
        break;

    case POS_SITTING:
        show_char_position( ch, victim, "сидит", SIT_AT, SIT_ON, buf );
        break;
            
    case POS_STANDING:
        show_char_position( ch, victim, "стоит", STAND_AT, STAND_ON, buf );
        break;

    case POS_FIGHTING:
        buf << "здесь, сражается ";

        if (victim->fighting == 0)
            buf << "неизвестно с кем...";
        else if (victim->fighting == ch)
            buf << "с {1{RТОБОЙ!!!{2";
        else if (victim->in_room == victim->fighting->in_room)
            buf << "с " << ch->sees( victim->fighting, '5') << ".";
        else
            buf << "кем-то, кто ушел...";

        buf << endl;
        show_char_blindness( ch, victim, buf );
        break;
    }

    if (HAS_SHADOW(victim))
        buf << " ...отбрасывает странную тень." << endl;
    
    if (victim->death_ground_delay > 0) {
        DLString rc = rprog_show_end( victim->in_room, victim, ch );

        if (!rc.empty( ))
            buf << " ... " << rc << endl;
    }
    
    if (victim->on) {
        DLString rc = oprog_show_end( victim->on, victim, ch );

        if (!rc.empty( ))
            buf << rc << endl;
    }

    ch->send_to( buf );
}

/*
 * Observation 
 */
static void show_char_diagnose( Character *ch, Character *victim, ostringstream &buf )
{
    ostringstream str;

    if (!CAN_DETECT( ch, DETECT_OBSERVATION ))
        return;

    if (IS_AFFECTED( victim, AFF_BLIND ))
        str << "Похоже, ничего не вид%1$nит|ят." << endl;
    if (IS_AFFECTED( victim, AFF_SCREAM ))
        str << "В ужасе зажима1$nет|ют уши." << endl;
    if (IS_AFFECTED( victim,  AFF_PLAGUE ))
        str << "Покрыт%1$Gо||а|ы чумными язвочками." << endl;
    if (IS_AFFECTED( victim, AFF_POISON ))
        str << "Отравлен%1$Gо||а|ы." << endl;
    if (IS_AFFECTED( victim, AFF_SLOW ))
        str << "Под воздействием З А М Е Д  Л  Е  Н   И    Я." << endl;
    if (IS_AFFECTED( victim, AFF_HASTE ))
        str << "Под воздействием ускорения, уииии!" << endl;
    if (IS_AFFECTED( victim, AFF_WEAKEN ))
        str << "Выгляд%1$nит|ят беспомощно и слабо." << endl;
    if (IS_AFFECTED( victim, AFF_CORRUPTION ))
        str << "Гни%1$nет|ют заживо." << endl;
    if (CAN_DETECT( victim, ADET_FEAR ))
        str << "Дрож%1$nит|ат от страха." << endl;
    if (IS_AFFECTED( victim, AFF_CURSE ))
        str << "Проклят%1$Gо||а|ы." << endl;
    if (IS_AFFECTED( victim, AFF_PROTECT_EVIL ))
        str << "Защищен%1$Gо||а|ы от зла" << endl;
    if (IS_AFFECTED( victim, AFF_PROTECT_GOOD ))
        str << "Защищен%1$Gо||а|ы от добра." << endl;

    DLString details = str.str();
    if (!details.empty()) 
        buf << endl << "Ты замечаешь важные детали:" << endl
            << fmt(0, details.c_str(), victim) << endl;
}

/*
 * Show character wounds
 */
static void show_char_wounds( Character *ch, Character *victim, ostringstream &buf )
{
    int percent;

    if (victim->max_hit > 0)
        percent = HEALTH(victim);
    else
        percent = -1;

    if (percent >= 100)
        buf << "{C в прекрасном состоянии";
    else if (percent >= 90)
        buf << fmt(ch, "{B име%1$nет|ют несколько царапин", victim);
    else if (percent >= 75)
        buf << fmt(ch, "{B име%1$nет|ют несколько маленьких ран и синяков", victim);
    else if (percent >= 50)
        buf << fmt(ch, "{G име%1$nет|ют довольно много ран", victim);
    else if (percent >=  30)
        buf << fmt(ch, "{Y име%1$nет|ют несколько больших, опасных ран и царапин", victim);
    else if (percent >= 15)
        buf << fmt(ch, "{M выгляд%1$nит|ят сильно поврежденн%1$Gым|ым|ой|ыми", victim);
    else if (percent >= 0 )
        buf << "{R в ужасном состоянии";
    else
        buf << fmt(ch, "{R истека%1$nет|ют кровью", victim);

    buf << ".{x" << endl;
}

static void show_char_description( Character *ch, Character *vict )
{
    if(IS_VAMPIRE(vict)){
        ostringstream buf;
        buf << "Монстр в своем ужасающем обличии. Нечисть и порождение тьмы." << endl
            << "Пара ярко-красных глаз и ослепительно острых клыков сверкают на фоне обсидиановой, почти черной, как ночь, кожи." << endl
            << fmt(0,"Огромное и мускулистое тело ночно%1$Gго|го|й хищни%1$Gка|ка|цы, обрамленное крыльями, какие есть у летучих мышей, замерло в ожидании.\n\r",vict);
        ch->send_to(buf);
        return;  
    }

    const char *dsc = vict->getDescription(LANG_DEFAULT).c_str();

    if ((vict->is_npc( ) && dsc) || (!vict->is_npc( ) && dsc[0])) {
        ch->send_to( dsc );
        return;
    }

    if (vict->getRace( )->isPC( )) {
        PCRace::Pointer pcRace = vict->getRace( )->getPC( );
        const char *rname = GET_SEX(vict, 
                                    pcRace->getMaleName( ).c_str( ),
                                    pcRace->getMaleName( ).c_str( ),
                                    pcRace->getFemaleName( ).c_str( ));
        if (ch == vict)
            oldact("Ты выглядишь как обычн$Gое|ый|ая $n1.", ch, rname, vict, TO_CHAR );
        else
            oldact("$E выглядит как обычн$Gое|ый|ая $n1.", ch, rname, vict, TO_CHAR );

        return;
    }
    
    oldact("Ты не видишь ничего особенного в $Z.", ch, 0, vict, TO_CHAR );
}

static void show_char_sexrace( Character *ch, Character *vict, ostringstream &buf )
{
    buf << "(";

    if (ch->getConfig( ).rucommands) {
        buf << (IS_VAMPIRE(vict) ? GET_SEX(vict, "вампир", "вампир", "вампирша")
                                : vict->getRace( )->getNameFor( ch, vict ).ruscase('1'));
    } else {
        buf << GET_SEX(vict, "male", "sexless", "female")
            << " "
            << (IS_VAMPIRE(vict) ? "vampire" : vict->getRace( )->getName( ));
    }
    buf << ", " << size_table.message(min(vict->size, SIZE_GARGANTUAN), '2') << " размера";
    buf << ") ";
}

/*
 *  Look at somebody 
 */
void show_char_to_char_1( Character *victim, Character *ch, bool fBrief )
{
    ostringstream buf;
    Character*        vict;
    bool naked;
    
    vict = victim->getDoppel( );
    
    if (!fBrief) 
        show_char_description( ch, vict );

    if (MOUNTED(vict))
        buf << ch->sees( vict, '1' ) << " верхом на " 
            << (ch == vict->mount ? "тебе" : ch->sees( MOUNTED( vict ), '6' ))
            << endl;

    if (RIDDEN(vict)) {
        buf << ch->sees( vict, '1' );

        if (ch == vict->mount)
            buf << " под твоим седлом";
        else
            buf << " под седлом, в котором сидит " 
                << ch->sees( RIDDEN( vict ), '1' );
        buf << "." << endl;
    }
    
    show_char_diagnose( ch, victim, buf );
    show_char_sexrace( ch, vict, buf ); 
    buf << ch->sees( vict, '1' );
    show_char_wounds( ch, victim, buf );
    ch->send_to( buf);
    
    if (fBrief)
        return;
    
    buf.str( "" );
    naked = show_char_equip( ch, victim, buf, false );

    if (ch != victim && victim->getReligion() == god_godiva && !ch->is_immortal()) {
        ch->pecho("\r\n{DПризрачное покрывало окутывает %1$C4, скрывая %1$P2 экипировку от твоего взора.{x", victim);
        return;
    }
    if (victim->getProfession( ) == prof_druid && victim->isAffected(gsn_shapeshift)) {
        ch->pecho("\r\nБоевая трансформация скрывает экипировку от твоего взора.");
        return;
    }

    if (!naked) {
        oldact("\r\n$C1 использует: ", ch, 0, victim, TO_CHAR );
        ch->send_to( buf );
    }
            
    if (victim->is_npc() && victim->getNPC()->behavior)
        if (victim->getNPC()->behavior->look_inv( ch ))
            return;

    if (victim != ch 
            && !ch->is_npc()
            && (number_percent( ) < gsn_peek->getEffective( ch )
                || ch->is_immortal()))

    {
        ch->pecho( "\n\rТы заглядываешь в инвентарь: " );
        gsn_peek->improve( ch, true );
        show_list_to_char( victim->is_mirror() ?
                vict->carrying : victim->carrying, ch, true, true );
    }
}

/*
 * display character equip list
 */
bool show_char_equip( Character *ch, Character *victim, ostringstream &buf, bool fShowEmpty )
{
    Wearlocation::DisplayList eq;
    Wearlocation::DisplayList::iterator e;
    bool naked = true;
    Character *        vict = (victim->is_mirror( ) ? victim->getDoppel( ) : victim);

    wearlocationManager->display( vict, eq );
    
    for (e = eq.begin( ); e != eq.end( ); e++) {
        DLString objName, wearName = e->first;
        Object *obj = e->second;

        if (!obj) {
            if (fShowEmpty)
                objName << "{" << CLR_NOEQ(ch) << "ничего.{x";
            else
                continue;
        }
        else if (!ch->can_see( obj )) {
            if (fShowEmpty)
                objName = "нечто.";
            else
                continue;
        }
        else 
            objName = format_obj_to_char( obj, ch, true );

        ostringstream mbuf;
        if (obj)
            webManipManager->decorateItem( mbuf, objName, obj, ch, DLString::emptyString, 1 );
        else
            mbuf << objName;

        buf << fmt(0, "<%-21s> %s\r\n", wearName.c_str( ), mbuf.str( ).c_str( ) );
        if (obj)
            naked = false;
    }

    return naked;
}
        


/*
 * Show people in the room
 */
static void show_people_to_char( Character *list, Character *ch, bool fShowMount )
{
    Character *rch;
    int life_count=0;

    for ( rch = list; rch != 0; rch = rch->next_in_room )
    {
        if (rch == ch)
            continue;

        if (!fShowMount && rch == ch->mount)
            continue;

        if (!rch->is_npc() && ch->get_trust() < rch->getPC()->invis_level)
            continue;
        
        if (ch->can_see( rch ))
            show_char_to_char_0( rch, ch );
// TODO invis & hide checks
        else if (!rch->is_immortal( )) {
            if (rch->in_room->isDark( ) && IS_AFFECTED(rch, AFF_INFRARED ))
                ch->pecho( "{WТы видишь взгляд {Rпылающих красных глаз{W, следящих за ТОБОЙ!{x" );
                
            life_count++;
        }
    }

    if (life_count && CAN_DETECT(ch, DETECT_LIFE))
        ch->pecho( "Ты чувствуешь присутствие %d жизненн%s форм%s в комнате.",
                    life_count,
                    GET_COUNT(life_count, "ой", "ых", "ых"),
                    GET_COUNT(life_count, "ы", "", ""));
}


/*---------------------------------------------------------------------------
 * extra-description looking (for 'look' and 'read') 
 *--------------------------------------------------------------------------*/
static bool oprog_look( Object *obj, Character *ch, const char *keyword )
{
    FENIA_CALL( obj, "Look", "Cs", ch, keyword );
    FENIA_NDX_CALL( obj, "Look", "OCs", obj, ch, keyword );
    return false;
}

static bool rprog_look( Room *room, Character *ch, const char *keyword )
{
    FENIA_CALL( room, "Look", "Cs", ch, keyword );
    return false;
}

static DLString oprog_extra_descr(Object *obj, Character *ch, const char *arg)
{
    FENIA_STR_CALL( obj, "ExtraDescr", "Cs", ch, arg )
    FENIA_NDX_STR_CALL( obj, "ExtraDescr", "OCs", obj, ch, arg )

    if (obj->behavior)
        return obj->behavior->extraDescription( ch, arg );

    return DLString::emptyString;
}

struct EDInfo {
    EDInfo( DLString k, DLString d, Object *o, Room *r ) :
        keyword( k ), description( d ), source( o ), sourceRoom( r )
    {
    }

    DLString keyword;
    DLString description;
    Object *source;
    Room *sourceRoom;
};

struct ExtraDescList : public list<EDInfo> {
    ExtraDescList( Character *ch, const char *arg, int number = 1 ) 
         : ch( ch ), arg( arg ), number( number )
    {
    }
    
    void putObjects( Object *list ) 
    { 
        for (Object *obj = list; obj != 0; obj = obj->next_content) {
            if (!ch->can_see( obj )) 
                continue;

            size_type startSize = size( );

            DLString desc = oprog_extra_descr( obj, ch, arg );
            if (!desc.empty( ))
                push_back( EDInfo( arg, desc, obj, 0 ) );
                
            size_type mySize = size( );
            putDescriptions( obj->extraDescriptions, obj, 0 );

            if (size( ) == mySize)
                putDescriptions( obj->pIndexData->extraDescriptions, obj, 0 );
            
            if (size( ) == startSize)
                putDefaultDescription( obj );
        }
    }
    
    void putDefaultDescription( Object *obj )
    {
        if (obj->getKeyword().matchesUnstrict(arg)) {
            DLString defaultDescr;
            if (obj->in_room)
                defaultDescr = obj->getDescription(LANG_DEFAULT);
            else
                defaultDescr = "Ты не видишь здесь ничего особенного.";
                
            push_back( EDInfo(String::toString(obj->getKeyword()), defaultDescr, obj, 0 ) );
        }
    }

    void putDescriptions( const ExtraDescrList &edList, Object *obj, Room *room )
    {
        for (auto &ed: edList)
            if (is_name( arg, ed->keyword.c_str() ))
                push_back( EDInfo( ed->keyword, ed->description.get(LANG_DEFAULT), obj, room ) );
    }
    
    bool output( )
    {
        int count;
        iterator i;
        ostringstream buf;

        for (count = 1, i = begin( ); i != end( ); i++, count++)
            if (count == number) {
                const ExtraDescrList &sourceEdList = i->source ? i->source->pIndexData->extraDescriptions : i->sourceRoom->getExtraDescr();
                buf << "{x";
                webManipManager->decorateExtraDescr( buf, i->description.c_str( ), sourceEdList, ch );
                buf << endl;

                ch->send_to( buf );

                if (i->source)
                    oprog_look( i->source, ch, i->keyword.c_str( ) );
                if (i->sourceRoom)
                    rprog_look( i->sourceRoom, ch, i->keyword.c_str( ) );
                return true;
            }

        return false;
    }

    Character *ch;
    const char *arg;
    int number;
};

bool do_look_extradescr( Character *ch, const char *arg, int number )
{
    ExtraDescList edlist( ch, arg, number );
    
    edlist.putObjects( ch->carrying );
    edlist.putObjects( ch->in_room->contents );
    edlist.putDescriptions( ch->in_room->getExtraDescr(), 0, ch->in_room );

    return edlist.output( );
}

/*---------------------------------------------------------------------------
 * 'look' subroutines 
 *--------------------------------------------------------------------------*/
static DLString rprog_descr( Room *room, Character *ch, const DLString &descr )
{   
    FENIA_STR_CALL( room, "Descr", "Cs", ch, descr.c_str( ) )
    return descr;
}

static DLString rprog_eexit_descr( Room *room, EXTRA_EXIT_DATA *peexit, Character *ch, const DLString &descr )
{   
    FENIA_STR_CALL( room, "ExtraExitDescr", "sCs", peexit->keyword, ch, descr.c_str( ) )
    return descr;
}

static void do_look_auto( Character *ch, Room *room, bool fBrief, bool fShowMount )
{
    ostringstream buf;

    if (eyes_darkened( ch )) {
        ch->pecho( "Здесь слишком темно... " );
        show_people_to_char( room->people, ch, fShowMount );
        return;
    }
    
    buf << "{" << CLR_RNAME(ch) << room->getName() << "{x";

    if (ch->getConfig( ).holy) 
        buf << " {" << CLR_RVNUM(ch) << "[Room " << room->vnum
            << "][" << room->areaName() << "]{x";

    buf << " " << web_edit_button(ch, "redit|show", room->vnum);
    
    buf << endl;

    if (!fBrief)
    {
        ostringstream rbuf;
        const char *dsc = room->getDescription();

        if (*dsc == '.')
            ++dsc;
        else
            rbuf << " ";
        
        webManipManager->decorateExtraDescr( rbuf, dsc, room->getExtraDescr(), ch );

        for (auto &peexit: room->extra_exits)
            if (ch->can_see( peexit ))
                rbuf << rprog_eexit_descr(room, peexit, ch, peexit->room_description.get(LANG_DEFAULT));

        buf << rprog_descr( room, ch, rbuf.str( ) );
    }

    show_room_affects_to_char(room, ch, buf);

    if (ch->getPC( ) && IS_SET(ch->getPC( )->act, PLR_AUTOEXIT))
    {
        buf << endl;
        ch->send_to( buf );
        show_exits_to_char( ch, room );
    }
    else
        ch->send_to( buf );
    
    show_list_to_char( room->contents, ch, false, false );
    show_people_to_char( room->people, ch, fShowMount );
}

static void do_look_move( Character *ch, bool fBrief )
{
    if (!ch->is_npc( ) && ch->getPC( )->getAttributes( ).isAvailable( "speedwalk" )) {
        if (eyes_darkened( ch ))
            ch->pecho( "Здесь слишком темно... " );
        else
            ch->pecho( "{W%s{x", ch->in_room->getName() );
        return;
    }

    do_look_auto( ch, ch->in_room, fBrief, false );
}


static void afprog_look( Character *looker, Character *victim )
{
    for (auto &paf: victim->affected.findAllWithHandler())
        if (paf->type->getAffect())
            paf->type->getAffect( )->onLook(SpellTarget::Pointer(NEW, victim), paf, looker);
}

static void mprog_look(Character *looker, Character *victim)
{
    FENIA_VOID_CALL( looker, "Look", "CC", looker, victim );
    FENIA_NDX_VOID_CALL( looker->getNPC(), "Look", "CCC", looker, looker, victim );
    FENIA_VOID_CALL( victim, "Look", "CC", looker, victim );
    FENIA_NDX_VOID_CALL( victim->getNPC(), "Look", "CCC", victim, looker, victim );
}

static void do_look_character( Character *ch, Character *victim )
{
    if (victim->can_see( ch )) {
        if (ch == victim)
            oldact("$c1 смотрит на себя.",ch,0,0,TO_ROOM);
        else
        {
            oldact("$c1 смотрит на тебя.", ch, 0, victim, TO_VICT);
            oldact("$c1 смотрит на $C4.",  ch, 0, victim, TO_NOTVICT);
        }
    }

    show_char_to_char_1( victim, ch, false );
    afprog_look( ch, victim );    
    mprog_look(ch, victim);
}

static bool do_look_direction( Character *ch, const char *arg1 )
{
    EXIT_DATA *pexit;
    int door;
    
    door = find_exit( ch, arg1, FEX_NONE );

    if (door < 0)
        return false;

    if ( ( pexit = ch->in_room->exit[door] ) == 0 )
    {
            ch->pecho( "Ничего особенного тут." );
            return true;
    }

    if (!pexit->description.get(LANG_DEFAULT).empty()) {
            ch->send_to( pexit->description.get(LANG_DEFAULT));
            ch->pecho("");
    }
    else
            ch->pecho( "Здесь нет ничего особенного." );

    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        oldact("$N1: тут закрыто.", ch, 0, direction_doorname(pexit), TO_CHAR);
    else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
        oldact("$N1: тут открыто.", ch, 0, direction_doorname(pexit), TO_CHAR);
    
    DoorKeyhole( ch, ch->in_room, door ).doExamine( );
    return true;
}

// TODO show act msg, item type and wear.
static void do_look_object( Character *ch, Object *obj )
{
        ostringstream buf;
            
        buf << "Ты смотришь на {c" << obj->getShortDescr( '4', LANG_DEFAULT ) << "{x."
            << " Это {W" << item_table.message(obj->item_type) << "{x";


        for (int i = 0; i < wearlocationManager->size( ); i++) {
            Wearlocation *loc = wearlocationManager->find( i );
            if (loc->matches( obj ) && !loc->getPurpose().empty()) {
                buf << ", " << loc->getPurpose( ).toLower( );
                break;
            }
        }

        buf << "." << endl;
        ch->send_to( buf.str( ) );

        DLString desc = oprog_extra_descr( obj, ch, String::toString(obj->getKeyword()).c_str() );
        DLString keywords = String::toString(obj->getKeyword());

        if (desc.empty( )) { 
            for (auto &ed: obj->extraDescriptions)
                if (arg_contains_someof( ed->keyword, keywords.c_str() )) {
                    desc = ed->description.get(LANG_DEFAULT);
                    break;
                }
        }

        if (desc.empty( )) { 
            for (auto &ed: obj->pIndexData->extraDescriptions)
                if (arg_contains_someof( ed->keyword, keywords.c_str() )) {
                    desc = ed->description.get(LANG_DEFAULT);
                    break;
                }
        }

        if (desc.empty( )) {
            if (obj->in_room)
                desc = obj->getDescription(LANG_DEFAULT);
            else
                desc = "Ты не видишь здесь ничего особенного.";
        }            

        ostringstream descBuf;
        webManipManager->decorateExtraDescr( descBuf, desc.c_str( ), obj->pIndexData->extraDescriptions, ch );
        ch->send_to( descBuf );

        oprog_look( obj, ch, keywords.c_str() );
}

static bool do_look_extraexit( Character *ch, const char *arg3 )
{
    EXTRA_EXIT_DATA * peexit = ch->in_room->extra_exits.find(arg3);

    if (!peexit)
        return false;

    if (!peexit->description.empty()
            && ch->can_see( peexit ) )
            ch->send_to( peexit->description.get(LANG_DEFAULT));
    else
            ch->pecho( "Здесь нет ничего особенного." );
    
    if (!peexit->short_desc_from.empty()
        && ch->can_see( peexit ) )
    {
            if ( IS_SET(peexit->exit_info, EX_CLOSED) )
            {
                ch->pecho( "%1$N1: тут закрыто.", peexit->short_desc_from.get(LANG_DEFAULT).c_str() );
            }
            else if ( IS_SET(peexit->exit_info, EX_ISDOOR) )
            {
                ch->pecho( "%1$N1: тут открыто.", peexit->short_desc_from.get(LANG_DEFAULT).c_str()  );
            }
    }
    
    ExtraExitKeyhole( ch, ch->in_room, peexit ).doExamine( );
    return true;
}

/*-------------------------------------------------------------------------
 * 'look' command 
 *-------------------------------------------------------------------------*/
CMDRUNP( look )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    Character *victim;
    int number;
    bool fAuto, fMove, fBrief, fShowMount;

    if (ch->desc == 0)
        return;

    if (ch->position < POS_SLEEPING) {
        ch->pecho( "Ты не видишь ничего, кроме звезд!" );
        return;
    }

    if (ch->position == POS_SLEEPING) {
        ch->pecho( "Ты ничего не видишь, ты спишь!" );
        return;
    }
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    fAuto = !str_cmp( arg1, "auto" );
    fMove = !str_cmp( arg1, "move" );
    fBrief = !ch->is_npc( ) && IS_SET(ch->comm, COMM_BRIEF);
    fShowMount = true;

    if (eyes_blinded( ch )) {
        if (fAuto || fMove)
            ch->pecho( "Ты не можешь видеть вещи!" );
        else
            eyes_blinded_msg( ch );

        return;
    }

    if (arg1[0] == '\0') {
        do_look_auto( ch, ch->in_room, false, fShowMount );
        return;
    }
    
    if (fAuto) {
        do_look_auto( ch, ch->in_room, fBrief, fShowMount );
        return;
    }

    if (fMove) {
        do_look_move( ch, fBrief );
        return;
    }

    if (eyes_darkened( ch )) {
        ch->pecho( "Тебе не удается ничего разглядеть в кромешной темноте." );
        return;
    }
    
    if (arg_is_in( arg1 ) || arg_is_on( arg1 )) {
        do_look_into( ch, arg2 );
        return;
    }

    if (( victim = get_char_room( ch, arg1 ) ) != 0) {
        do_look_character( ch, victim );
        return;
    }

    Object *obj;
    if (get_arg_id( arg1 ) && (obj = get_obj_here( ch, arg1 ))) {
        do_look_object( ch, obj );
        return;
    }
    
    if (do_look_extradescr( ch, arg3, number ))
        return;

    if (do_look_extraexit( ch, arg3 ))
        return;

    if (do_look_direction( ch, arg1 ))
        return;

    ch->pecho( "Ты не видишь этого тут." );
}


