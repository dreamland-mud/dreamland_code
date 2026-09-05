/* 'vault' -- personal off-world object storage. The player-facing half of the
 * object bank: at any bank room the vault stores whole items (a bag with its
 * contents = one entry) out of object_list and materializes them back on
 * demand, so an offline player's hoard never rides the obj_update sweep.
 *
 * Logic + gate + policy live here in C++; the store primitives are in
 * loadsave/save_bank. Player text is trilingual via lmsg(). (A hot-reloadable
 * Fenia wording layer is a planned follow-up -- it needs the command to live in
 * a fenia-linked plugin, and comm is not one.)
 *
 * Immortal owner-override: 'vault *<owner> <sub...>' operates on another
 * character's cell (get/put still use the immortal's own inventory), so a god
 * can seed, inspect and drain a pilot player's vault without logging in as them.
 */
#include <vector>
#include <string.h>

#include "commandtemplate.h"
#include "character.h"
#include "pcharacter.h"
#include "core/object.h"

#include "save_bank.h"
#include "loadsave.h"
#include "lang.h"
#include "dl_ctype.h"
#include "behavior.h"
#include "room.h"

#include "merc.h"
#include "def.h"
#include "l10n.h"

/*-------------------------------------------------------------------------
 * small helpers
 *------------------------------------------------------------------------*/
static DLString vault_lower( const DLString &s )
{
    DLString r = s;
    for ( size_t i = 0; i < r.size( ); i++ )
        r[i] = dl_tolower( r[i] );
    return r;
}

static DLString vault_capitalize( const DLString &s )
{
    if ( s.empty( ) )
        return s;
    DLString r = s;
    r[0] = dl_toupper( r[0] );
    for ( size_t i = 1; i < r.size( ); i++ )
        r[i] = dl_tolower( r[i] );
    return r;
}

// Is 'tok' one of a small set of subcommand synonyms? (English + RU + UA.)
static bool vault_word_in( const DLString &tok, const char *const *set )
{
    for ( int i = 0; set[i] != 0; i++ )
        if ( tok == set[i] )
            return true;
    return false;
}

/* Effective prototype-or-override view of one stored entry, for display and
 * matching without materializing the object. */
static const XMLMultiString & vault_entry_shortdescr( const BankEntry &be, OBJ_INDEX_DATA *proto )
{
    if ( !be.shortDescr.emptyValues( ) )
        return be.shortDescr;
    static XMLMultiString empty;
    return proto != 0 ? proto->short_descr : empty;
}

static int vault_entry_type( const BankEntry &be, OBJ_INDEX_DATA *proto )
{
    if ( be.itemType != -1 )
        return be.itemType;
    return proto != 0 ? proto->item_type : 0;
}

static int vault_entry_level( const BankEntry &be, OBJ_INDEX_DATA *proto )
{
    if ( be.level != -1 )
        return be.level;
    return proto != 0 ? proto->level : 0;
}

/*-------------------------------------------------------------------------
 * v1 deposit policy: refuse the whole subtree if any node is limited or carries
 * a live timer. Banked objects don't tick, so a timer would freeze mid-count and
 * a limited item's count/limit accounting is the dupe risk -- both deferred
 * until limit-accounting is proven. (NOSAVEDROP is refused deeper, in
 * bank_deposit, because that one would be silently destroyed on serialize.)
 * Returns 0 ok, 1 limited, 2 timer.
 *------------------------------------------------------------------------*/
