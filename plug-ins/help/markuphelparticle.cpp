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
#include "helpmeta.h"
#include "clanmanager.h"
#include "clan.h"
#include "merc.h"

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

    DLString body = out.str();

    // In game only, like the title header: the website has its own address for
    // an article and gets the related list as the 'related' field of the dump.
    if (ch && ch->desc && ch->desc->connected == CON_PLAYING) {
        // The id closes the header line, so 'help <id>' brings the reader back.
        if (getID() > 0) {
            DLString tag = DLString(" {D[") + DLString(getID()) + "]{x";
            DLString::size_type eol = body.find('\n');

            if (eol == DLString::npos)
                body << tag;
            else {
                if (eol > 0 && body.at(eol - 1) == '\r')
                    eol--;
                body.insert(eol, tag);
            }
        }

        DLString related = help_related_line(this, ch);

        if (!related.empty()) {
            if (!body.empty() && body.at(body.size() - 1) != '\n')
                body << "\n";
            body << "\n" << related << "\n";
        }
    }

    return body;
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

/** A clan's article ends with the clan's ranks, lowest first, read from the
 *  clan's title table so the list never goes stale. One line per rank. */
static void help_clan_ranks(const HelpArticle *help, Character *ch, ostringstream &in)
{
    if (!clanManager)
        return;

    Clan *clan = clanManager->findByHelpId(help->getID());
    if (!clan)
        return;

    lang_t lang = Player::displayLang(ch);
    StringList male = clan->getRankTitles(lang, SEX_MALE);
    StringList female = clan->getRankTitles(lang, SEX_FEMALE);
    if (male.empty())
        return;

    ostringstream buf;
    StringList::const_iterator f = female.begin();
    int level = 0;

    for (auto &m: male) {
        DLString name = m;

        if (f != female.end()) {
            if (!f->empty() && *f != m) {
                if (name.empty())
                    name = *f;
                else
                    name = name + "/" + *f;
            }
            f++;
        }

        if (!name.empty())
            buf << help_meta_line(DLString(level), name) << endl;
        level++;
    }

    if (buf.str().empty())
        return;

    in << endl << endl << "%PAUSE%{W" << l(ch, "Ранги клана") << "{x" << endl
       << buf.str() << "%RESUME%";
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
    help_clan_ranks(this, ch, in);
}

void MarkupHelpArticle::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    DefaultHelpFormatter( in.str( ).c_str( ) ).run( ch, out );
}


