/* $Id: selfrate.cpp,v 1.1.2.5 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "so.h"
#include "commandtemplate.h"
#include "selfrate.h"

#include "pcharacter.h"

#include "loadsave.h"
#include "infonet.h"
#include "messengers.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "def.h"

bool rated_as_newbie( PCMemoryInterface* pcm )
{
    return pcm->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" )->rate == 0;
}
bool rated_as_expert( PCMemoryInterface* pcm )
{
    return pcm->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" )->rate == 1;
}
bool rated_as_guru( PCMemoryInterface* pcm )
{
    return pcm->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" )->rate == 2;
}



CMDRUN( selfrate )
{
    int rate;
    XMLAttributeSelfRate::Pointer attr;
    DLString args = constArguments;
    DLString arg = args.getOneArgument( );
    PCharacter *pch = ch->getPC( );
    
    if (!pch) {
        ch->send_to( "Ты обычное животное.\r\n" );
        return;
    }

    if (IS_CHARMED(pch)) {
        pch->send_to( "Ничего не произошло.\r\n" );
        return;
    }
        
    attr = pch->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" );

    if (arg.empty( )) {
        pch->printf( "Твой уровень опытности: {W%s{x.\r\n", attr->getRateAlias( ).c_str( ) );
        return;
    }


    if (arg_oneof_strict(arg, "newbie", "ньюби"))
        rate = 0;
    else if (arg_oneof_strict(arg, "expert", "эксперт"))
        rate = 1;
    else if (arg_oneof_strict(arg, "guru", "гуру"))
        rate = 2;
    else {
        pch->printf( "Выбери, кто ты: {lRньюби, эксперт или гуру{lEnewbie, expert или guru{x.\r\n" );
        return;
    }

    if (rate == attr->rate) 
        pch->printf( "Но ты и так %s!\r\n", attr->getRateAlias( ).c_str( ));
    else if (rate < attr->rate) 
        pch->send_to( "Ты не можешь понизить оценку своего уровня опытности.\r\n" );
    else {
        attr->rate = rate;
        DLString alias = attr->getRateAlias();
        pch->printf( "Поздравляем! Теперь ты {W%s{x.\r\n", alias.c_str( ));

        DLString what = fmt(0, "%#^C1 теперь %s!", pch, attr->getRateAlias(pch).c_str());
        infonet(0, 0, "{CТоржественный голос из $o2: ", what.c_str());
        send_discord_orb(what);
    }
}

static const char *rate_alias [] = { "newbie", "expert", "guru" };
static const char *rate_alias_ru [] = { "ньюби", "эксперт", "гуру" };

XMLAttributeSelfRate::XMLAttributeSelfRate( ) 
{
}

DLString XMLAttributeSelfRate::getRateAlias( PCharacter *looker ) const
{
    short r = rate.getValue();

    if (r > 2 || r < 0)
        return "unknown!";
    
    ostringstream buf;

    if (looker) {
        buf << (looker->getConfig().rucommands ? rate_alias_ru[r] : rate_alias[r]);
    } else {
        buf << "{lE" << rate_alias[r] << "{lR" << rate_alias_ru[r] << "{lx";
    }

    return buf.str();
}

bool XMLAttributeSelfRate::handle( const WhoisArguments &args )
{
    DLString l;
    
    l << "самоуверенность уровня {W" << getRateAlias(args.looker) << "{x";
    args.lines.push_back( l );
    return true;
}

extern "C"
{
    SO::PluginList initialize_selfrate( ) {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeSelfRate> >( ppl );
        
        return ppl;
    }
}

