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
#include "act.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * 'ban' command
 *---------------------------------------------------------------------------*/
void CBan::doShow( const Ban & rec, ostringstream &buf )
{
    time_t exp = rec.expire.getTime();
    
    DLString expmsg = "never";
    if(exp)
        expmsg = Date::getStringFromSecond(exp - time(0));
    
    int flag = rec.flags.getValue();
    
    buf << fmt(0, 
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

void CBan::doBan( const DLString & constArguments, ostringstream &buf )
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
            buf << "Ban index out of range." << endl;
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
            rec.responsible.setValue( "" );
            rec.pattern.setValue( patt );
            rec.flags.setValue( BAN_ALL );
            rec.expire = Date(0);
        }
    }
    
    while(!arguments.empty( )) {
        DLString arg = arguments.getOneArgument();

        if(arg.strPrefix("none") || arg.strPrefix("off")) {
            if(it == bm->end())
                buf << "Not banned" << endl;
            else {
                bm->erase(it);
                buf << "Ban deleted" << endl;
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
                    buf << "Oops... error parsing time" << endl;
                    return;
                }
            }
            rec.expire = Date(exp ? exp + time(0) : 0);
            changed = true;
        } else if(arg.strPrefix("comment") || arg.strPrefix("reason")) {
            rec.comment.setValue(arguments.getOneArgument());
            changed = true;
        } else {
            buf << fmt(0, "Unknown option %s", arg.c_str( )) << endl;
            doUsage( buf );
            return;
        }
    }

    if(!changed && it == bm->end()) {
        buf << "Not banned" << endl;
        return;
    }

    doShow(rec, buf);
    
    if(changed) {
        if(it == bm->end()) {
            buf << "New ban" << endl;
            bm->push_back(rec);
        } else {
            *it = rec;
        }

        bm->save();
    }
}

void CBan::action(const DLString &constArguments, ostringstream &buf)
{
    DLString arguments = constArguments, patt;
    
    if (arguments.empty( )) {
        doUsage( buf );
        return;
    }
    
    patt = arguments.getOneArgument( );

    if(patt.strPrefix("list")) {
        doList( buf );
    }
    else if (patt.strPrefix("kick")) {
        doKick( buf );
    } else {
        doBan(constArguments, buf);
    }     
}

COMMAND(CBan, "ban")
{
    ostringstream buf;

    CBan::action(constArguments, buf);
    ch->send_to(buf);
}

void CBan::doUsage( ostringstream &buf )
{
   buf 
    << 
        "Использование: \r\n"
        "ban list                      - список банов\r\n"       
        "ban {{<pattern>|<index>}       - подробности про бан\r\n"
        "ban {{<pattern>|<index>} [off|none|all|player|newbie|confirm|communicate] [expire <timespec>] [comment <reason>] - добавить/изменить/удалить бан"
    << endl;
}

void CBan::doKick( ostringstream &buf )
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
    
   buf << fmt(0, "%d descriptors kicked.", cnt ) << endl;
}

void CBan::doList(ostringstream &buf)
{
    BanManager::iterator it;
    BanManager *bm = BanManager::getThis();
    int i;
    
    if(bm->empty()) {
        buf << "Nothing is banned. Strange..." << endl;
        return;
    }

   buf << " # | pattern                        | flags    | until          | comment" << endl;

    for(i=0, it = bm->begin(); it != bm->end(); it++, i++) {
        const Date &until = it->expire;
        time_t exp = until.getTime();
        
        DLString untmsg = "forever";
        if(exp)
            untmsg = until.getTimeAsString("%d/%m/%y %H:%M");
        
        int flag = it->flags.getValue();
        
       buf << fmt(0, "%2d | %-30s | %-8s | %-14s | %-10.10s", i,
                it->pattern.getValue().c_str(), 
                ban_flags.names( flag ).c_str( ),
                untmsg.c_str(),
                it->comment.getValue().c_str()) << endl;
    }
}