static int vault_policy_reason( Object *obj )
{
    if ( obj->pIndexData->limit != -1 )
        return 1;
    if ( obj->timer > 0 )
        return 2;

    for ( Object *c = obj->contains; c != 0; c = c->next_content ) {
        int r = vault_policy_reason( c );
        if ( r != 0 )
            return r;
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * listing
 *------------------------------------------------------------------------*/
static void vault_show_entry( Character *ch, int num, const BankEntry &be, lang_t lang )
{
    OBJ_INDEX_DATA *proto = get_obj_index( be.vnum );

    DLString name = vault_entry_shortdescr( be, proto ).getForLang( lang );
    if ( name.empty( ) )
        name = lmsg( lang, "(unknown item)", "(неизвестный предмет)", "(невідомий предмет)" );

    int itype = vault_entry_type( be, proto );
    int lvl   = vault_entry_level( be, proto );
    DLString typeName = item_table.name( itype );

    if ( be.contents > 0 )
        ch->pecho( lmsg( lang,
            "[%2d] %s {D(%s, %d items, lvl %d){x",
            "[%2d] %s {D(%s, предметов: %d, ур. %d){x",
            "[%2d] %s {D(%s, предметів: %d, рів. %d){x" ),
            num, name.c_str( ), typeName.c_str( ), be.contents, lvl );
    else
        ch->pecho( lmsg( lang,
            "[%2d] %s {D(%s, lvl %d){x",
            "[%2d] %s {D(%s, ур. %d){x",
            "[%2d] %s {D(%s, рів. %d){x" ),
            num, name.c_str( ), typeName.c_str( ), lvl );
}

// List entries, optionally only those whose display index is in 'showIdx' (used
// by find/filter). showIdx empty -> show all.
static void vault_list( Character *ch, const std::vector<BankEntry> &entries,
                        lang_t lang, const DLString &ownerLabel )
{
    if ( entries.empty( ) ) {
        if ( ownerLabel.empty( ) )
            ch->pecho( lmsg( lang,
                "Your vault is empty.",
                "Твое хранилище пусто.",
                "Твоє сховище порожнє." ) );
        else
            ch->pecho( lmsg( lang,
                "%s's vault is empty.",
                "Хранилище %s пусто.",
                "Сховище %s порожнє." ), ownerLabel.c_str( ) );
        return;
    }

    if ( ownerLabel.empty( ) )
        ch->pecho( lmsg( lang,
            "Your vault holds %d entries:",
            "В твоем хранилище хранится предметов: %d",
            "У твоєму сховищі зберігається предметів: %d" ),
            (int)entries.size( ) );
    else
        ch->pecho( lmsg( lang,
            "%s's vault holds %d entries:",
            "В хранилище %s хранится предметов: %d",
            "У сховищі %s зберігається предметів: %d" ),
            ownerLabel.c_str( ), (int)entries.size( ) );

    for ( size_t i = 0; i < entries.size( ); i++ )
        vault_show_entry( ch, (int)i + 1, entries[i], lang );

    ch->pecho( lmsg( lang,
        "Use {y'vault get <number|name>'{x to take one out, {y'vault find <word>'{x to search.",
        "Команда {y'vault get <номер|название>'{x достанет предмет, {y'vault find <слово>'{x -- поищет.",
        "Команда {y'vault get <номер|назва>'{x дістане предмет, {y'vault find <слово>'{x -- пошукає." ) );
}

/*-------------------------------------------------------------------------
 * subcommand word sets
 *------------------------------------------------------------------------*/
static const char *WORDS_PUT[]    = { "put", "store", "положить", "сложить", "покласти", "класти", 0 };
static const char *WORDS_GET[]    = { "get", "take", "взять", "взяти", "дістати", 0 };
static const char *WORDS_FIND[]   = { "find", "search", "найти", "искать", "знайти", "шукати", 0 };
static const char *WORDS_FILTER[] = { "filter", "type", "фильтр", "фільтр", "тип", 0 };
static const char *WORDS_LIST[]   = { "list", "all", "список", "все", "усе", 0 };

/*-------------------------------------------------------------------------
 * the command
 *------------------------------------------------------------------------*/
CMDRUN( vault )
{
    lang_t lang = viewerLang( ch );

    if ( ch->is_npc( ) ) {
        ch->pecho( lmsg( lang, "Not for mobs.", "Не для мобов.", "Не для мобів." ) );
        return;
    }

    // Gate: a bank room (same behavior the money bank uses). ATM-object parity
    // is a known follow-up.
    Behavior *bankBhv = behaviorManager->findExisting( "bank" );
    bool inBank = bankBhv != 0
        && ch->in_room != 0
        && ch->in_room->pIndexData->behaviors.isSet( bankBhv->getIndex( ) );

    if ( !inBank ) {
        ch->pecho( lmsg( lang,
            "You need to be at a bank to reach your vault.",
            "Чтобы добраться до хранилища, нужно быть в банке.",
            "Щоб дістатися до сховища, треба бути в банку." ) );
        return;
    }

    DLString args = constArguments;

    // Optional immortal owner-override: '*<owner>' targets another cell.
    DLString kind = "player";
    DLString key;
    DLString ownerLabel;      // non-empty only when overriding, for messages

    DLString peek = args.getOneArgument( );   // consumes the first token
    if ( !peek.empty( ) && peek.at( 0 ) == '*' ) {
        if ( !ch->is_immortal( ) ) {
            ch->pecho( lmsg( lang,
                "Only immortals can open someone else's vault.",
                "Только бессмертные могут открыть чужое хранилище.",
                "Лише безсмертні можуть відкрити чуже сховище." ) );
            return;
        }
        DLString owner = vault_capitalize( peek.substr( 1 ) );
        if ( owner.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Usage: vault *<owner> [subcommand]",
                "Использование: vault *<владелец> [подкоманда]",
                "Використання: vault *<власник> [підкоманда]" ) );
            return;
        }
        key = owner;
        ownerLabel = owner;
        // args now holds the rest; take the real subcommand token below.
        peek = args.getOneArgument( );
    }
    else {
        // Self: needs a PC to own the cell.
        PCharacter *pch = ch->getPC( );
        if ( pch == 0 ) {
            ch->pecho( lmsg( lang,
                "You have no vault.",
                "У тебя нет хранилища.",
                "У тебе немає сховища." ) );
            return;
        }
        key = pch->getName( );
    }

    DLString sub = vault_lower( peek );

    /*---- vault put <item> -------------------------------------------------*/
    if ( vault_word_in( sub, WORDS_PUT ) ) {
        DLString itemArg = args.getOneArgument( );
        if ( itemArg.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Store what?",
                "Убрать в хранилище что?",
                "Сховати що?" ) );
            return;
        }

        Object *obj = get_obj_carry( ch, itemArg );
        if ( obj == 0 ) {
            ch->pecho( lmsg( lang,
                "You aren't carrying '%s'.",
                "У тебя нет с собой '%s'.",
                "У тебе немає з собою '%s'." ), itemArg.c_str( ) );
            return;
        }

        int reason = vault_policy_reason( obj );
        if ( reason == 1 ) {
            ch->pecho( lmsg( lang,
                "Limited items can't be stored in a vault yet.",
                "Уникальные предметы пока нельзя убирать в хранилище.",
                "Унікальні предмети поки не можна ховати у сховище." ) );
            return;
        }
        if ( reason == 2 ) {
            ch->pecho( lmsg( lang,
                "That has a timer running -- let it expire before storing it.",
                "На нем идет таймер -- дождись, пока он истечет, прежде чем убирать.",
                "На ньому цокає таймер -- дочекайся, доки він мине, перш ніж ховати." ) );
            return;
        }

        // Capture name BEFORE deposit: bank_deposit extracts obj (obj is gone
        // after a true return, and must not be dereferenced).
        DLString name = obj->getShortDescr( lang );

        if ( !bank_deposit( obj, kind, key ) ) {
            ch->pecho( lmsg( lang,
                "That can't be stored in a vault.",
                "Это нельзя убрать в хранилище.",
                "Це не можна сховати у сховище." ) );
            return;
        }

        if ( ownerLabel.empty( ) )
            ch->pecho( lmsg( lang,
                "You store %s in your vault.",
                "Ты убираешь %s в свое хранилище.",
                "Ти ховаєш %s до свого сховища." ), name.c_str( ) );
        else
            ch->pecho( lmsg( lang,
                "You store %s in %s's vault.",
                "Ты убираешь %s в хранилище %s.",
                "Ти ховаєш %s до сховища %s." ), name.c_str( ), ownerLabel.c_str( ) );
        return;
    }

    /*---- vault find <word> ------------------------------------------------*/
    if ( vault_word_in( sub, WORDS_FIND ) ) {
        DLString kw = args.getOneArgument( );
        if ( kw.empty( ) ) {
            ch->pecho( lmsg( lang, "Find what?", "Найти что?", "Знайти що?" ) );
            return;
        }

        std::vector<BankEntry> entries;
        bank_browse( kind, key, entries );

        std::vector<BankEntry> hits;
        for ( size_t i = 0; i < entries.size( ); i++ ) {
            OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
            if ( vault_entry_shortdescr( entries[i], proto ).matchesSubstring( kw ) )
                hits.push_back( entries[i] );
        }

        if ( hits.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Nothing in the vault matches '%s'.",
                "В хранилище нет ничего похожего на '%s'.",
                "У сховищі нема нічого схожого на '%s'." ), kw.c_str( ) );
            return;
        }

        vault_list( ch, hits, lang, ownerLabel );
        return;
    }

    /*---- vault filter <type> ----------------------------------------------*/
    if ( vault_word_in( sub, WORDS_FILTER ) ) {
        DLString typeArg = args.getOneArgument( );
        if ( typeArg.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Filter by which item type? (weapon, armor, potion, ...)",
                "Отфильтровать по какому типу? (weapon, armor, potion, ...)",
                "Відфільтрувати за яким типом? (weapon, armor, potion, ...)" ) );
            return;
        }

        int ft = item_table.value( typeArg );
        if ( ft == NO_FLAG ) {
            ch->pecho( lmsg( lang,
                "Unknown item type '%s'.",
                "Неизвестный тип предмета '%s'.",
                "Невідомий тип предмета '%s'." ), typeArg.c_str( ) );
            return;
        }

        std::vector<BankEntry> entries;
        bank_browse( kind, key, entries );

        std::vector<BankEntry> hits;
        for ( size_t i = 0; i < entries.size( ); i++ ) {
            OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
            if ( vault_entry_type( entries[i], proto ) == ft )
                hits.push_back( entries[i] );
        }

        if ( hits.empty( ) ) {
            ch->pecho( lmsg( lang,
                "No %s in the vault.",
                "В хранилище нет ни одного типа '%s'.",
                "У сховищі нема жодного типу '%s'." ), item_table.name( ft ).c_str( ) );
            return;
        }

        vault_list( ch, hits, lang, ownerLabel );
        return;
    }

    /*---- vault list -------------------------------------------------------*/
    if ( sub.empty( ) || vault_word_in( sub, WORDS_LIST ) ) {
        std::vector<BankEntry> entries;
        bank_browse( kind, key, entries );
        vault_list( ch, entries, lang, ownerLabel );
        return;
    }

    /*---- vault get <n|word>  (and bare 'vault <n>') -----------------------*/
    // Anything not matched above is a withdrawal target: an explicit 'get', a
    // bare number, or a bare keyword.
    DLString target;
    if ( vault_word_in( sub, WORDS_GET ) )
        target = args.getOneArgument( );
    else
        target = peek;              // bare 'vault <n>' / 'vault <word>'

    if ( target.empty( ) ) {
        ch->pecho( lmsg( lang, "Take out what?", "Достать что?", "Дістати що?" ) );
        return;
    }

    std::vector<BankEntry> entries;
    bank_browse( kind, key, entries );
    if ( entries.empty( ) ) {
        vault_list( ch, entries, lang, ownerLabel );   // prints the empty message
        return;
    }

    long long targetId = 0;
    bool found = false;

    if ( target.isNumber( ) ) {
        int n = atoi( target.c_str( ) );
        if ( n >= 1 && n <= (int)entries.size( ) ) {
            targetId = entries[n - 1].id;
            found = true;
        }
        else {
            ch->pecho( lmsg( lang,
                "No entry number %d in the vault.",
                "В хранилище нет записи под номером %d.",
                "У сховищі нема запису під номером %d." ), n );
            return;
        }
    }
    else {
        // keyword: collect matches by display index
        std::vector<int> matchIdx;
        for ( size_t i = 0; i < entries.size( ); i++ ) {
            OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
            if ( vault_entry_shortdescr( entries[i], proto ).matchesSubstring( target ) )
                matchIdx.push_back( (int)i );
        }

        if ( matchIdx.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Nothing in the vault matches '%s'.",
                "В хранилище нет ничего похожего на '%s'.",
                "У сховищі нема нічого схожого на '%s'." ), target.c_str( ) );
            return;
        }
        if ( matchIdx.size( ) > 1 ) {
            ch->pecho( lmsg( lang,
                "Several entries match '%s' -- pick a number:",
                "Несколько записей похожи на '%s' -- выбери номер:",
                "Декілька записів схожі на '%s' -- обери номер:" ), target.c_str( ) );
            for ( size_t k = 0; k < matchIdx.size( ); k++ )
                vault_show_entry( ch, matchIdx[k] + 1, entries[ matchIdx[k] ], lang );
            return;
        }

        targetId = entries[ matchIdx[0] ].id;
        found = true;
    }

    if ( !found )
        return;

    if ( !bank_withdraw_entry( ch, kind, key, targetId ) ) {
        ch->pecho( lmsg( lang,
            "Couldn't retrieve that entry -- it may be corrupt (kept for a fix).",
            "Не удалось достать эту запись -- возможно, она повреждена (сохранена для починки).",
            "Не вдалося дістати цей запис -- можливо, він пошкоджений (збережено для полагодження)." ) );
        return;
    }

    // Locate the just-materialized object by Id for a correct name.
    DLString name;
    for ( Object *o = ch->carrying; o != 0; o = o->next_content )
        if ( o->getID( ) == targetId ) {
            name = o->getShortDescr( lang );
            break;
        }
    if ( name.empty( ) )
        name = lmsg( lang, "the item", "предмет", "предмет" );

    ch->pecho( lmsg( lang,
        "You take %s out of the vault.",
        "Ты достаешь %s из хранилища.",
        "Ти дістаєш %s зі сховища." ), name.c_str( ) );
}
