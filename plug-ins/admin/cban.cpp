/* $Id: cban.cpp,v 1.1.4.8.6.3 2007/09/11 00:33:45 rufina Exp $
 *
 * ruffina, 2005
 */

#include "cban.h"

#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"


#include "autoflags.h"
#include "ban.h"
#include "descriptor.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * 'ban' command
 *---------------------------------------------------------------------------*/
void CBan::doShow( Character* ch, const Ban & rec)
{
    time_t exp = rec.expire.getTime();
    
    DLString expmsg = "never";
    if(exp)
        expmsg = Date::getStringFromSecond(exp - time(0));
    
    int flag = rec.flags.getValue();
    
    ch->printf(
            "Host: %s\r\n"
            "Created by: %s\r\n"
            "Flag: %s\r\n"
            "Expires: %s\r\n"
            "Comment: %s\r\n",
                rec.pattern.getValue().c_str(),
                rec.responsible.getValue().c_str(),
                ban_flags.names( flag ).c_str( ),
                expmsg.c_str(),
                rec.comment.getValue().c_str()
            );
}

void CBan::doBan( Character* ch, const DLString & constArguments)
{
    bool changed = false;
    DLString arguments = constArguments, patt;
    BanManager *bm = BanManager::getThis();
    BanManager::iterator it;
    Ban rec;

    patt = arguments.getOneArgument( );

    if(patt.isNumber()) {
        unsigned int n = patt.toInt();

        if(n >= bm->size()) {
            ch->send_to("ban index out of range\r\n");
            return;
        }
        
        it = bm->begin() + n;
    } else {
        for(it = bm->begin();it != bm->end(); it++)
            if(it->pattern.getValue() == patt) {
                rec = *it;
                break;
            }

        if(it == bm->end()) {
            rec.responsible.setValue( ch->getName( ) );
            rec.pattern.setValue( patt );
            rec.flags.setValue( BAN_ALL );
            rec.expire = Date(0);
        }
    }
    
    while(!arguments.empty( )) {
        DLString arg = arguments.getOneArgument();

        if(arg.strPrefix("none") || arg.strPrefix("off")) {
            if(it == bm->end())
                ch->send_to("not baned\r\n");
            else {
                bm->erase(it);
                ch->send_to("ban deleted\r\n");
                bm->save();
            }
            return;
        } else if(arg.strPrefix("newbie")) {
            rec.flags.setValue(BAN_NEWBIES);
            changed = true;
        } else if(arg.strPrefix("player")) {
            rec.flags.setValue(BAN_PLAYER);
            changed = true;
        } else if(arg.strPrefix("confirm")) {
            rec.flags.setValue(BAN_CONFIRM);
            changed = true;
        } else if(arg.strPrefix("communicate")) {
            rec.flags.setValue(BAN_COMMUNICATE);
            changed = true;
        } else if(arg.strPrefix("all")) {
            rec.flags.setValue(BAN_ALL);
            changed = true;
        } else if(arg.strPrefix("expire")) {
            DLString sexp = arguments.getOneArgument();
            int exp = 0;
            
            if(!sexp.strPrefix("never")) {
                try {
                    exp = Date::getSecondFromString(sexp);
                } catch(...) {
                }

                if(exp == 0) {
                    ch->send_to("oops... error parsing time\r\n");
                    return;
                }
            }
            rec.expire = Date(exp ? exp + time(0) : 0);
            changed = true;
        } else if(arg.strPrefix("comment") || arg.strPrefix("reason")) {
            rec.comment.setValue(arguments.getOneArgument());
            changed = true;
        } else {
            ch->printf("unknown option %s\r\n", arg.c_str( ));
            doUsage( ch );
            return;
        }
    }

    if(!changed && it == bm->end()) {
        ch->send_to("not baned\r\n");
        return;
    }

    doShow(ch, rec);
    
    if(changed) {
        if(it == bm->end()) {
            ch->send_to("new ban\r\n");
            bm->push_back(rec);
        } else {
            *it = rec;
        }

        bm->save();
    }
}

COMMAND(CBan, "ban")
{
    DLString arguments = constArguments, patt;
    
    if (arguments.empty( )) {
        ch->send_to("Pattern missing\r\n");
        doUsage( ch );
        return;
    }
    
    patt = arguments.getOneArgument( );

    if(patt.strPrefix("list")) {
        doList( ch );
    }
    else if (patt.strPrefix("kick")) {
        doKick( ch );
    } else {
        doBan(ch, constArguments);
    }
}

void CBan::doUsage( Character *ch )
{
    ch->send_to( 
        "Использование: \r\n"
        "ban list                      - список банов\r\n"       
        "ban {<pattern>|<index>}       - подробности про бан\r\n"
        "ban {<pattern>|<index>} [off|none|all|player|newbie|confirm|communicate] [expire <timespec>] [comment <reason>] - добавить/изменить/удалить бан\r\n ");
}

void CBan::doKick( Character *ch )
{
    ostringstream ostr;
    Descriptor *d, *d_next;
    int cnt = 0;
    
    for ( d = descriptor_list; d != 0; d = d_next ) {
        d_next = d->next;
        
        if (banManager->check( d, BAN_ALL )) {
            d->close( );
            cnt++;
        }
    }
    
    ch->printf( "%d descriptors kicked.\r\n", cnt );
}

void CBan::doList( Character *ch )
{
    BanManager::iterator it;
    BanManager *bm = BanManager::getThis();
    int i;
    
    if(bm->empty()) {
        ch->send_to("Nothing is banned. Strange...\n");
        return;
    }

    ch->send_to(" # | pattern                        | flags    | until          | comment\r\n");

    for(i=0, it = bm->begin(); it != bm->end(); it++, i++) {
        const Date &until = it->expire;
        time_t exp = until.getTime();
        
        DLString untmsg = "forever";
        if(exp)
            untmsg = until.getTimeAsString("%d/%m/%y %H:%M");
        
        int flag = it->flags.getValue();
        
        ch->printf("%2d | %-30s | %-8s | %-14s | %-10.10s\r\n", i,
                it->pattern.getValue().c_str(), 
                ban_flags.names( flag ).c_str( ),
                untmsg.c_str(),
                it->comment.getValue().c_str());
    }
}

