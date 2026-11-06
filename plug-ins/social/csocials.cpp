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
#include "screenreader.h"
#include "def.h"
#include "l10n.h"
#include "websocketrpc.h"
#include "arg_utils.h"

#include <algorithm>
#include <vector>

static const int SOCIAL_NAME_WIDTH = 12;

/** The clickable keyword, padded to the column by its VISIBLE length: the
 *  {hh...{hx markup is stripped by the client, so padding the marked-up string
 *  would push every description six characters out of line. */
static DLString social_link( const DLString &name )
{
    DLString buf = DLString("{hh") + name + "{hx";

    for (int i = (int)name.size(); i < SOCIAL_NAME_WIDTH; i++)
        buf += " ";

    return buf;
}

static DLString social_column( const DLString &value )
{
    DLString buf = value;

    for (int i = (int)value.size(); i < SOCIAL_NAME_WIDTH; i++)
        buf += " ";

    return buf;
}

CMDRUN( socials )
{
    ostringstream buf;
    lang_t lang = viewerLang(ch);

    // The Latin keyword is always typeable, so it is always the first column.
    // A second name column only says something where it differs from it.
    bool showNative = (lang != LANG_EN);

    // Load order by default; `socials rus` (or ua) sorts by the native-name
    // column instead. compareRussian, because byte order scrambles KOI8 Cyrillic.
    vector<const Social *> socials;
    for (auto &e: SocialManager::getThis( )->getElements( ))
        socials.push_back( e.getConstPointer<Social>( ) );

    DLString arg = DLString( constArguments ).getOneArgument( );
    if (showNative && arg_oneof( arg, "rus", "рус", "ua", "укр" ))
        std::sort( socials.begin( ), socials.end( ),
                   [lang]( const Social *a, const Social *b ) {
                       return a->getNameFor( lang ).compareRussian( b->getNameFor( lang ) ) < 0;
                   } );

    vector<const Social *>::const_iterator i;

    WebList web(ch, "socials");
    for (i = socials.begin( ); web.active( ) && i != socials.end( ); i++) {
        const Social *s = *i;
        DLString label = s->getName( );

        if (showNative)
            label += " (" + s->getNameFor( lang ) + ")";
        label += " -- " + s->getShortDescFor( lang );

        int row = web.row( label, s->getName( ) );
        web.action( row, "do", s->getName( ) );
        web.action( row, "social_at", s->getName( ) );
        web.action( row, "help", "help " + s->getName( ) );
    }
    web.send( );

    // No table for a screen reader: one social per line, read straight through.
    if (uses_screenreader( ch )) {
        for (i = socials.begin( ); i != socials.end( ); i++) {
            const Social *s = *i;

            buf << "{hh" << s->getName( ) << "{hx";
            if (showNative)
                buf << " (" << s->getNameFor( lang ) << ")";
            buf << " -- " << s->getShortDescFor( lang ) << endl;
        }

        buf << endl
            << fmt(ch, _("См. справку по каждому социалу, чтобы увидеть, как он выглядит."))
            << endl;
        page_to_char( buf.str( ).c_str( ), ch );
        return;
    }

    buf << "{W==============================================================================={x" << endl;

    if (showNative)
        buf << " " << social_column( l(ch, "Название") )
            << "{W|{x " << social_column( l(ch, "По-русски") )
            << "{W|{x " << l(ch, "Описание") << endl
            << "{W-------------+-------------+---------------------------------------------------{x" << endl;
    else
        buf << " " << social_column( l(ch, "Название") )
            << "{W|{x " << l(ch, "Описание") << endl
            << "{W-------------+-----------------------------------------------------------------{x" << endl;

    for (i = socials.begin( ); i != socials.end( ); i++) {
        const Social *s = *i;

        buf << " {c" << social_link( s->getName( ) ) << "{x{W|{x ";

        if (showNative)
            buf << social_column( s->getNameFor( lang ) ) << "{W|{x ";

        buf << s->getShortDescFor( lang ) << endl;
    }

    buf << fmt(ch, _("См. справку по каждому социалу, чтобы увидеть, как он выглядит.")) << endl;
    page_to_char( buf.str( ).c_str( ), ch );
}
