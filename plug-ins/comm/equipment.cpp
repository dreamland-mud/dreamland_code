#include "commandtemplate.h"
#include "character.h"
#include "core/object.h"
#include "wearlocation.h"
#include "player_utils.h"
#include "arg_utils.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

bool show_char_equip( Character *ch, Character *victim, ostringstream &buf, bool fShowEmpty );

/*---------------------------------------------------------------------------
 * 'equipment' command 
 *--------------------------------------------------------------------------*/
CMDRUNP( equipment )
{
    ostringstream buf;
    bool naked;
    // Empty slots are a wall of 'nothing.' lines (painful on a screen reader),
    // so only worn items are listed; 'equipment all' shows every slot.
    bool showAll = arg_is_all( DLString( argument ).getOneArgument( ) );

    naked = show_char_equip( ch, ch, buf, showAll );

    if (naked) {
        ch->pecho( _("На тебе совсем ничего нет -- не мешало бы одеться!") );
        return;
    }

    ch->pecho( _("Ты используешь:") );
    ch->send_to( buf );

    if (showAll)
        return;

    Wearlocation::DisplayList eq;
    wearlocationManager->display( ch, eq, Player::lang( ch ) );

    int empty = 0;
    for (Wearlocation::DisplayList::iterator e = eq.begin( ); e != eq.end( ); e++)
        if (!e->second)
            empty++;

    if (empty > 0)
        ch->pecho( _("Свободных слотов: {W%1$d{x. Показать все: {y{hcэкипировка все{x."), empty );
}

