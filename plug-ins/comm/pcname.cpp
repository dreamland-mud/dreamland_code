/* T8 name-form editor: let a player set their English and Ukrainian name forms
 * for viewers reading in those languages. The form for the language the player
 * registered under is their locked default and can't be changed here; the two
 * others fall back to auto-romanisation (#832) / the Russian form until set.
 */
#include <algorithm>
#include <vector>

#include "pcname.h"
#include "pcharacter.h"
#include "pcharactermemory.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "inflectedstring.h"
#include "grammar_entities_impl.h"
#include "admincommand.h"
#include "descriptor.h"
#include "wiznet.h"
#include "act.h"
#include "messengers.h"
#include "logstream.h"
#include "string_utils.h"
#include "commandtemplate.h"
#include "morphology.h"
#include "dl_ctype.h"
#include "merc.h"
#include "def.h"
#include <arg_utils.h>
#include "l10n.h"

using namespace std;

// A player can edit their name forms from this level on (Kit's call).
#define NAME_EDIT_LEVEL   10
// A proposed form may differ from the login by up to a third of its romanised
// length (at least one letter) and apply at once; anything further goes to the
// immortal queue, to block impersonation.
#define NAME_MODERATE_RATIO 3
// Longest stored form: a Flexer pad is the nominative plus six endings.
#define NAME_FORM_MAX 64

static const char *ATTR_NAMEFORM = "nameform";

// Levenshtein edit distance. Runtime strings are single-byte KOI8, so this is a
// per-character distance for names; case-insensitive.
static int name_distance( const DLString &a, const DLString &b )
{
    size_t n = a.size( ), m = b.size( );
    vector<int> prev( m + 1 ), cur( m + 1 );

    for (size_t j = 0; j <= m; j++)
        prev[j] = (int)j;

    for (size_t i = 1; i <= n; i++) {
        cur[0] = (int)i;
        for (size_t j = 1; j <= m; j++) {
            int cost = (dl_tolower( a[i-1] ) == dl_tolower( b[j-1] )) ? 0 : 1;
            cur[j] = min( min( prev[j] + 1, cur[j-1] + 1 ), prev[j-1] + cost );
        }
        swap( prev, cur );
    }
    return prev[m];
}

// Nominative of a form the player typed: either a plain word or a full Flexer
// pad ("Деменці|я|ї|ї|ю|єю|ї", or the "=a|b|c" explicit-case variant).
static DLString name_nominative( const DLString &form )
{
    if (form.find( '|' ) == DLString::npos && (form.empty( ) || form.at( 0 ) != '='))
        return form;
    return InflectedString( form ).decline( '1' );
}

// What the player's name currently reads as in `lang`, as a clean nominative:
// English romanises the login, the others show the Russian/login form.
static DLString name_reference( PCharacter *pch, lang_t lang )
{
    if (lang == LANG_EN)
        return String::translitToLatin( pch->getName( ) );

    DLString rus = pch->getRussianName( ).decline( '1' );
    return rus.empty( ) ? pch->getName( ) : rus;
}

// Is the nominative close enough to the player's own name to apply without an
// immortal? Compared both as typed and romanised, so a Cyrillic spelling of a
// Latin login (Деменція for Dementia) is measured letter for letter.
static bool name_close_enough( PCharacter *pch, const DLString &nom, const DLString &ref )
{
    DLString latinNom   = String::translitToLatin( nom );
    DLString latinLogin = String::translitToLatin( pch->getName( ) );

    int dist = name_distance( nom, ref );
    dist = min( dist, name_distance( latinNom, String::translitToLatin( ref ) ) );
    dist = min( dist, name_distance( latinNom, latinLogin ) );

    int allowed = max( 1, (int)latinLogin.size( ) / NAME_MODERATE_RATIO );
    return dist <= allowed;
}

// Letters, hyphen and apostrophe, plus pad syntax ('|', leading '=') for the
// declined Cyrillic forms. Keeps colour codes and markup out of a name every
// other player will see.
static bool name_charset_ok( const DLString &form, lang_t lang )
{
    bool pad = (lang != LANG_EN);

    for (DLString::size_type i = 0; i < form.size( ); i++) {
        char c = form.at( i );
        if (pad && ((c == '=' && i == 0) || c == '|'))
            continue;
        if (!dl_isalpha( c ) && c != '-' && c != '\'')
            return false;
    }
    return true;
}

