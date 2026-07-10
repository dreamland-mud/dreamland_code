#include "commandtemplate.h"
#include "character.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

bool do_look_extradescr( Character *ch, const char *arg, int number );

/*-------------------------------------------------------------------------
 * 'read' command 
 *-------------------------------------------------------------------------*/
CMDRUNP( read )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int number;

    if (eyes_blinded( ch )) {
        eyes_blinded_msg( ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    number = number_argument( arg1, arg2 );

    if (!do_look_extradescr( ch, arg2, number ))
        ch->pecho( _("Ты не видишь этого тут.") );
}
