/* $Id$
 *
 * ruffina, 2004
 */
#include "markuphelparticle.h"
#include "helpformatter.h"
#include "websocketrpc.h"
#include "character.h"
#include "descriptor.h"

MarkupHelpArticle::~MarkupHelpArticle( )
{
}

DLString MarkupHelpArticle::getText( Character *ch ) const
{
    ostringstream in, out;

    getRawText( ch, in );
    applyFormatter( ch, in, out );

    return out.str( );
}

void MarkupHelpArticle::editButton(Character *ch, ostringstream &in) const
{
    in << "%PAUSE% " << web_edit_button(ch, "hedit", getID()) << "%RESUME%";
}

DLString MarkupHelpArticle::editButton(Character *ch) const
{
    ostringstream buf;
    editButton(ch, buf);
    return buf.str();
}

void MarkupHelpArticle::getRawText( Character *ch, ostringstream &in ) const
{
    if (ch)
        in << "Справка на тему {C" << getKeyword() << "{x: "
        << editButton(ch) << endl;

    in << *this;
}

void MarkupHelpArticle::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    DefaultHelpFormatter( in.str( ).c_str( ) ).run( ch, out );
}


