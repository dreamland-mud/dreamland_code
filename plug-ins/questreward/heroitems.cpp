/* $Id$
 *
 * Hero item helpers shared by the quest trader and the Fenia API.
 */
#include <utility>
#include <vector>

#include "heroitems.h"
#include "pcharacter.h"
#include "save_bank.h"
#include "merc.h"
#include "def.h"

const LangText HERO_ADJECTIVES[3][3] = {
    {
        { "holy", "Священн|ое|ого|ому|ое|ым|ом", "Священн|е|ого|ому|е|им|ому" },
        { "holy", "Священн|ый|ого|ому|ый|ым|ом", "Священн|ий|ого|ому|ий|им|ому" },
        { "holy", "Священн|ая|ой|ой|ую|ой|ой",   "Священн|а|ої|ій|у|ою|ій" },
    },
    {
        { "shimmering", "Мерцающ|ее|его|ему|ее|им|ем", "Мерехтлив|е|ого|ому|е|им|ому" },
        { "shimmering", "Мерцающ|ий|его|ему|ий|им|ем", "Мерехтлив|ий|ого|ому|ий|им|ому" },
        { "shimmering", "Мерцающ|ая|ей|ей|ую|ей|ей",   "Мерехтлив|а|ої|ій|у|ою|ій" },
    },
    {
        { "devilish", "Дьявольск|ое|ого|ому|ое|им|ом", "Диявольськ|е|ого|ому|е|им|ому" },
        { "devilish", "Дьявольск|ий|ого|ому|ий|им|ом", "Диявольськ|ий|ого|ому|ий|им|ому" },
        { "devilish", "Дьявольск|ая|ой|ой|ую|ой|ой",   "Диявольськ|а|ої|ій|у|ою|ій" },
    },
};

const LangText & hero_adjective( Character *ch, int gender )
{
    int a = IS_GOOD(ch) ? 0 : IS_NEUTRAL(ch) ? 1 : 2;
    int g;

    switch (gender) {
    case SEX_MALE:   g = 1; break;
    case SEX_FEMALE: g = 2; break;
    default:         g = 0; break;
    }

    return HERO_ADJECTIVES[a][g];
}

int hero_vault_count( PCharacter *pch, int vnum )
{
    std::vector<std::pair<DLString, DLString> > cells;
    cells.push_back( std::make_pair( DLString( "player" ), pch->getName( ).toLower( ) ) );

    DLString acct = bank_vault_account( pch->getName( ) );
    if (!acct.empty( ))
        cells.push_back( std::make_pair( DLString( "account" ), acct ) );

    DLString clan = pch->getClan( ).getName( ).toLower( );
    if (!clan.empty( ))
        cells.push_back( std::make_pair( DLString( "clan" ), clan ) );

    DLString me = pch->getName( ).toLower( );
    int count = 0;

    for (size_t i = 0; i < cells.size( ); i++) {
        std::vector<BankEntry> entries;
        bank_browse( cells[i].first, cells[i].second, entries );

        for (size_t j = 0; j < entries.size( ); j++)
            if (entries[j].vnum == vnum && entries[j].owner.toLower( ) == me)
                count++;
    }

    return count;
}
