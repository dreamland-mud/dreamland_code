#include "character.h"
#include "commandtemplate.h"
#include "areahelp.h"
#include "websocketrpc.h"
#include "areabehaviorplugin.h"
#include "comm.h"
#include "act.h"
#include "dl_strings.h"
#include "merc.h"
#include "def.h"

CMDRUNP( areas )
{
    ostringstream buf, areaBuf, clanBuf, mansionBuf;
    int acnt = 0, ccnt = 0, mcnt = 0;
    AreaIndexData *pArea;
    int minLevel, maxLevel, level;
    DLString arguments( argument ), args, arg1, arg2;
    
    arguments.stripWhiteSpace( );
    level = minLevel = maxLevel = -1;
    args = arguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments.getOneArgument( );
    
    if (!arg1.empty( ) && !arg2.empty( ) && arg1.isNumber( ) != arg2.isNumber( )) {
        ch->println( "Использование: \r\n"
                     "{lEareas [<level> | <min level> <max level> | <string>]"
                     "{lRзоны [<уровень> | <мин.уровень> <макс.уровень> | <название>]{lx" );
        return;
    }
    
    try {
        if (arg1.isNumber( )) {
            level = minLevel = arg1.toInt( );
            args.clear( );
        }

        if (arg2.isNumber( )) {
            level = -1;
            maxLevel = arg2.toInt( );
            args.clear( );
        }
    } catch (const ExceptionBadType & ) {
        ch->send_to( "Уровень зоны указан неверно.\r\n" );
        return;
    }
    
    if (level != -1) 
        buf << "{YАрии мира Dream Land для уровня " << level << ":{x" << endl;
    else if (!args.empty( ))
        buf << "{YНайдены арии: {x" << endl;
    else if (minLevel != -1 && maxLevel != -1)
        buf << "{YАрии мира Dream Land, для уровней " 
            << minLevel << " - " << maxLevel << ":{x" << endl;
    else
        buf << "{YВсе арии мира Dream Land: {x" << endl;
    
    buf << "Название                Сложность        Название                Сложность" << endl
        << "-------------------------------------------------------------------------------" << endl;

    for (pArea = area_first; pArea; pArea = pArea->next) {
        if (IS_SET(pArea->area_flag, AREA_HIDDEN|AREA_SYSTEM)) 
            continue;
        
        // Filter areas by level or level range.
        if (level != -1) {
            if (pArea->low_range > level || pArea->high_range < level)
                continue;
        }
        else if (minLevel != -1 && maxLevel != -1) {
            if (pArea->low_range < minLevel || pArea->high_range > maxLevel)
                continue;
        }

        // Filter areas by name.
        if (!args.empty( )) {
            DLString aname = DLString( pArea->name ).colourStrip( );
            DLString altname = DLString( pArea->altname ).colourStrip( );
            DLString acredits = DLString( pArea->credits ).colourStrip( );
            if (!is_name( args.c_str( ), aname.c_str( ) ) 
                    && !is_name( args.c_str( ), acredits.c_str( ) )
                    && !is_name( args.c_str( ), altname.c_str( ) ))
                continue;
        }
        
        bool isMansion = area_is_mansion(pArea);
        bool isClan = area_is_clan(pArea);    
        ostringstream &str =  isMansion ? mansionBuf : isClan ? clanBuf : areaBuf;
        int &cnt = isMansion ? mcnt : isClan ? ccnt : acnt;
        const char *nameFmt = (isMansion || isClan) ? "%-40.40s " : "%-23.23s ";

        // Make area name a clickable link to area help, if present.
        AreaHelp *ahelp = area_selfhelp(pArea);
        int hid = ahelp && !help_is_empty(*ahelp) ? ahelp->getID() : 0;
        if (hid > 0) {
            DLString aname = pArea->name;
            str << fmt(ch, web_cmd(ch, "help " + DLString(hid), nameFmt).c_str(), aname.colourStrip().c_str());
        } else {            
            str << fmt(ch, nameFmt, pArea->name);
        }

        // For normal areas, display level range and danger level.
        if (!isClan && !isMansion) {
            DLString levels;
            if (area_has_levels(pArea))
                levels << pArea->low_range << "-" << pArea->high_range;
            str << fmt(ch, "%7s", levels.c_str());            

            DLString danger = area_danger_short(pArea);
            str << " " << fmt(ch, "%6s   ", danger.c_str());
        }

        if (++cnt % 2 == 0)
            str << endl;
    }

    if (!areaBuf.str().empty()) { 
        buf << areaBuf.str();
        if (acnt % 2)
            buf << endl;
    }

    if (!clanBuf.str().empty()) {
        buf << endl << "{yКлановые территории:{x" << endl << clanBuf.str();
        if (ccnt % 2)
            buf << endl;
    }

    if (!mansionBuf.str().empty()) {
        buf << endl << "{yПригороды под застройку:{x" << endl << mansionBuf.str();
        if (mcnt % 2)
            buf << endl;
    }
    buf << endl << "Подробнее о каждой зоне смотри в {Wсправка название_зоны{x." << endl;
    page_to_char( buf.str( ).c_str( ), ch );        
}