// The form as it will be stored: English as typed, a typed pad as typed, a
// Cyrillic nominative declined through the morphology sidecar.
static DLString name_stored_form( PCharacter *pch, lang_t lang, const DLString &form )
{
    if (lang == LANG_EN || form.find( '|' ) != DLString::npos || form.at( 0 ) == '=')
        return form;

    DLString gender = "-";
    if (pch->getSex( ) == SEX_MALE)   gender = "masc";
    if (pch->getSex( ) == SEX_FEMALE) gender = "femn";

    if (lang == LANG_RU)
        return Morphology::declineRu( form, "NOUN", gender );
    return Morphology::declineUa( form, "NOUN", gender );
}

// Write one language slot, online or offline. Online characters live in the
// player list as PCharacter, offline ones as PCharacterMemory.
static bool name_apply( PCMemoryInterface *pci, lang_t lang, const DLString &form )
{
    PCharacter *pc = pci->getPlayer( );
    if (pc) {
        if (lang == LANG_EN)      pc->setEnglishName( form );
        else if (lang == LANG_RU) pc->setRussianName( form );
        else                      pc->setUkrainianName( form );
        return true;
    }

    PCharacterMemory *pcm = dynamic_cast<PCharacterMemory *>( pci );
    if (!pcm)
        return false;

    if (lang == LANG_EN)      pcm->setEnglishName( form );
    else if (lang == LANG_RU) pcm->setRussianName( form );
    else                      pcm->setUkrainianName( form );
    return true;
}

// File the request (replacing any earlier one) and tell the gods.
static void name_queue( PCharacter *pch, lang_t lang, const DLString &stored, const DLString &nom )
{
    XMLAttributeNameForm::Pointer attr =
        pch->getAttributes( ).getAttr<XMLAttributeNameForm>( ATTR_NAMEFORM );

    attr->lang.setValue( lang2attr( lang ) );
    attr->form.setValue( stored );
    attr->date.setTime( Date::getCurrentTime( ) );
    attr->responsible.setValue( "" );
    attr->reason.setValue( "" );
    attr->accepted.setValue( false );
    pch->save( );

    wiznet( WIZ_CONFIRM, 0, 0,
            "%^C1 просит форму имени %s: %s ({y{hcnameapprove list{x).",
            (Character *)pch, lang2attr( lang ).c_str( ), nom.c_str( ) );

    DLString discord;
    discord << "Name form request: " << pch->getName( ) << " asks for the "
            << lang2attr( lang ) << " form \"" << nom << "\" (" << stored << "). "
            << "Approve in game: nameapprove " << pch->getName( )
            << ", or: nameapprove " << pch->getName( ) << " deny <reason>";
    send_discord_wiznet( discord );

    LogStream::sendNotice( ) << "nameform: " << pch->getName( ) << " requests "
                             << lang2attr( lang ) << " form " << stored << endl;
}

static void name_show( PCharacter *pch )
{
    lang_t base = pch->getBaseLang( );
    ostringstream buf;

    buf << "Твое имя (" << pch->getName( ) << ") показывается на разных языках так:" << endl;
    buf << "  " << (base == LANG_EN ? "* " : "  ") << "English:    "
        << pch->getNameP( '1', LANG_EN ) << endl;
    buf << "  " << (base == LANG_RU ? "* " : "  ") << "Русский:    "
        << pch->getNameP( '1', LANG_RU ) << endl;
    buf << "  " << (base == LANG_UA ? "* " : "  ") << "Українська: "
        << pch->getNameP( '1', LANG_UA ) << endl;
    buf << "(* -- язык регистрации, его форму менять нельзя.)" << endl;

    pch->send_to( buf );
}

