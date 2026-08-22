#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "pcrace.h"
#include "grammar_entities_impl.h"
#include "act.h"
#include "commandtemplate.h"
#include "xmlkillingattribute.h"
#include "player_utils.h"
#include "vnum.h"
#include "xmlattributeticker.h"
#include "alignment.h"
#include "religion.h"
#include "desire.h"
#include "playerattributes.h"
#include "interp.h"
#include "arg_utils.h"
#include "stats_apply.h"
#include "mudtags.h"
#include "string_utils.h"
#include "char_weight.h"
#include "msgformatter.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

RELIG(none);
PROF(samurai);

/* Four whole sentences rather than one assembled from pieces. A catalog key has
 * to be a fixed literal, and the old version built its format string at runtime
 * out of four fragments, so nothing here could ever be looked up. The branches
 * pick exactly what the concatenation used to produce, including the detail that
 * the coin-count placeholder follows whichever number is actually present.
 * fmt's first argument was NULL, i.e. no viewer at all, so even the Russian
 * plural forms were resolved blind; it is the caller's character now. */
static DLString show_money( Character *ch, int g, int s )
{
    if (g > 0 && s > 0)
        return fmt( ch, _("{Y%1$d{x золот%1$Iая|ые|ых и {W%2$d{x серебрян%2$Iая|ые|ых моне%2$Iта|ты|т."), g, s );

    if (g > 0)
        return fmt( ch, _("{Y%1$d{x золот%1$Iая|ые|ых моне%1$Iта|ты|т."), g );

    if (s > 0)
        return fmt( ch, _("{W%1$d{x серебрян%1$Iая|ые|ых моне%1$Iта|ты|т."), s );

    return fmt( ch, _("нет денег.") );
}

static DLString show_experience( PCharacter *ch )
{
    return fmt( ch, _("У тебя %1$d очк%1$Iо|а|ов опыта. "
               "До следующего уровня осталось %2$d очк%2$Iо|а|ов из %3$d."),
               ch->exp.getValue( ),
               ch->getExpToLevel( ),
               ch->getExpPerLevel( ch->getLevel( ) + 1 ) - ch->getExpPerLevel( ) );
}

CMDRUNP( worth )
{
    ch->send_to( l(ch, "У тебя ") );
    ch->pecho( show_money( ch, ch->gold, ch->silver ) );

    if ( ch->is_npc() )
            return;

    ch->pecho( show_experience( ch->getPC( ) ) );

    auto killed = ch->getPC()->getAttributes().getAttr<XMLKillingAttribute>("killed");

    // Numbered on purpose: English needs no gender and drops %1$G, and with
    // sequential args that would shift %d onto the Character* and %s onto a
    // kill count -- an int read as char*. See the same fix in remort/cmlt.cpp.
    ch->pecho(_("Ты уби%1$Gло|л|ла {Y%2$d{x %3$s, {W%4$d{x %5$s и {r%6$d{x %7$s персонажей."),
            ch, 
            killed->align[N_ALIGN_GOOD], l(ch, "добрых"),
            killed->align[N_ALIGN_NEUTRAL], l(ch, "нейтральных"),
            killed->align[N_ALIGN_EVIL], l(ch, "злых"));
}


#define MAX_MSGTABLE_SIZE 25
struct msgpair_t {
    int value;
    const char *msg;
};
typedef msgpair_t msgtable_t [MAX_MSGTABLE_SIZE];

const char * msgtable_lookup( const msgtable_t &table, int value )
{
    for (int i = 0; table[i].value != -1; i++)
        if (table[i].value > value)
            return (i == 0 ? table[i].msg : table[i-1].msg);
        else if (table[i].value == value)
            return table[i].msg;
        else if (table[i+1].value == -1)
            return table[i].msg;


    return "";
}

msgtable_t msg_positions = {
    { POS_DEAD,     "Ты ТРУП!!!"                  },
    { POS_MORTAL,   "Ты при смерти."               },
    { POS_INCAP,    "Ты в беспомощном состоянии." },
    { POS_STUNNED,  "Тебя оглушили."              },
    { POS_SLEEPING, "Ты спишь."                   },
    { POS_RESTING,  "Ты отдыхаешь."               },
    { POS_SITTING,  "Ты сидишь."                  },
    { POS_FIGHTING, "Ты сражаешься."              },
    { POS_STANDING, "Ты стоишь."                  },
    { -1 }
};


