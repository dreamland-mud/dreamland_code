/* $Id$
 *
 * ruffina, 2004
 */
#include "markuphelparticle.h"
#include "helpformatter.h"
#include "websocketrpc.h"
#include "character.h"
#include "descriptor.h"

// Get a list of additional keywords not mentioned in the title.
DLString help_article_disambig(const HelpArticle *help)
{
    DLString ltitle = help->getTitle(DLString::emptyString);
    ltitle.toLower();

    const StringSet &keywords = help->getAllKeywords();
    StringSet disambig;
    for (auto &kw: keywords) {
        DLString kwLower = kw.toLower();
        if (ltitle.find(kwLower) == DLString::npos)
            disambig.insert(kwLower);
    }

    return disambig.toString();
}

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
    if (ch && ch->desc && ch->desc->connected == CON_PLAYING) {
        DLString title = getTitle(DLString::emptyString);
        DLString disambig = help_article_disambig(this);

        in << "{WСправка на тему {C" << title << "{x " << editButton(ch) << endl;
    if (!disambig.empty())
        in << "{DКлючевые слова: " << disambig << "{x" << endl << endl;
    }

    in << *this;
}

void MarkupHelpArticle::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    DefaultHelpFormatter( in.str( ).c_str( ) ).run( ch, out );
}


