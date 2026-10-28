/* $Id$
 *
 * ruffina, 2004
 */
#include "markuphelparticle.h"
#include "helpformatter.h"
#include "helprelated.h"
#include "websocketrpc.h"
#include "character.h"
#include "pcharacter.h"
#include "player_utils.h"
#include "descriptor.h"
#include "l10n.h"

// Get a list of additional keywords not mentioned in the title.
DLString help_article_disambig(const HelpArticle *help, lang_t lang)
{
    DLString ltitle = help->getTitle(DLString::emptyString, lang);
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

    // In game only, like the title header: the website gets the same list as
    // the 'related' field of the help dump and lays it out itself.
    if (ch && ch->desc && ch->desc->connected == CON_PLAYING) {
        DLString related = help_related_line(this, ch);

        if (!related.empty()) {
            DLString body = out.str();
            if (!body.empty() && body.at(body.size() - 1) != '\n')
                out << endl;
            out << endl << related << endl;
        }
    }

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
        DLString title = getTitle(DLString::emptyString, Player::displayLang(ch));
        DLString disambig = help_article_disambig(this, Player::displayLang(ch));

        in << l(ch, "{WСправка на тему {C") << title << "{x " << editButton(ch) << endl;
        if (!disambig.empty())
            in << l(ch, "{DКлючевые слова: ") << disambig << "{x" << endl << endl;
    }

    in << text.getForLang(Player::displayLang(ch));
}

void MarkupHelpArticle::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    DefaultHelpFormatter( in.str( ).c_str( ) ).run( ch, out );
}


