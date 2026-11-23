#include "pcharacter.h"
#include "string_utils.h"
#include "commandtemplate.h"
#include "player_utils.h"
#include "mudtags.h"
#include "merc.h"
#include "def.h"
#include <arg_utils.h>
#include "l10n.h"
#include "accountmanager.h"
#include "commonattributes.h"
#include "dreamland.h"

static bool fix_title( PCharacter *ch, DLString &title )
{
    if (DLString( "{" ).strSuffix( title )
        && !DLString( "{{" ).strSuffix( title ))
    {
        title = String::truncate(title, title.length( ) - 1 );
    }

    title.replaces( "{/", "" );
    title.replaces( "{*", "" );
    title.replaces( "{+", "" );

    if (title.colourStrip().size() > 50) {
        ch->pecho( _("Слишком длинный титул.") );
        return false;
    }
    
    return true;
}

CMDRUNP( title )
{
    DLString arg = argument;
    PCharacter *pch = ch->getPC( );

    if (ch->is_npc( ))
        return;

    if (IS_SET(ch->act, PLR_NO_TITLE)) {
        ch->pecho( _("Ты не можешь сменить титул.") );
        return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
        ostringstream buf;
        const DLString &title = pch->getTitle( );
        DLString parsed = Player::title(pch, Player::displayLang(pch));
        parsed.stripWhiteSpace( );
        
        if (parsed.empty( )) {
            buf << "У тебя нет титула." << endl;
        }
        else {
            buf << "Ты носишь титул " << parsed << "{x";
            if (parsed != title)
                buf << " (" << title << "{x)";
            buf << "." << endl;
        }

        pch->send_to( buf );
        return;
    }

    if (arg == "clear" || arg == "очистить") {
        pch->setTitle( DLString::emptyString );
        pch->pecho( _("Титул удален.") );
        return;
    }

    if (!fix_title( pch, arg ))
        return;

    pch->setTitle( arg );

    pch->pecho(_("Теперь ты {W%C1{x%s{x"), pch, Player::title(pch, Player::displayLang(pch)).c_str());
}

static bool fix_pretitle( PCharacter *ch, DLString &title )
{
    // Only colour codes may pass. Any other tag ({l, {S, {I, {h, push/pop...) leaves
    // a letter behind after colour stripping and would stay open into the name.
    static const char *colours = "bcgmrwyBCGMRWYDxX";
    for (DLString::size_type i = 0; i < title.size( ); i++)
        if (title[i] == '{') {
            if (i + 1 >= title.size( ) || title[i + 1] == 0 || !strchr( colours, title[i + 1] )) {
                ch->pecho( _("В претитуле разрешены только цветовые коды.") );
                return false;
            }
            i++;
        }

    ostringstream buf;
    mudtags_convert( 
        title.c_str( ), buf, 
        TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOWEB|TAGS_ENFORCE_NOCOLOR|TAGS_ENFORCE_RAW );

    DLString stripped = buf.str( );
    DLString nospace = stripped;
    nospace.stripWhiteSpace( );
   
    if (stripped.size( ) > 25) {
        ch->pecho( _("Слишком длинный претитул!") );
        return false;
    }
    
    if (nospace.size( ) != stripped.size( )) {
        ch->pecho( _("В начале или в конце претитула не должно быть пробелов.") );
        return false;
    }
    
    for (unsigned int i = 0; i < stripped.size( ); i++)
        if (!dl_isalpha( stripped[i] ) 
                && stripped[i] != ' ' 
                && stripped[i] != '\'') 
        {
            ch->pecho( _("В претитуле разрешено использовать только буквы, пробелы и одинарные кавычки.") );
            return false;
        }

    if (stripped.size( ) != title.size( )) {
        DLString buf;
        buf << "{1" << title << "{x{2";
        title = buf;
    }

    return true;
}

// Colour in a pretitle is a Ko-fi supporter perk. Saving a pretitle while the
// account supports marks it coloured; saving one without support unmarks it,
// so after a lapse the colour lasts until the next edit. The old remort
// purchase (remorts.pretitle) keeps colour forever and skips all this.
// Call before setPretitle: the cached name is rebuilt there.
static void pretitle_colour_rights( PCharacter *pch, const DLString &arg )
{
    if (pch->getRemorts( ).pretitle)
        return;

    DLString id = AccountManager::accountOf( pch->getName( ) );
    bool supporter = !id.empty( )
        && AccountManager::supporterUntil( id ) > dreamland->getCurrentTime( );

    if (supporter) {
        pch->getAttributes( ).getAttr<XMLStringAttribute>( "pretitle_color" )->setValue( "1" );
        return;
    }

    pch->getAttributes( ).eraseAttribute( "pretitle_color" );

    if (arg.find( '{' ) != DLString::npos)
        pch->pecho( _("Цвета в предтитуле видны только у тех, кто поддерживает игру. Подробнее: {hh5273поддержка{x.") );
}

CMDRUNP( pretitle )
{
    PCharacter *pch = ch->getPC( );
    DLString arg = argument;
    DLString rus;

    if (!pch)
        return;

    if (IS_SET(pch->act, PLR_NO_TITLE)) {
         pch->pecho( _("Ты не можешь изменить претитул."));
         return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
        DLString eng = pch->getPretitle( );
        DLString rus = pch->getRussianPretitle( );

        pch->pecho( _("Твой претитул: %s\r\nРусский претитул: %s"),
                     (eng.empty( ) ? "(нет)" : eng.c_str( )),
                     (rus.empty( ) ? "(нет)" : rus.c_str( )) );
        return;
    }
    
    if (arg == "clear" || arg == "очистить") {
        pretitle_colour_rights( pch, DLString::emptyString );
        pch->setPretitle( DLString::emptyString );
        pch->setRussianPretitle( DLString::emptyString );
        pch->pecho(_("Русский и английский претитулы очищены."));
        return;
    }
    
    rus = arg.getOneArgument( );

    if (rus == "rus" || rus == "рус") {
        if (!fix_pretitle( pch, arg ))
            return;

        pretitle_colour_rights( pch, arg );
        pch->setRussianPretitle( arg );
        pch->pecho( _("Русский претитул: %s"), arg.c_str( ) );
    }
    else { 
        arg = argument;

        if (!fix_pretitle( pch, arg ))
            return;

        pretitle_colour_rights( pch, arg );
        pch->setPretitle( arg );
        pch->pecho( _("Твой претитул: %s"), arg.c_str( ) );
    }
}