// Classic linear ("prose") score -- a comprehensive, screen-reader-friendly
// dump of every stat. Used as the fall-through body for the accessible 'score'
// command and for NPCs; players normally get the Fenia panel or score_ascii.
static void score_prose( Character *ch )
{
    ostringstream buf;
    Room *room = 0;
    PCharacter *pch = ch->getPC( );

    buf << fmt( ch, _("Ты {W%1$s%2$s{x, уровень {C%3$d{w"),
                   ch->seeName( ch, '1' ).c_str( ),
                   ch->is_npc( ) ? "" : Player::title(ch->getPC( ), Player::displayLang(ch)).c_str( ),
                   ch->getRealLevel( ));
    
    if (!ch->is_npc( ))
        buf << fmt( ch, _(", тебе %1$d %1$Iгод|года|лет (%2$d ча%2$Iс|са|сов)."),
                        pch->age.getYears( ), pch->age.getHours( ) ); 
    
    buf << endl;

    if (ch->getRealLevel( ) != ch->get_trust( ))
        buf << fmt( ch, _("Уровень доверия к тебе составляет %1$d."), ch->get_trust( ) ) << endl;

    // One whole line rather than streamed label fragments: the catalog is keyed on
    // the finished Russian string, so fragments can never resolve. Same wording as
    // the Fenia score (command/score/runFunc), which is the panel players actually
    // see -- this prose version is the no-Fenia fall-through and `oscore` on an NPC.
    buf << fmt( ch, _("{wРаса:{W %1$s  {wРазмер:{W %2$s  {wПол:{W %3$s  {wКласс:{W %4$s"),
                ch->getRace( )->getNameFor( ch, ch ).ruscase('1').c_str( ),
                size_table.message( ch->size, '1', Player::displayLang(ch) ).c_str( ),
                sex_table.message( ch->getSex( ), '1', Player::displayLang(ch) ).c_str( ),
                ch->getProfession( )->getNameFor( ch ).c_str( ) );

    if (!ch->is_npc( ))
        room = get_room_instance( ch->getPC()->getHometown( )->getAltar() );
    else
        room = get_room_instance( ROOM_VNUM_TEMPLE );

    buf << fmt( ch, _("  {wДом:{W %1$s{x"),
                room ? room->areaName( Player::displayLang(ch) ).c_str( )
                     : fmt( ch, _("Потерян") ).c_str( ) ) << endl
        << fmt(ch, _("У тебя {R%d{x/{r%d{x жизни, {C%d{x/{c%d{x энергии и %d/%d движения.\n\r"),
                    ch->hit.getValue( ), ch->max_hit.getValue( ), 
                    ch->mana.getValue( ), ch->max_mana.getValue( ), 
                    ch->move.getValue( ), ch->max_move.getValue( ));
    
    if (!ch->is_npc( )) 
        buf << fmt( ch, _("У тебя {Y%1$d{x практи%1$Iка|ки|к и {Y%2$d{x тренировочн%2$Iая|ые|ых сесси%2$Iя|и|й."),
                       pch->practice.getValue( ), pch->train.getValue( ) )
            << endl;
    
    buf << fmt(ch, _("Ты несешь {W%d/%d{x вещей с весом {W%d/%d{x фунтов.\n\r"),
                ch->carry_number, Char::canCarryNumber(ch),
                Char::getCarryWeight(ch)/10, Char::canCarryWeight(ch)/10 );

    if (ch->is_npc( )) {
        buf << fmt(ch, 
            _("Твои параметры: исходные, (текущие)\n\r"
            "      Сила : %d(%d)    Интеллект : %d(%d)\n\r"
            "  Мудрость : %d(%d)     Ловкость : %d(%d)\n\r"
            "  Сложение : %d(%d)      Обаяние : %d(%d)\n\r"),
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR),
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT),
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS),
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX),
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON),
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA) );

    } else {
        buf << fmt(ch, 
            _("Твои параметры: исходные, {c({Wтекущие{c){x, [{Cмаксимальные{x]\n\r"
            "      Сила: %d{c({W%d{c){x [{C%d{x]   Интеллект: %d{c({W%d{c){x [{C%d{x]\n\r"
            "  Мудрость: %d{c({W%d{c){x [{C%d{x]    Ловкость: %d{c({W%d{c){x [{C%d{x]\n\r"
            "  Сложение: %d{c({W%d{c){x [{C%d{x]     Обаяние: %d{c({W%d{c){x [{C%d{x]\n\r"),
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR), pch->getMaxStat(STAT_STR),
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT), pch->getMaxStat(STAT_INT),
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS), pch->getMaxStat(STAT_WIS),
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX), pch->getMaxStat(STAT_DEX),
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON), pch->getMaxStat(STAT_CON),
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA), pch->getMaxStat(STAT_CHA) );

    }

    buf << fmt(ch, _("У тебя {W%d{x очков опыта, и %s\n\r"),
                  ch->exp.getValue( ),
                  show_money( ch, ch->gold, ch->silver ).c_str( ) );

    /* KIO shows exp to level */
    if (!ch->is_npc() && ch->getRealLevel( ) < LEVEL_HERO - 1)
        buf << fmt(ch, _("Тебе нужно набрать {W%d{x очков опыта до следующего уровня.\n\r"),
                    ch->getPC()->getExpToLevel( ) );

    if (!ch->is_npc( )) {
        XMLAttributeTimer::Pointer qd = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
        int qtime = qd ? qd->getTime( ) : 0;
        bool hasQuest = pch->getAttributes( ).isAvailable( "quest" );
        
        buf << fmt( ch, _("У тебя {Y%1$d{x квестов%1$Iая|ые|ых едини%1$Iца|цы|ц. "),
                       pch->getQuestPoints() );
        if (qtime == 0)
            buf << l(ch, "У тебя сейчас нет задания.");
        else
            buf << fmt( ch, _("До %1$s квеста осталось {Y%2$d{x ти%2$Iк|ка|ков."),
                       l(ch, hasQuest ? "конца" : "следующего"),
                       qtime );

        buf << endl;

        bool newline = false;

        if (ch->getProfession( ) != prof_samurai) {
            if (ch->wimpy > 0) {
                buf << fmt(ch, _("Ты попытаешься убежать при %d жизни.  "), ch->wimpy.getValue( ) );
                newline = true;
            }
        } else {
            if (ch->getPC()->death > 0)
                buf << fmt(ch, _("Тебя убили уже {r%1$d{x ра%1$Iз|за|з."), ch->getPC()->death.getValue());
            else
                buf << l(ch, "Тебя еще ни разу не убивали.");
            newline = true;
        }
        
        if (ch->getPC()->guarding != 0) {
            buf << fmt(ch, _("Ты охраняешь: %s. "), ch->seeName( ch->getPC()->guarding, '4' ).c_str( ) );
            newline = true;
        }

        if (ch->getPC()->guarded_by != 0) {
            buf << fmt(ch, _("Ты охраняешься: %s."), ch->seeName( ch->getPC()->guarded_by, '5' ).c_str( ) );
            newline = true;
        }
        
        if (newline)
            buf << endl;
    }

    // Report current desire status, as progress bar or percents.
    if (!ch->is_npc( )) {
        ostringstream dbuf;

        for (int i = 0; i < desireManager->size( ); i++) {
            ostringstream b;
            
            desireManager->find( i )->report( ch->getPC( ), b );
            
            if (!b.str( ).empty( ))
                dbuf << b.str( ) << " ";
        }

        if (!dbuf.str( ).empty( ))
            buf << dbuf.str( ) << endl;
    }
    
    buf << l(ch, msgtable_lookup( msg_positions, ch->position ));

    if (ch->is_adrenalined( ) && ch->position > POS_INCAP)
        buf << " " << l(ch, "Твоя кровь полна адреналина!");
    
    buf << endl;

    /* print AC values */
    buf << fmt(ch, _("Защита от укола {W%d{x, от удара {W%d{x, от разрезания {W%d{x, от экзотики {W%d{x.\n\r"),
            GET_AC(ch,AC_PIERCE),
            GET_AC(ch,AC_BASH),
            GET_AC(ch,AC_SLASH),
            GET_AC(ch,AC_EXOTIC));
    buf << fmt(ch, _("Точность: {C%d{x  Урон: {C%d{x  Защита от заклинаний: {C%d{x\n\r"),
                ch->hitroll.getValue( ), ch->damroll.getValue( ), ch->saving_throw.getValue( ) );

    buf << fmt(ch, _("У тебя %s натура.  "), align_name( ch, viewerLang(ch) ).ruscase( '1' ).c_str( ) );
    
    switch (ch->ethos.getValue( )) {
    case ETHOS_LAWFUL:
            buf << l(ch, "Ты почитаешь порядок.\n\r");
            break;
    case ETHOS_NEUTRAL:
            buf << l(ch, "У тебя нейтральный этос.\n\r");
            break;
    case ETHOS_CHAOTIC:
            buf << l(ch, "Ты хаотик.\n\r");
            break;
    default:
            if (!ch->is_npc( ))
                buf << l(ch, "У тебя нет этоса, сообщи об этом Богам!\n\r");
            else
                buf << "\n\r";
            break;
    }
    
    if (!ch->is_npc( )) {
        if (ch->getReligion( ) == god_none)
            buf << fmt(ch, _("Ты не определил%Gось|ся|ась с выбором религии.  "), ch);
        else
            buf << fmt(ch, _("Твоя религия: {C%s{x.  "), ch->getReligion( )->getNameFor( ch ).ruscase( '1' ).c_str( ));
        
        buf << fmt(ch, _("Твои заслуги перед законом:  %d.\n\r"), ch->getPC( )->getLoyalty());

        auto killed = ch->getPC()->getAttributes().getAttr<XMLKillingAttribute>("killed");

       buf << fmt(ch, _("Ты уби%1$Gло|л|ла {Y%2$d{x %3$s, {W%4$d{x %5$s и {r%6$d{x %7$s персонажей.\n\r"),
                        ch, 
                        killed->align[N_ALIGN_GOOD], l(ch, "добрых"),
                        killed->align[N_ALIGN_NEUTRAL], l(ch, "нейтральных"),
                        killed->align[N_ALIGN_EVIL], l(ch, "злых"));
    }
    
    /* RT wizinvis and holy light */
    if (ch->is_immortal( )) 
        buf << fmt(ch, _("Божественный взор %s. Невидимость %d уровня, инкогнито %d уровня."),
                   l(ch, IS_SET(ch->act, PLR_HOLYLIGHT) ? "включен" : "выключен"),
                   ch->getPC( )->invis_level.getValue( ),
                   ch->getPC( )->incog_level.getValue( ) )
            << endl;

    // Collect information from various attributes, such as craft professions.    
    if (pch) {
        list<DLString> attrLines;
        if (pch->getAttributes( ).handleEvent( ScoreArguments( pch, attrLines ) ))
                for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
                                buf << *l << endl;
                        }
    }

    if (IS_GHOST(ch)) {
        buf << fmt(ch, _("{xТы призрак и обретёшь плоть через {Y%1$d {xсекун%1$Iду|ды|д."),
                 pch->ghost_time*(PULSE_MOBILE/dreamland->getPulsePerSecond()))
        << endl;
    }

    ch->send_to( buf );

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "nocolor noempty" );
}