CMDRUNP( pcname )
{
    PCharacter *pch = ch->getPC( );
    if (!pch)
        return;

    DLString args = argument;
    DLString sub  = args.getOneArgument( );

    if (sub.empty( ) || arg_is_show( sub )) {
        name_show( pch );
        return;
    }

    if (pch->getLevel( ) < NAME_EDIT_LEVEL) {
        pch->pecho( _("Задавать имена по языкам можно только с %d уровня."), NAME_EDIT_LEVEL );
        return;
    }

    // Which language slot is being set.
    lang_t lang;
    if (arg_oneof( sub, "en", "англ", "английский" ))
        lang = LANG_EN;
    else if (arg_oneof( sub, "ua", "укр", "украинский", "українська" ))
        lang = LANG_UA;
    else if (arg_oneof( sub, "ru", "рус", "русский" ))
        lang = LANG_RU;
    else {
        pch->pecho( _("Используй: имя <en|ru|ua> <имя>. Без аргументов -- показать формы.") );
        return;
    }

    lang_t base = pch->getBaseLang( );
    if (lang == base) {
        pch->pecho( _("Это язык, под которым ты зарегистрирован -- эту форму менять нельзя.") );
        return;
    }

    DLString form = args;
    form.stripWhiteSpace( );
    if (form.empty( )) {
        pch->pecho( _("Укажи, как записать имя: имя <en|ua> <имя>.") );
        return;
    }

    // Charset: an English form must be Latin, a Ukrainian one Cyrillic.
    bool cyr = String::hasCyrillic( form );
    if (lang == LANG_EN && cyr) {
        pch->pecho( _("Английская форма имени должна быть латиницей.") );
        return;
    }
    if (lang != LANG_EN && !cyr) {
        pch->pecho( _("Русская и украинская формы имени должны быть кириллицей.") );
        return;
    }
    if (!name_charset_ok( form, lang )) {
        pch->pecho( _("В имени могут быть только буквы, дефис и апостроф.") );
        return;
    }

    DLString nom = name_nominative( form );
    if (nom.empty( ) || nom.size( ) > 24 || form.size( ) > NAME_FORM_MAX) {
        pch->pecho( _("Слишком длинное имя.") );
        return;
    }

    // Uniqueness: must not be another player's login name.
    PCMemoryInterface *other = PCharacterManager::find( nom );
    if (other != 0 && !String::equalLess( other->getName( ), pch->getName( ) )) {
        pch->pecho( _("Это имя уже занято другим игроком.") );
        return;
    }

    DLString stored = name_stored_form( pch, lang, form );

    // Moderation: a form far from the player's own name waits for an immortal
    // (blocks impersonation); small fixes apply immediately.
    DLString reference = name_reference( pch, lang );
    if (!name_close_enough( pch, nom, reference )) {
        name_queue( pch, lang, stored, nom );
        pch->pecho( _("Форма %1$s заметно отличается от твоего имени (%2$s), поэтому ее должен "
                      "одобрить бессмертный. Заявка отправлена богам, ответ придет сюда. "
                      "Новая заявка заменит прежнюю."),
                    nom.c_str( ), reference.c_str( ) );
        return;
    }

    name_apply( pch, lang, stored );

    // A fix that went through on its own supersedes a request still waiting.
    XMLAttributeNameForm::Pointer pending =
        pch->getAttributes( ).findAttr<XMLAttributeNameForm>( ATTR_NAMEFORM );
    if (pending && pending->isPending( ))
        pch->getAttributes( ).eraseAttribute( ATTR_NAMEFORM );

    pch->pecho( _("Готово. Теперь на этом языке тебя видят как: %s"),
                pch->getNameP( '1', lang ).c_str( ) );
}

/*-----------------------------------------------------------------------------
 * 'nameapprove' -- the immortal side of the queue
 *---------------------------------------------------------------------------*/
static void nameform_list( Character *ch )
{
    const PCharacterMemoryList &players = PCharacterManager::getPCM( );
    ostringstream buf;
    int count = 0;

    for (auto &p: players) {
        XMLAttributeNameForm::Pointer attr =
            p.second->getAttributes( ).findAttr<XMLAttributeNameForm>( ATTR_NAMEFORM );
        if (!attr || !attr->isPending( ))
            continue;

        buf << fmt( 0, "%-15s %-3s %-15s %s  %s\r\n",
                    p.second->getName( ).c_str( ),
                    attr->lang.getValue( ).c_str( ),
                    name_nominative( attr->form.getValue( ) ).c_str( ),
                    attr->date.getTimeAsString( "%H:%M %b %d" ).c_str( ),
                    attr->form.getValue( ).c_str( ) );
        count++;
    }

    if (count == 0) {
        ch->pecho( "No name-form requests are waiting." );
        return;
    }

    ch->pecho( "{W%-15s %-3s %-15s %-12s  %s{x", "Name", "Lng", "Form", "Date", "Full form" );
    ch->send_to( buf );
    ch->pecho( "Approve: nameapprove <player>. Deny: nameapprove <player> deny [reason]." );
}

