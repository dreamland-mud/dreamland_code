#include "commandtemplate.h"
#include "core/object.h"
#include "character.h"
#include "loadsave.h"
#include "fight.h"
#include "wearloc_utils.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

/*
 * DEAD in normal play. The live 'compare' is Fenia (command/compare/runFunc);
 * per WrappedCommand::entryPoint a runFunc override fully replaces this native
 * handler for PCs and NPCs alike, so this body only fires as a boot-safety
 * fall-through if the Fenia command never loaded. Do NOT l10n/"fix" it expecting
 * an in-game effect -- edit the Fenia override instead.
 */
CMDRUNP( compare )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Object *obj1;
    Object *obj2;
    int value1;
    int value2;
    const char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        ch->pecho(_("Сравнить что и с чем.?"));
        return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == 0 )
    {
        ch->pecho(_("У тебя нет этого."));
        return;
    }

    if (arg2[0] == '\0')
    {
        for (obj2 = ch->carrying; obj2 != 0; obj2 = obj2->next_content)
        {
            if (obj2->wear_loc != wear_none
            &&  ch->can_see(obj2)
            &&  obj1->item_type == obj2->item_type
            &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
                break;
        }

        if (obj2 == 0)
        {
            ch->pecho(_("На тебе нет ничего, с чем можно было бы сравнить."));
            return;
        }
    }

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == 0 )
    {
        ch->pecho(_("У тебя нет этого."));
        return;
    }

    msg                = 0;
    value1        = 0;
    value2        = 0;

    if ( obj1 == obj2 )
    {
        msg = "Ты сравниваешь %1$O4 с сам%1$Gим|им|ой|ими собой. Выглядят одинаково.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
        msg = "Ты не можешь сравнить %1$O4 и %2$O4.";
    }
    else
    {
        switch ( obj1->item_type )
        {
        default:
            msg = "Ты не можешь сравнить %1$O4 и %2$O4.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->value0() + obj1->value1() + obj1->value2();
            value2 = obj2->value0() + obj2->value1() + obj2->value2();
            break;

        case  ITEM_WEAPON:
            value1 = weapon_ave(obj1);
            value2 = weapon_ave(obj2);
            break;
        }
    }

    if ( msg == 0 )
    {
             if ( value1 == value2 ) msg = "%1$^O1 и %2$O1 выглядят одинаково.";
        else if ( value1  > value2 ) msg = "%1$^O1 выгляд%1$nит|ят лучше чем %2$O1.";
        else                         msg = "%1$^O1 выгляд%1$nит|ят хуже чем %2$O1.";
    }
    
    ch->pecho( msg, obj1, obj2 );
}