/**
 * Output each score piece separately, as requested by the argument.
 */
static void do_score_args(Character *ch, const DLString &arg)
{
    PCharacter *pch = ch->getPC();
    if (!pch)
        return;

    /* Match what the player typed against the stat's name in EVERY language, not
     * just the English bit name and the Russian pad baked into the binary. A
     * Ukrainian typing "інт" got nothing at all, and the one-line "ум" special
     * case below was the shape of the problem: a synonym hardcoded because there
     * was nowhere else to put it.
     *
     * stat_table.message() resolves through flagmessages.json, which already
     * carries all six stats in Ukrainian (інтелект, мудрість, спритність...),
     * so this works today rather than waiting on data.
     *
     * The Russian pad stays matchable for everyone, which is both the historical
     * behaviour and the point: somebody who switched their config to Ukrainian
     * still has the Russian word in their fingers, the same argument the card
     * makes for keeping the old movement aliases.
     *
     * The viewer's OWN language is the only thing added, deliberately, rather
     * than every language to everybody. This block runs before the rest of the
     * command, and it prefix-matches, so each word it learns can swallow a short
     * abbreviation that a handler further down was already answering -- Ukrainian
     * "статура" shadows "стать" at ст/стат. Confining the new vocabulary to the
     * reader's own language means a Russian or English player sees no change at
     * all, and the collision is limited to the people the feature is for. The
     * precedence question it raises (does a two-letter prefix mean the stat or
     * the gender) is a card, not something to settle inside this loop. */
    lang_t viewerLanguage = Player::displayLang( ch );
    int stat = -1;

    for (int i = 0; i < stat_table.size && stat < 0; i++) {
        if (arg.strPrefix( stat_table.fields[i].name )) {
            stat = i;
            break;
        }

        const lang_t langs[] = { LANG_RU, viewerLanguage };

        // Nominative and accusative, the two the pad is asked for elsewhere.
        for (int l = 0; l < 2 && stat < 0; l++)
            for (const char *gcase = "14"; *gcase; gcase++) {
                DLString pad = stat_table.message( stat_table.fields[i].value, *gcase, langs[l] );

                if (!pad.empty( ) && arg.strPrefix( pad )) {
                    stat = i;
                    break;
                }
            }
    }

    /* The frame's own stat labels, which are not prefixes of anything above.
     *
     * oscore abbreviates them to fit a four-column cell, and an abbreviation is
     * not a prefix -- that is why "ум" was a hardcoded case in the first place:
     * the Russian frame says "Ум  :" while the word is "интеллект". Ukrainian
     * has the same problem four times over, because its labels were abbreviated
     * differently from its pads: the frame reads Розум, Спрт, Тіло and Чари
     * while the pads are інтелект, спритність, статура and харизма. So a
     * Ukrainian typing the word they are looking at got nothing for four of the
     * six stats. Only Сила and Мудр happened to line up.
     *
     * Matched EXACTLY, not by prefix, which is what the old "ум" case did. A
     * prefix here reaches past the stats: everything further down this function
     * matches its own localized synonyms, so "у" and "р" were answering with the
     * level and "т" with the training count. Those are one keystroke and people
     * use them. Exact match costs nothing the paragraph above asks for -- the
     * frame prints these strings whole, so typing one whole is exactly "typing
     * the word you are looking at". */
    static const struct { const char *label; int stat; } frameLabels[] = {
        { "ум",    STAT_INT },   // RU frame "Ум  :"
        { "розум", STAT_INT },   // UA frame "Розум:"
        { "спрт",  STAT_DEX },   // UA frame "Спрт:"
        { "тіло",  STAT_CON },   // UA frame "Тіло:"
        { "чари",  STAT_CHA },   // UA frame "Чари:"
        { 0, 0 }
    };

    for (int i = 0; stat < 0 && frameLabels[i].label; i++)
        if (arg == frameLabels[i].label)
            stat = frameLabels[i].stat;

    if (stat >=0) {
        ch->pecho(_("%^N1 %d (%d), максимум %d."), 
            stat_table.fields[stat].message, ch->perm_stat[stat], 
            ch->getCurrStat(stat), pch->getMaxStat(stat));
        return;
    }
   
    if (arg_is(arg, "hp")) {
        ch->pecho(_("Здоровье %d из %d."), ch->hit, ch->max_hit);
        return;
    } 
    if (arg_is(arg, "mana")) {
        ch->pecho(_("Мана %d из %d."), ch->mana, ch->max_mana);
        return;
    } 
    if (arg_is(arg, "moves")) {
        ch->pecho(_("Шагов %d из %d."), ch->move, ch->max_move);
        return;
    } 
    if (arg_is(arg, "level")) {
        ch->pecho(_("Уровень %d."), ch->getRealLevel());
        return;
    } 
    if (arg_is(arg, "race")) {
        if (ch->getRace()->isPC()) {
            PCRace::Pointer pcRace = ch->getRace()->getPC(); 
            ch->pecho(_("Ты %N1."), GET_SEX(ch,
                            pcRace->getMaleName().c_str(),
                            pcRace->getMaleName().c_str(),
                            pcRace->getFemaleName().c_str()));
        }
        return;
    } 
    if (arg_is(arg, "sex")) {   
        ch->pecho(_("Пол %s."), ch->getSex( ) == 0 ? l(ch, "потерян") : sex_table.message( ch->getSex( ), '1', Player::displayLang(ch) ).c_str( ));
        return;
    }
    if (arg_is(arg, "class")) {
        ch->pecho(_("Ты %N1."), ch->getProfession()->getRusName().c_str());
        return;
    } 
    if (arg_is(arg, "align")) {
        ch->pecho(_("У тебя %s натура."), align_name_short(ch, Grammar::MultiGender::FEMININE));
        return;
    } 
    if (arg_is(arg, "ethos")) {
        ch->pecho(_("У тебя %s этос."), ethos_table.message(ch->ethos, '1', Player::displayLang(ch)).c_str());
        return;
    } 
    if (arg_is(arg, "hometown")) {
        Room *room = get_room_instance(pch->getHometown()->getAltar());
        ch->pecho(_("Твой дом - %s."),
                  room ? room->areaName(Player::displayLang(ch)).c_str()
                       : fmt(ch, _("потерян")).c_str());
        return;
    } 
    if (arg_is(arg, "religion")) {
        if (ch->getReligion() == god_none)
            ch->pecho(_("Ты не определил%Gось|ся|ась с выбором религии."), ch);
        else
            ch->pecho(_("Религия %s."), ch->getReligion()->getRussianName().ruscase('1').c_str());
        return;
    } 
    if (arg_is(arg, "practice")) {
        ch->pecho(_("Практик %d."), pch->practice);
        return;
    } 
    if (arg_is(arg, "train")) {
        ch->pecho(_("Тренировки %d."), pch->train);
        return;
    } 
    if (!str_prefix("quest", arg.c_str()) || !str_prefix("квест", arg.c_str())) {
        ch->pecho(_("Используй команды {y{hcквест время{x и {y{hcквест очки{x."));
        return;
    } 
    if (arg_is(arg, "wimpy")) {
        ch->pecho(_("Трусость %d."), ch->wimpy);
        return;
    } 
    if (arg_is(arg, "death")) {
        ch->pecho(_("Смертей %d."), pch->death);
        return;
    } 
    if (arg_is(arg, "position")) {
        ch->pecho(l(ch, msgtable_lookup(msg_positions, ch->position)));
        return;
    }
    if (arg_is(arg, "gold")) {
        ch->pecho(_("Золота %d."), ch->gold);
        return;
    } 
    if (arg_is(arg, "silver")) {
        ch->pecho(_("Серебра %d."), ch->silver);
        return;
    } 
    if (arg_is(arg, "weight")) {
        ch->pecho(_("Вес %d из %d."), Char::getCarryWeight(ch)/10, Char::canCarryWeight(ch)/10);
        return;
    } 
    if (arg_is(arg, "items")) {
        ch->pecho(_("Вещи %d из %d."), ch->carry_number, Char::canCarryNumber(ch));
        return;
    } 
    if (arg_is(arg, "exp")) {
        ch->pecho(_("Опыта до уровня %d."), pch->getExpToLevel());
        return;
    }
    if (arg_is(arg, "age")) {
        ch->pecho(_("Возраст %d."), pch->age.getYears());
        return;
    }

    if (arg_is(arg, "hr")) {
        ch->pecho(_("Точность %d."), ch->hitroll);
        return;
    } 
    if (arg_is(arg, "dr")) {
        ch->pecho(_("Урон %d."), ch->damroll);
        return;
    } 
    if (arg_is(arg, "ac")) {
        ch->pecho(_("Защита от уколов %d, ударов %d, разрезов %d, экзотики %d."), 
                    GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
                    GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));
        return;
    }
    if (arg_is(arg, "saves")) {
        ch->pecho(_("Защита от заклинаний %d."), ch->saving_throw);
        return;
    }
   
    ch->pecho(_("Такого параметра не существует или он скрыт от тебя, попробуй что-то еще.")); 
}


