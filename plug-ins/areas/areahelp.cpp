/* $Id$
 *
 * ruffina, 2004
 */
#include "areahelp.h"
#include "plugininitializer.h"
#include "mocregistrator.h"
#include "merc.h"
#include "mercdb.h"
#include "dl_strings.h"

/*-------------------------------------------------------------------
 * AreaHelp 
 *------------------------------------------------------------------*/
const DLString AreaHelp::TYPE = "AreaHelp";

void AreaHelp::getRawText( Character *ch, ostringstream &in ) const
{
    AREA_DATA *area = areafile->area;
    
    if (!selfHelp) {
        in << *this;
        return;
    }

    in << "Зона {Y" << area->name << "{x, " 
       << "уровни {Y" << area->low_range << "-" << area->high_range << "{x, "
       << "автор {y" << area->authors << "{x";
    if (str_cmp(area->translator, ""))
        in << ", перевод {y" << area->translator << "{x";
    in << endl
       << endl;

    if (!empty())
       in << *this << endl;
    
    if (str_cmp(area->speedwalk, ""))
        in << "{yКак добраться{x: " << area->speedwalk << endl;
}

PluginInitializer<MocRegistrator<AreaHelp> > initAreaHelp;