// Record the verdict and tell the player now, or on their next login.
static void nameform_resolve( Character *ch, PCMemoryInterface *pci,
                              XMLAttributeNameForm::Pointer attr,
                              bool accepted, const DLString &reason )
{
    attr->responsible.setValue( ch->getNameC( ) );
    attr->accepted.setValue( accepted );
    attr->reason.setValue( reason );

    PCharacter *victim = pci->getPlayer( );
    if (victim)
        attr->deliver( victim );

    PCharacterManager::saveMemory( pci );
}

CMDADM( nameapprove )
{
    DLString args = constArguments;
    args.stripWhiteSpace( );
    DLString name = args.getOneArgument( );

    if (name.empty( ) || arg_is_list( name )) {
        nameform_list( ch );
        return;
    }

    PCMemoryInterface *pci = PCharacterManager::find( name );
    if (!pci) {
        ch->pecho( "Player not found. Misspelled name?" );
        return;
    }

    XMLAttributeNameForm::Pointer attr =
        pci->getAttributes( ).findAttr<XMLAttributeNameForm>( ATTR_NAMEFORM );
    if (!attr || !attr->isPending( )) {
        ch->pecho( "No pending name-form request from %s.", pci->getName( ).c_str( ) );
        return;
    }

    DLString sub = args.getOneArgument( );
    DLString nom = name_nominative( attr->form.getValue( ) );

    if (arg_oneof( sub, "deny", "reject", "отказать" )) {
        args.stripWhiteSpace( );
        nameform_resolve( ch, pci, attr, false, args );
        ch->pecho( "Denied %s form \"%s\" for %s.",
                   attr->lang.getValue( ).c_str( ), nom.c_str( ), pci->getName( ).c_str( ) );
        wiznet( WIZ_CONFIRM, 0, 0, "%^C1 отклоняет форму имени %s для %s.",
                ch, nom.c_str( ), pci->getName( ).c_str( ) );
        return;
    }

    if (!sub.empty( ) && sub != attr->lang.getValue( )) {
        ch->pecho( "That request is for the %s form, not %s.",
                   attr->lang.getValue( ).c_str( ), sub.c_str( ) );
        return;
    }

    if (!name_apply( pci, attr2lang( attr->lang.getValue( ) ), attr->form.getValue( ) )) {
        ch->pecho( "Could not set the name form for %s.", pci->getName( ).c_str( ) );
        return;
    }

    nameform_resolve( ch, pci, attr, true, "" );
    ch->pecho( "Approved %s form \"%s\" for %s.",
               attr->lang.getValue( ).c_str( ), nom.c_str( ), pci->getName( ).c_str( ) );
    wiznet( WIZ_CONFIRM, 0, 0, "%^C1 одобряет форму имени %s для %s.",
            ch, nom.c_str( ), pci->getName( ).c_str( ) );
}

/*-----------------------------------------------------------------------------
 * XMLAttributeNameForm, NameFormLoginListener
 *---------------------------------------------------------------------------*/
void XMLAttributeNameForm::deliver( PCharacter *pch )
{
    DLString nom = name_nominative( form.getValue( ) );

    if (accepted.getValue( ))
        pch->pecho( _("{WБоги одобрили новую форму твоего имени: %1$s.{x"), nom.c_str( ) );
    else if (reason.getValue( ).empty( ))
        pch->pecho( _("{RБоги отклонили форму имени %1$s.{x"), nom.c_str( ) );
    else
        pch->pecho( _("{RБоги отклонили форму имени %1$s: %2$s{x"),
                    nom.c_str( ), reason.getValue( ).c_str( ) );

    pch->getAttributes( ).eraseAttribute( ATTR_NAMEFORM );
}

void NameFormLoginListener::run( int oldState, int newState, Descriptor *d )
{
    if (newState != CON_PLAYING || !d || !d->character)
        return;

    PCharacter *pch = d->character->getPC( );
    if (!pch)
        return;

    if (pch->is_immortal( )) {
        int waiting = 0;
        for (auto &p: PCharacterManager::getPCM( )) {
            XMLAttributeNameForm::Pointer a =
                p.second->getAttributes( ).findAttr<XMLAttributeNameForm>( ATTR_NAMEFORM );
            if (a && a->isPending( ))
                waiting++;
        }
        if (waiting > 0)
            pch->pecho( "Name-form requests waiting: %d ({y{hcnameapprove list{x).", waiting );
    }

    XMLAttributeNameForm::Pointer attr =
        pch->getAttributes( ).findAttr<XMLAttributeNameForm>( ATTR_NAMEFORM );
    if (attr && !attr->isPending( )) {
        attr->deliver( pch );
        pch->save( );
    }
}
