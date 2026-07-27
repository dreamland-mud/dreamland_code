/* T8 name-form editor: let a player set their English and Ukrainian name forms
 * for viewers reading in those languages. The form for the language the player
 * registered under is their locked default and can't be changed here; the two
 * others fall back to auto-romanisation (#832) / the Russian form until set.
 */
#include <algorithm>
#include <vector>

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
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
// A proposed form this far (or more) from the auto-transliteration of the login
// is held for immortal moderation, to block impersonation.
#define NAME_MODERATE_DIFF 3

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

// The language whose form is the player's locked default -- the one they
// registered under. Not persisted yet (TODO: capture the nanny's first-question
// language into a stored baseLang), so infer from the login's script: a Cyrillic
// login is an RU/UA registration (lock RU), a Latin one an EN registration.
static lang_t name_base_lang( PCharacter *pch )
{
    return String::hasCyrillic( pch->getName( ) ) ? LANG_RU : LANG_EN;
}

// The default form a viewer of `lang` sees when the player has set nothing:
// English romanises the login, the others show the Russian/login form.
static DLString name_default_form( PCharacter *pch, lang_t lang )
{
    if (lang == LANG_EN)
        return String::translitToLatin( pch->getName( ) );

    DLString rus = pch->getRussianName( ).getFullForm( );
    return rus.empty( ) ? pch->getName( ) : rus;
}

static void name_show( PCharacter *pch )
{
    lang_t base = name_base_lang( pch );
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

    lang_t base = name_base_lang( pch );
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
    if (form.colourStrip( ).size( ) > 24) {
        pch->pecho( _("Слишком длинное имя.") );
        return;
    }

    // Uniqueness: must not be another player's login name.
    PCMemoryInterface *other = PCharacterManager::find( form );
    if (other != 0 && !String::equalLess( other->getName( ), pch->getName( ) )) {
        pch->pecho( _("Это имя уже занято другим игроком.") );
        return;
    }

    // Moderation: a form far from the auto-transliteration of the login is held
    // for an immortal (blocks impersonation); small fixes apply immediately.
    // TODO: a real pending-approval queue + immortal approve/reject command;
    // for now a divergent form is rejected with a note to ask an immortal.
    DLString reference = name_default_form( pch, lang );
    if (name_distance( form, reference ) >= NAME_MODERATE_DIFF) {
        pch->pecho( _("Эта форма слишком отличается от твоего имени (%s) -- "
                      "такую замену должен подтвердить бессмертный. Обратись к нему."),
                    reference.c_str( ) );
        return;
    }

    if (lang == LANG_EN) {
        pch->setEnglishName( form );
    }
    else {
        // Russian/Ukrainian decline: turn the nominative the player typed into a
        // Flexer pad via the morphology sidecar (gender from the character's sex).
        DLString gender = "-";
        if (pch->getSex( ) == SEX_MALE)   gender = "masc";
        if (pch->getSex( ) == SEX_FEMALE) gender = "femn";

        if (lang == LANG_RU)
            pch->setRussianName( Morphology::declineRu( form, "NOUN", gender ) );
        else
            pch->setUkrainianName( Morphology::declineUa( form, "NOUN", gender ) );
    }

    pch->pecho( _("Готово. Теперь на этом языке тебя видят как: %s"),
                pch->getNameP( '1', lang ).c_str( ) );
}