#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

// The classic framed ("ASCII-art") score. Player-only: the caller must guard
// is_npc() first, because pch is dereferenced throughout.
static void score_ascii( Character *ch )
{
    int ekle=0;
    PCharacter *pch = ch->getPC( );
    
    const char *CLR_FRAME = MILD(ch) ? "{Y" : "{G";
    const char *CLR_BAR   = MILD(ch) ? "{D" : "{C";
    const char *CLR_CAPT  = MILD(ch) ? "{g" : "{R";

    // Players only: NPC and single-parameter paths are handled by the CMDRUNP
    // wrappers before they reach this renderer.
    
    XMLAttributeTimer::Pointer qd = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
    int age = pch->age.getYears( );
    Room *room = get_room_instance( pch->getHometown( )->getAltar( ) );
    DLString profName = ch->getProfession( )->getNameFor( ch );

    ostringstream name;
    DLString title = Player::title(pch, Player::displayLang(ch));
    name << ch->seeName( ch, '1' ) << "{x ";
    mudtags_convert(title.c_str( ), name, TAGS_CONVERT_VIS, ch);

    DLString ethos = ethos_table.message( ch->ethos, '1', Player::displayLang(ch) );


    ch->pecho( 
"%s\n\r"
"      /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\", 
             CLR_FRAME);
    /* The age word was GET_COUNT(age, "год", "года", "лет") -- the Russian
     * three-way plural, straight into the frame, Russian for every reader. The
     * %I alternation does the same job through the catalog, so English and
     * Ukrainian get their own forms; the numbered placeholder picks a form
     * without printing the number, which is what this cell needs.
     *
     * The cell had to grow: %4s is a MINIMUM width, not a truncation, and
     * Ukrainian "років" is five characters -- it would have pushed the right
     * edge of the box out by one on that one line. %-6s with a single trailing
     * space spans the same 65 columns the frame's tildes do, so every form from
     * "год" to "років" now fits without moving anything. */
    DLString ageWord = fmt( ch, _("%1$Iгод|года|лет"), age );

    ch->pecho(
        fmt ( 0, "     %s|   %s%-50.50s {y%3d{x %-6s %s|____|",
                CLR_FRAME,
                CLR_CAPT,
                name.str( ).c_str( ),
                age,
                ageWord.c_str( ),
                CLR_FRAME ) );
                
        
    ch->pecho(
_("     %s|%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r" 
"     | %sУровень:{x  %3d        %s|%s Сила:{x %2d{c({x%2d{c){x {C%2d{x %s| %sРелигия:{x %-14.14s%s|\n\r"
"     | %sРаса :{x  %-12s %s| %sУм  :{x %2d{c({x%2d{c){x {C%2d{x %s| %sПрактик   :{x   %3d      %s|\n\r"
"     | %sПол  :{x  %-11s  %s| %sМудр:{x %2d{c({x%2d{c){x {C%2d{x %s| %sТренировок:{x   %3d      %s|\n\r"
"     | %sКласс:{x  %-13s%s| %sЛовк:{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. единиц:{x  %-6d%s |\n\r"
"     | %sНатура:{x %-11s  %s| %sСлож:{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. время:{x   %-3d %s   |\n\r"
"     | %sЭтос :{x  %-12s %s| %sОбая:{x %2d{c({x%2d{c){x {C%2d{x %s| %s%s :{x   %3d      %s|"),

            CLR_FRAME, CLR_BAR, CLR_FRAME,

            CLR_CAPT,
            ch->getRealLevel( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR), pch->getMaxStat(STAT_STR),
            CLR_BAR,
            CLR_CAPT,
            (ch->getReligion() == god_none ? l(ch, "не определена") : ch->getReligion( )->getNameFor( ch ).ruscase( '1' ).c_str( )),
            CLR_FRAME,

            CLR_CAPT,
            pch->getRace( )->getPC( )->getScoreNameFor( ch, ch ).c_str( ), 
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT), pch->getMaxStat(STAT_INT),
            CLR_BAR, 
            CLR_CAPT,
            pch->practice.getValue( ),
            CLR_FRAME,

            CLR_CAPT,
            ch->getSex( ) == 0 ? l(ch, "потерян") : sex_table.message( ch->getSex( ), '1', Player::displayLang(ch) ).c_str( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS), pch->getMaxStat(STAT_WIS),
            CLR_BAR,
            CLR_CAPT,
            pch->train.getValue( ),
            CLR_FRAME,

            CLR_CAPT,
            profName.c_str( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX), pch->getMaxStat(STAT_DEX),
            CLR_BAR,
            CLR_CAPT,
            pch->getQuestPoints(),
            CLR_FRAME,

            CLR_CAPT,
            align_name_short(ch, Grammar::MultiGender::FEMININE),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON), pch->getMaxStat(STAT_CON),
            CLR_BAR,
            CLR_CAPT,
            qd ? qd->getTime( ) : 0,
            CLR_FRAME,

            CLR_CAPT,
            String::truncate(ethos, 12).c_str( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA), pch->getMaxStat(STAT_CHA),
            CLR_BAR,
            CLR_CAPT,
            ch->getProfession( ) == prof_samurai 
                ?  l(ch, "Смертей  ") : l(ch, "Трусость ") ,
            ch->getProfession( ) == prof_samurai 
                ? pch->death.getValue( ) : ch->wimpy.getValue( ),
            CLR_FRAME);

        ch->pecho(
            fmt ( ch, _("     %s| %sДом  :{x  %-30.30s %s| {Y%-22s %s|"),
                CLR_FRAME,
                CLR_CAPT,
                room ? room->areaName(Player::displayLang(ch)).c_str()
                     : fmt(ch, _("Потерян")).c_str(),
                CLR_BAR,
                l(ch, msgtable_lookup( msg_positions, ch->position )),
                CLR_FRAME,
                CLR_BAR, CLR_FRAME) );

    ch->pecho(
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|",
        CLR_BAR, CLR_FRAME);
            
    if (pch->guarding != 0) {
        ekle = 1;
        ch->pecho( 
_("     %s| {wТы охраняешь    :{Y %-10s                                    %s|"),
            CLR_FRAME,
            ch->seeName( pch->guarding, '4' ).c_str(),
            CLR_FRAME);
    }

    if (pch->guarded_by != 0) {
        ekle = 1;
        ch->pecho( 
_("     %s| {wТебя охраняет     :{Y %-10s                                  %s|"),
        CLR_FRAME,
        ch->seeName( pch->guarded_by, '1' ).c_str(),
        CLR_FRAME);
    }

    // Report only active desires in 'score'.
    for (int i = 0; i < desireManager->size( ); i++) {
        Desire *desire = desireManager->find(i);
        if (desire->isActive(ch->getPC())) {
            ostringstream buf;
        
            desire->report(ch->getPC(), buf);

            if (!buf.str( ).empty( )) {
                ekle = 1;
                ch->pecho( "     %s| {w%-64s%s|", 
                            CLR_FRAME,
                            buf.str( ).c_str( ),
                            CLR_FRAME );
            }
        }
    }

    if (ch->is_adrenalined()) {
        ekle = 1;
        ch->pecho( 
_("     %s| {yАдреналин кипит в твоих венах!                                  %s|"),
                 CLR_FRAME,
                 CLR_FRAME );
    }

    if (IS_GHOST(ch)) {
        ekle = 1;
        ch->pecho( 
_("     %1$s| {xТы призрак и обретёшь плоть через {Y%2$3d {xсекунд%2$Iу|ы|.                  %1$s|"),
                 CLR_FRAME,
                 pch->ghost_time*(PULSE_MOBILE/dreamland->getPulsePerSecond()),
                 CLR_FRAME );
    }

    if (ch->is_immortal()) {
        ekle = 1;
        ch->pecho( 
_("     %s| {wНевидимость: уровня %3d   "
         "Инкогнито: уровня %3d                 %s|"),
              CLR_FRAME,
              pch->invis_level.getValue( ),
              pch->incog_level.getValue( ),
              CLR_FRAME);
    }

    list<DLString> attrLines;
    if (ch->getPC()->getAttributes( ).handleEvent( ScoreArguments( ch->getPC(), attrLines ) )) {
        ekle = 1;
        for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
            ch->pecho("     %s| {w%-64s%s|", 
                        CLR_FRAME,
                        l->c_str(),
                        CLR_FRAME);
        }
    }

    if (ekle) {
        ch->pecho( 
"     %s|%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|",
                CLR_FRAME,
                CLR_BAR,
                CLR_FRAME);
    }


    ch->pecho( 
_("     %s| %sВещи          :{x     %3d/%-4d        %sЗащита от уколов:{x   %-5d   %s|\n\r"
"     | %sВес           :{x  %6d/%-8d    %sЗащита от ударов:{x   %-5d   %s|\n\r"
"     | %sЗолото        :{Y %-10d          %sЗащита от разрезов:{x %-5d   %s|\n\r"
"     | %sСеребро       :{W %-10d          %sЗащита от экзотики:{x %-5d   %s|\n\r"
"     | %sЕдиниц опыта  :{x %-6d              %sЗащита от заклинаний:{x %4d  %s|"),
        CLR_FRAME,
        CLR_CAPT,
        ch->carry_number, Char::canCarryNumber(ch),
        CLR_CAPT,
        GET_AC(ch,AC_PIERCE),
        CLR_FRAME,

        CLR_CAPT,
        Char::getCarryWeight(ch)/10, Char::canCarryWeight(ch)/10,
        CLR_CAPT,
        GET_AC(ch,AC_BASH),
        CLR_FRAME,

        CLR_CAPT,
        ch->gold.getValue( ),
        CLR_CAPT,
        GET_AC(ch,AC_SLASH),
        CLR_FRAME,

        CLR_CAPT,
        ch->silver.getValue( ),
        CLR_CAPT,
        GET_AC(ch,AC_EXOTIC),
        CLR_FRAME,

        CLR_CAPT,
        ch->exp.getValue( ),
        CLR_CAPT,
        ch->saving_throw.getValue( ),
        CLR_FRAME);

    ch->pecho( 
_("     %s| %sОпыта до уровня:{x %-6d                                         %s|\n\r"
"     |                                    %sЖизни:{x %5d / %5d         %s|"),
        CLR_FRAME,
        CLR_CAPT,
        pch->getExpToLevel( ),
        CLR_FRAME,

        CLR_CAPT,
        ch->hit.getValue( ), ch->max_hit.getValue( ),
        CLR_FRAME);

    ch->pecho( 
_("     %s| %sТочность      :{x   %-3d            %sЭнергии:{x %5d / %5d         %s|\n\r"
"     | %sУрон          :{x   %-3d           %sДвижения:{x %5d / %5d         %s|"),
        CLR_FRAME,
        CLR_CAPT,
        ch->hitroll.getValue( ),
        CLR_CAPT,
        ch->mana.getValue( ), ch->max_mana.getValue( ),
        CLR_FRAME,

        CLR_CAPT,
        ch->damroll.getValue( ),
        CLR_CAPT,
        ch->move.getValue( ), ch->max_move.getValue( ),
        CLR_FRAME);


    ch->pecho( 
"  %s/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r"
"  \\________________________________________________________________\\__/{x",
        CLR_FRAME);

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "noempty");
}


/*
 * 'oscore' -- the classic detailed score. Players get the framed ASCII layout;
 * NPCs get the linear prose. A single parameter (e.g. 'oscore saves') prints
 * just that field. The accessible 'score' (Fenia command/score) delegates its
 * own parameter form here, so 'score saves' keeps working.
 */
CMDRUNP( oscore )
{
    DLString arg = argument;
    if (!arg.empty( )) {
        do_score_args( ch, arg );
        return;
    }

    if (ch->is_npc( )) {
        score_prose( ch );
        return;
    }

    score_ascii( ch );
}

/*
 * 'score' proper is the accessible panel implemented in Fenia
 * (command/score/runFunc). Per WrappedCommand::entryPoint the runFunc override
 * replaces this native handler for PCs and NPCs alike, so it only fires as a
 * boot-safety fall-through if the Fenia command is unavailable (e.g. Fenia never
 * loaded): render the linear prose score, honouring a single-parameter query.
 */
CMDRUNP( score )
{
    DLString arg = argument;
    if (!arg.empty( )) {
        do_score_args( ch, arg );
        return;
    }

    score_prose( ch );
}


