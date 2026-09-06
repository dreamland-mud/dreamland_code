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
 *
 * Listing is always sorted by item type then name A-Z (vault_browse_sorted), so
 * the row numbers a viewer sees match what 'vault get <n>' withdraws. Row
 * numbers and the per-type counts are clickable via {hc (they degrade to plain
 * text on a telnet/screen-reader client, so nothing is lost for blind players).
 */
#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <string.h>

#include "commandtemplate.h"
#include "character.h"
#include "pcharacter.h"
#include "core/object.h"
#include "inflectedstring.h"
#include "dl_strings.h"

#include "save_bank.h"
#include "loadsave.h"
#include "arg_utils.h"
#include "wearloc_utils.h"
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

static DLString vault_upper( const DLString &s )
{
    DLString r = s;
    for ( size_t i = 0; i < r.size( ); i++ )
        r[i] = dl_toupper( r[i] );
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

// The command stem a clickable link should send: "vault" for self, or
// "vault *<Owner>" for an immortal viewing someone else's cell, so a clicked
// row/type link stays on the same target.
static DLString vault_cmd_prefix( const DLString &ownerLabel )
{
    if ( ownerLabel.empty( ) )
        return DLString( "vault" );
    return DLString( "vault *" ) + ownerLabel;
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

/* Resolved NOMINATIVE display name for an entry in the viewer's language. The
 * stored short_descr is a raw Flexer pad ("кузнечн|ый|ого|... моло|т|та|..."),
 * so both display and keyword-matching must decline it to case 1 -- printing the
 * pad verbatim leaks the pipes, and a substring match against the pad never
 * finds the nominative word ("молот" is not a substring of "моло|т|та"). Uses
 * the prototype's gender, same as Object::updateCachedNoun. Empty pad -> "". */
static DLString vault_entry_name( const BankEntry &be, OBJ_INDEX_DATA *proto, lang_t lang )
{
    DLString pad = vault_entry_shortdescr( be, proto ).getForLang( lang );
    if ( pad.empty( ) )
        return DLString( "" );

    Grammar::MultiGender g = ( proto != 0 ) ? proto->gram_gender : Grammar::MultiGender( );
    InflectedString noun( pad, g );
    return noun.decline( '1' );          // '1' = nominative, the codebase idiom
}

/* Case-insensitive substring match of kwLower against an entry's name across ALL
 * declined case forms, so 'vault find молот' finds "кузнечный молот" AND a
 * declined query ('find молота') still finds it. russian_case_all_forms expands
 * the pad to every case ("молот молота молоту ...") and colour-strips; a
 * pipe-less name (EN, or a ShortDesc override) expands to itself, so a partial
 * ('find nis' -> "nishtyak") still matches. Display stays nominative via
 * vault_entry_name; only matching widens to all forms. */
static bool vault_entry_matches( const BankEntry &be, OBJ_INDEX_DATA *proto, lang_t lang, const DLString &kwLower )
{
    DLString pad = vault_entry_shortdescr( be, proto ).getForLang( lang );
    if ( pad.empty( ) )
        return false;
    DLString forms = russian_case_all_forms( pad ).toLower( );
    return forms.find( kwLower ) != DLString::npos;
}

/*-------------------------------------------------------------------------
 * sorting: type then name A-Z, so the numbering a viewer sees is stable across
 * list / get / find / filter (all go through vault_browse_sorted). The type key
 * is the language-independent canonical item_table name (grouping is the same
 * for everyone); the name key is colour-stripped + lowered so A-Z isn't thrown
 * off by a leading colour code. id breaks ties for a deterministic order.
 *------------------------------------------------------------------------*/
static void vault_sort_entries( std::vector<BankEntry> &entries, lang_t lang )
{
    std::stable_sort( entries.begin( ), entries.end( ),
        [lang]( const BankEntry &a, const BankEntry &b ) -> bool {
            OBJ_INDEX_DATA *pa = get_obj_index( a.vnum );
            OBJ_INDEX_DATA *pb = get_obj_index( b.vnum );

            DLString ta = item_table.name( vault_entry_type( a, pa ) );
            DLString tb = item_table.name( vault_entry_type( b, pb ) );
            if ( ta != tb )
                return ta < tb;

            DLString na = vault_entry_name( a, pa, lang ).colourStrip( ).toLower( );
            DLString nb = vault_entry_name( b, pb, lang ).colourStrip( ).toLower( );
            if ( na != nb )
                return na < nb;

            return a.id < b.id;
        } );
}

static void vault_browse_sorted( const DLString &kind, const DLString &key,
                                 std::vector<BankEntry> &entries, lang_t lang )
{
    bank_browse( kind, key, entries );
    vault_sort_entries( entries, lang );
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
// One row. The [nn] index is a clickable {hc that sends '<prefix> get <n>'; the
// label keeps the plain "[ n]" so telnet/screen-reader clients still read it.
static void vault_show_entry( Character *ch, int num, const BankEntry &be, lang_t lang,
                              const DLString &cmdPrefix )
{
    OBJ_INDEX_DATA *proto = get_obj_index( be.vnum );

    DLString name = vault_entry_name( be, proto, lang );
    if ( name.empty( ) )
        name = lmsg( lang, "(unknown item)", "(неизвестный предмет)", "(невідомий предмет)" );

    int itype = vault_entry_type( be, proto );
    int lvl   = vault_entry_level( be, proto );
    DLString typeName = item_table.name( itype );

    if ( be.contents > 0 )
        ch->pecho( lmsg( lang,
            "{hc'%s get %d'[%2d]{x %s {D(%s, %d items, lvl %d){x",
            "{hc'%s get %d'[%2d]{x %s {D(%s, предметов: %d, ур. %d){x",
            "{hc'%s get %d'[%2d]{x %s {D(%s, предметів: %d, рів. %d){x" ),
            cmdPrefix.c_str( ), num, num, name.c_str( ), typeName.c_str( ), be.contents, lvl );
    else
        ch->pecho( lmsg( lang,
            "{hc'%s get %d'[%2d]{x %s {D(%s, lvl %d){x",
            "{hc'%s get %d'[%2d]{x %s {D(%s, ур. %d){x",
            "{hc'%s get %d'[%2d]{x %s {D(%s, рів. %d){x" ),
            cmdPrefix.c_str( ), num, num, name.c_str( ), typeName.c_str( ), lvl );
}

// The per-type overview line: only the types actually present, each a clickable
// {hc that sends '<prefix> filter <type>'. Built in the sorted (grouped) order.
// Returns the composed, tag-carrying string; the caller pecho's it.
static DLString vault_type_summary_line( const std::vector<BankEntry> &entries, lang_t lang,
                                         const DLString &cmdPrefix )
{
    std::vector< std::pair<DLString,int> > tally;   // (canonical type name, count), first-seen order
    for ( size_t i = 0; i < entries.size( ); i++ ) {
        OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
        DLString t = item_table.name( vault_entry_type( entries[i], proto ) );
        bool found = false;
        for ( size_t k = 0; k < tally.size( ); k++ )
            if ( tally[k].first == t ) { tally[k].second++; found = true; break; }
        if ( !found )
            tally.push_back( std::pair<DLString,int>( t, 1 ) );
    }

    std::ostringstream buf;
    for ( size_t k = 0; k < tally.size( ); k++ ) {
        if ( k > 0 )
            buf << "  ";
        buf << "{hc'" << cmdPrefix << " filter " << tally[k].first << "'"
            << vault_upper( tally[k].first ) << " (" << tally[k].second << "){x";
    }
    return DLString( buf.str( ) );
}

// Full listing. Over 50 entries collapses to the per-type overview unless
// forceFull (explicit 'vault list' / 'vault all').
static void vault_list( Character *ch, const std::vector<BankEntry> &entries,
                        lang_t lang, const DLString &ownerLabel, const DLString &cmdPrefix,
                        bool forceFull )
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

    // Count header -- amount-aware noun via %I (1 предмет / 3 предмета / 17 предметов).
    if ( ownerLabel.empty( ) )
        ch->pecho( lmsg( lang,
            "Your vault holds %d %Iitem|items|items:",
            "В твоем хранилище хранится %d %Iпредмет|предмета|предметов:",
            "У твоєму сховищі зберігається %d %Iпредмет|предмети|предметів:" ),
            (int)entries.size( ), (int)entries.size( ) );
    else
        ch->pecho( lmsg( lang,
            "%s's vault holds %d %Iitem|items|items:",
            "В хранилище %s хранится %d %Iпредмет|предмета|предметов:",
            "У сховищі %s зберігається %d %Iпредмет|предмети|предметів:" ),
            ownerLabel.c_str( ), (int)entries.size( ), (int)entries.size( ) );

    if ( entries.size( ) > 50 && !forceFull ) {
        ch->pecho( lmsg( lang,
            "Too many to list in full -- pick a type, or narrow with {y'vault find <word>'{x:",
            "Слишком много, чтобы показать все -- выбери тип или сузь через {y'vault find <слово>'{x:",
            "Забагато, щоб показати все -- обери тип або звузь через {y'vault find <слово>'{x:" ) );
        ch->pecho( "%s", vault_type_summary_line( entries, lang, cmdPrefix ).c_str( ) );
        ch->pecho( lmsg( lang,
            "({y'%s list'{x shows every entry.)",
            "({y'%s list'{x покажет все записи.)",
            "({y'%s list'{x покаже всі записи.)" ), cmdPrefix.c_str( ) );
        return;
    }

    for ( size_t i = 0; i < entries.size( ); i++ )
        vault_show_entry( ch, (int)i + 1, entries[i], lang, cmdPrefix );

    ch->pecho( lmsg( lang,
        "Use {y'vault get <number|name>'{x to take one out, {y'vault find <word>'{x to search. By type:",
        "Команда {y'vault get <номер|название>'{x достанет предмет, {y'vault find <слово>'{x -- поищет. По типу:",
        "Команда {y'vault get <номер|назва>'{x дістане предмет, {y'vault find <слово>'{x -- пошукає. За типом:" ) );
    ch->pecho( "%s", vault_type_summary_line( entries, lang, cmdPrefix ).c_str( ) );
}

/*-------------------------------------------------------------------------
 * subcommand word sets
 *------------------------------------------------------------------------*/
static const char *WORDS_PUT[]    = { "put", "store", "deposit", "положить", "сложить", "депозит", "покласти", "класти", 0 };
static const char *WORDS_GET[]    = { "get", "take", "withdraw", "взять", "снять", "взяти", "зняти", "дістати", 0 };
static const char *WORDS_FIND[]   = { "find", "search", "найти", "искать", "знайти", "шукати", 0 };
static const char *WORDS_FILTER[] = { "filter", "type", "фильтр", "фільтр", "тип", 0 };
static const char *WORDS_LIST[]   = { "list", "all", "список", "все", "усе", 0 };

// Print the entries at the given ORIGINAL full-list indices (so the row numbers
// match what 'vault get <n>' expects), then the get hint. Used by find/filter.
static void vault_show_rows( Character *ch, const std::vector<BankEntry> &entries,
                             const std::vector<int> &hitIdx, lang_t lang, const DLString &cmdPrefix )
{
    for ( size_t k = 0; k < hitIdx.size( ); k++ )
        vault_show_entry( ch, hitIdx[k] + 1, entries[ hitIdx[k] ], lang, cmdPrefix );

    ch->pecho( lmsg( lang,
        "Take one out with {y'vault get <number>'{x.",
        "Достать: {y'vault get <номер>'{x.",
        "Дістати: {y'vault get <номер>'{x." ) );
}

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
        // Owner becomes a directory name: allow only [A-Za-z0-9] so '*../../x'
        // can't mkdir/write outside the bank tree. PC names are letters anyway.
        DLString ownerArg = peek.substr( 1 );
        bool okName = !ownerArg.empty( );
        for ( size_t i = 0; okName && i < ownerArg.size( ); i++ ) {
            char c = ownerArg[i];
            if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ) )
                okName = false;
        }
        if ( !okName ) {
            ch->pecho( lmsg( lang,
                "Usage: vault *<owner> [subcommand]  (letters/digits only)",
                "Использование: vault *<владелец> [подкоманда]  (только буквы/цифры)",
                "Використання: vault *<власник> [підкоманда]  (лише літери/цифри)" ) );
            return;
        }
        // Key is the pfile-canonical lowercase form (so delete/rename cleanup and
        // the owner's own vault land on the same cell); label stays capitalized.
        key = ownerArg.toLower( );
        ownerLabel = vault_capitalize( ownerArg );
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
        // Lowercase to match the pfile-canonical key (delete/rename cleanup).
        key = pch->getName( ).toLower( );
    }

    DLString sub = vault_lower( peek );
    DLString cmdPrefix = vault_cmd_prefix( ownerLabel );

    /*---- vault put <item> / put all / put all.<kw> ------------------------*/
    if ( vault_word_in( sub, WORDS_PUT ) ) {
        DLString itemArg = args.getOneArgument( );
        if ( itemArg.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Store what?",
                "Убрать в хранилище что?",
                "Сховати що?" ) );
            return;
        }

        bool bulkAll = arg_is_all( itemArg );
        bool bulkDot = !bulkAll && arg_is_alldot( itemArg );

        if ( bulkAll || bulkDot ) {
            DLString kw;
            if ( bulkDot ) {
                size_t dot = itemArg.find( '.' );
                kw = ( dot != DLString::npos ) ? DLString( itemArg.substr( dot + 1 ) ) : DLString( "" );
            }

            int stored = 0, skipped = 0;
            Object *obj_next = 0;
            for ( Object *obj = ch->carrying; obj != 0; obj = obj_next ) {
                obj_next = obj->next_content;                 // capture: bank_deposit extracts obj
                if ( obj->wear_loc != wear_none )             // never bank worn gear
                    continue;
                if ( !ch->can_see( obj ) )
                    continue;
                if ( bulkDot && !obj_has_name( obj, kw, ch ) )
                    continue;

                if ( vault_policy_reason( obj ) != 0 ) { skipped++; continue; }
                if ( !bank_deposit( obj, kind, key ) )  { skipped++; continue; }
                stored++;
            }

            if ( stored > 0 )
                ch->getPC( )->save( );

            if ( stored == 0 && skipped == 0 ) {
                ch->pecho( lmsg( lang,
                    "You have nothing like that to store.",
                    "У тебя нет такого, чтобы убрать в хранилище.",
                    "У тебе немає такого, щоб сховати у сховище." ) );
                return;
            }

            if ( ownerLabel.empty( ) )
                ch->pecho( lmsg( lang,
                    "You store %d %Iitem|items|items in your vault.",
                    "Ты убираешь %d %Iпредмет|предмета|предметов в свое хранилище.",
                    "Ти ховаєш %d %Iпредмет|предмети|предметів до свого сховища." ), stored, stored );
            else
                ch->pecho( lmsg( lang,
                    "You store %d %Iitem|items|items in %s's vault.",
                    "Ты убираешь %d %Iпредмет|предмета|предметов в хранилище %s.",
                    "Ти ховаєш %d %Iпредмет|предмети|предметів до сховища %s." ), stored, stored, ownerLabel.c_str( ) );

            if ( skipped > 0 )
                ch->pecho( lmsg( lang,
                    "Skipped %d %Iitem|items|items (unique, timed or unstorable).",
                    "Пропущено %d %Iпредмет|предмета|предметов (уникальные, с таймером или несохраняемые).",
                    "Пропущено %d %Iпредмет|предмети|предметів (унікальні, з таймером або незберігані)." ), skipped, skipped );
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
                "Items with a running timer can't be stored.",
                "Предметы с активным таймером нельзя убрать в хранилище.",
                "Предмети з активним таймером ховати не можна." ) );
            return;
        }

        // Capture name BEFORE deposit: bank_deposit extracts obj (obj is gone
        // after a true return, and must not be dereferenced).
        DLString name = obj->getShortDescr( '1', lang );   // nominative, not the raw pad

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

        // Persist the inventory change now: the cell is on disk instantly, so
        // save ch too or a crash inside the ~60s autosave window would leave the
        // pfile still holding the item (dupe on withdraw). ch is who lost it,
        // including the immortal owner-override case.
        ch->getPC( )->save( );
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
        vault_browse_sorted( kind, key, entries, lang );

        // Keep ORIGINAL indices: 'vault get <n>' numbers the full list, so the
        // rows shown here must carry their full-list number, not a 1..N reindex.
        DLString kwLower = kw.toLower( );
        std::vector<int> hitIdx;
        for ( size_t i = 0; i < entries.size( ); i++ ) {
            OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
            if ( vault_entry_matches( entries[i], proto, lang, kwLower ) )
                hitIdx.push_back( (int)i );
        }

        if ( hitIdx.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Nothing in the vault matches '%s'.",
                "В хранилище нет ничего похожего на '%s'.",
                "У сховищі нема нічого схожого на '%s'." ), kw.c_str( ) );
            return;
        }

        ch->pecho( lmsg( lang,
            "Vault entries matching '%s':",
            "Записи хранилища по '%s':",
            "Записи сховища за '%s':" ), kw.c_str( ) );
        vault_show_rows( ch, entries, hitIdx, lang, cmdPrefix );
        return;
    }

    /*---- vault filter [<type>] --------------------------------------------*/
    if ( vault_word_in( sub, WORDS_FILTER ) ) {
        DLString typeArg = args.getOneArgument( );

        std::vector<BankEntry> entries;
        vault_browse_sorted( kind, key, entries, lang );

        // No type given -> show the per-type overview of what's actually there.
        if ( typeArg.empty( ) ) {
            if ( entries.empty( ) ) {
                vault_list( ch, entries, lang, ownerLabel, cmdPrefix, true );
                return;
            }
            ch->pecho( lmsg( lang,
                "Filter by which item type?",
                "Отфильтровать по какому типу?",
                "Відфільтрувати за яким типом?" ) );
            ch->pecho( "%s", vault_type_summary_line( entries, lang, cmdPrefix ).c_str( ) );
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

        std::vector<int> hitIdx;
        for ( size_t i = 0; i < entries.size( ); i++ ) {
            OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
            if ( vault_entry_type( entries[i], proto ) == ft )
                hitIdx.push_back( (int)i );
        }

        if ( hitIdx.empty( ) ) {
            ch->pecho( lmsg( lang,
                "No %s in the vault.",
                "В хранилище нет ни одного типа '%s'.",
                "У сховищі нема жодного типу '%s'." ), item_table.name( ft ).c_str( ) );
            return;
        }

        ch->pecho( lmsg( lang,
            "Vault entries of type %s:",
            "Записи хранилища типа %s:",
            "Записи сховища типу %s:" ), item_table.name( ft ).c_str( ) );
        vault_show_rows( ch, entries, hitIdx, lang, cmdPrefix );
        return;
    }

    /*---- vault list -------------------------------------------------------*/
    // Bare 'vault' collapses a >50 list to the per-type overview; an explicit
    // 'vault list' / 'vault all' forces the full dump.
    if ( sub.empty( ) || vault_word_in( sub, WORDS_LIST ) ) {
        std::vector<BankEntry> entries;
        vault_browse_sorted( kind, key, entries, lang );
        vault_list( ch, entries, lang, ownerLabel, cmdPrefix, !sub.empty( ) );
        return;
    }

    /*---- vault get <n|word> / get all / get all.<kw>  (and bare 'vault <n>') */
    // Anything not matched above is a withdrawal target: an explicit 'get', a
    // bare number, a bare keyword, or all / all.<kw>.
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
    vault_browse_sorted( kind, key, entries, lang );
    if ( entries.empty( ) ) {
        vault_list( ch, entries, lang, ownerLabel, cmdPrefix, true );   // prints the empty message
        return;
    }

    bool bulkAll = arg_is_all( target );
    bool bulkDot = !bulkAll && arg_is_alldot( target );

    if ( bulkAll || bulkDot ) {
        DLString kw;
        if ( bulkDot ) {
            size_t dot = target.find( '.' );
            kw = ( dot != DLString::npos ) ? DLString( target.substr( dot + 1 ) ).toLower( ) : DLString( "" );
        }

        // Collect target Ids up front -- Ids are stable across withdrawals, so a
        // withdrawal that unlinks a cell can't disturb the rest of the loop.
        std::vector<long long> ids;
        for ( size_t i = 0; i < entries.size( ); i++ ) {
            if ( bulkDot ) {
                OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
                if ( !vault_entry_matches( entries[i], proto, lang, kw ) )
                    continue;
            }
            ids.push_back( entries[i].id );
        }

        if ( ids.empty( ) ) {
            ch->pecho( lmsg( lang,
                "Nothing in the vault matches '%s'.",
                "В хранилище нет ничего похожего на '%s'.",
                "У сховищі нема нічого схожого на '%s'." ), target.c_str( ) );
            return;
        }

        int got = 0;
        for ( size_t k = 0; k < ids.size( ); k++ )
            if ( bank_withdraw_entry( ch, kind, key, ids[k] ) )
                got++;

        if ( got > 0 )
            ch->getPC( )->save( );

        ch->pecho( lmsg( lang,
            "You take %d %Iitem|items|items out of the vault.",
            "Ты достаешь %d %Iпредмет|предмета|предметов из хранилища.",
            "Ти дістаєш %d %Iпредмет|предмети|предметів зі сховища." ), got, got );
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
        DLString targetLower = target.toLower( );
        std::vector<int> matchIdx;
        for ( size_t i = 0; i < entries.size( ); i++ ) {
            OBJ_INDEX_DATA *proto = get_obj_index( entries[i].vnum );
            if ( vault_entry_matches( entries[i], proto, lang, targetLower ) )
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
                vault_show_entry( ch, matchIdx[k] + 1, entries[ matchIdx[k] ], lang, cmdPrefix );
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
            name = o->getShortDescr( '1', lang );   // nominative, not the raw pad
            break;
        }
    if ( name.empty( ) )
        name = lmsg( lang, "the item", "предмет", "предмет" );

    ch->pecho( lmsg( lang,
        "You take %s out of the vault.",
        "Ты достаешь %s из хранилища.",
        "Ти дістаєш %s зі сховища." ), name.c_str( ) );

    // Persist the pickup: the cell was unlinked on disk instantly, so save ch too
    // or a crash inside the ~60s autosave window would lose the item (gone from
    // both the vault and the pfile). ch is who received it.
    ch->getPC( )->save( );
}
