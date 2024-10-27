#include "pcharacter.h"
#include "string_utils.h"
#include "commandtemplate.h"
#include "player_utils.h"
#include "mudtags.h"
#include "merc.h"
#include "def.h"
#include <arg_utils.h>

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
        ch->pecho( "Слишком длинный титул." );
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
        ch->pecho( "Ты не можешь сменить титул." );
        return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
        ostringstream buf;
        const DLString &title = pch->getTitle( );
        DLString parsed = Player::title(pch);
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
        pch->pecho( "Титул удален." );
        return;
    }

    if (!fix_title( pch, arg ))
        return;

    pch->setTitle( arg );

    pch->pecho("Теперь ты {W%C1{x%s{x", pch, Player::title(pch).c_str());
}

static bool fix_pretitle( PCharacter *ch, DLString &title )
{
    ostringstream buf;
    mudtags_convert( 
        title.c_str( ), buf, 
        TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOWEB|TAGS_ENFORCE_NOCOLOR|TAGS_ENFORCE_RAW );

    DLString stripped = buf.str( );
    DLString nospace = stripped;
    nospace.stripWhiteSpace( );
   
    if (stripped.size( ) > 25) {
        ch->pecho( "Слишком длинный претитул!" );
        return false;
    }
    
    if (nospace.size( ) != stripped.size( )) {
        ch->pecho( "В начале или в конце претитула не должно быть пробелов." );
        return false;
    }
    
    for (unsigned int i = 0; i < stripped.size( ); i++)
        if (!dl_isalpha( stripped[i] ) 
                && stripped[i] != ' ' 
                && stripped[i] != '\'') 
        {
            ch->pecho( "В претитуле разрешено использовать только буквы, пробелы и одинарные кавычки." );
            return false;
        }

    if (stripped.size( ) != title.size( )) {
        DLString buf;
        buf << "{1" << title << "{x{2";
        title = buf;
    }

    return true;
}

CMDRUNP( pretitle )
{
    PCharacter *pch = ch->getPC( );
    DLString arg = argument;
    DLString rus;

    if (!pch)
        return;

    if (IS_SET(pch->act, PLR_NO_TITLE)) {
         pch->pecho( "Ты не можешь изменить претитул.");
         return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
        DLString eng = pch->getPretitle( );
        DLString rus = pch->getRussianPretitle( );

        pch->pecho( "Твой претитул: %s\r\nРусский претитул: %s",
                     (eng.empty( ) ? "(нет)" : eng.c_str( )),
                     (rus.empty( ) ? "(нет)" : rus.c_str( )) );
        return;
    }
    
    if (arg == "clear" || arg == "очистить") {
        pch->setPretitle( DLString::emptyString );
        pch->setRussianPretitle( DLString::emptyString );
        pch->pecho("Русский и английский претитулы очищены.");
        return;
    }
    
    rus = arg.getOneArgument( );

    if (rus == "rus" || rus == "рус") {
        if (!fix_pretitle( pch, arg ))
            return;

        pch->setRussianPretitle( arg );
        pch->pecho( "Русский претитул: %s", arg.c_str( ) );
    }
    else { 
        arg = argument;

        if (!fix_pretitle( pch, arg ))
            return;

        pch->setPretitle( arg );
        pch->pecho( "Твой претитул: %s", arg.c_str( ) );
    }
}

