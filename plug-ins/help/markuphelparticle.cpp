/* $Id$
 *
 * ruffina, 2004
 */
#include "markuphelparticle.h"
#include "helpformatter.h"
#include "websocketrpc.h"

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


void MarkupHelpArticle::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Справка на тему {C" << getKeyword() << "{x: "
       << "%PAUSE% " << web_edit_button(ch, "hedit", getID()) << "%RESUME%" << endl;

    in << *this;
}

void MarkupHelpArticle::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    DefaultHelpFormatter( in.str( ).c_str( ) ).run( ch, out );
}


