/* $Id: csocials.cpp,v 1.1.2.1.6.2 2008/02/23 13:41:49 rufina Exp $
 *
 * ruffina, 2004
 * short social descriptions and some translations by Thoren 
 */

/* 
 *
 * sturm, 2003
 */

#include "commandtemplate.h"
#include "social.h"
#include "socialmanager.h"

#include "class.h"

#include "pcharacter.h"
#include "comm.h"
#include "act.h"
#include "def.h"

CMDRUN( socials )
{
    ostringstream buf;

    buf << "{W==============================================================================={x" << endl
        <<   " Название   {W|{x По-русски   {W|{x Описание   "  << endl
        << "{W------------+-------------+----------------------------------------------------{x" << endl;
    
    SocialManager::LoadedList::const_iterator i;
    const SocialManager::LoadedList &socials = SocialManager::getThis( )->getElements( );

    for (i = socials.begin( ); i != socials.end( ); i++) {
        const Social *s = i->getConstPointer<Social>( );

        buf << dlprintf( "{c%-10s{x | %-11s| %-s", 
                         s->getName( ).c_str( ), 
                         s->getRussianName( ).c_str( ), 
                         s->getShortDesc( ).c_str( ) )
            << endl;
    }

    page_to_char( buf.str( ).c_str( ), ch );
}


