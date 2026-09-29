#include <climits>
#include "clantreasury.h"
#include "clanrecords.h"
#include "clantypes.h"
#include "clanreference.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "dreamland.h"
#include "logstream.h"
#include "merc.h"

static const int DEFAULT_GOLD_CAP = 100000;
static const int RANK_COUNT = 9;
static const int DEFAULT_RANK_CAP[RANK_COUNT] = { 75, 78, 81, 84, 88, 91, 94, 97, 100 };
/** Cost of donation ranks 1..4, paid one after another. */
static const int DONATION_STEP[] = { 1000, 2000, 4000, 8000 };
static const int DONATION_TOP_RANK = 4;

int clan_gold_cap( const Clan &clan )
{
    const ClanMembership *m = clan.getMembership( );
    if (m && m->goldCap.getValue( ) > 0)
        return m->goldCap.getValue( );
    return DEFAULT_GOLD_CAP;
}

static bool add_overflows( int value, int delta )
{
    return (delta > 0 && value > INT_MAX - delta) || (delta < 0 && value < INT_MIN - delta);
}

DLString clan_bank_add( Clan &clan, int gold, int silver, int qp )
{
    ClanData *data = clan.getData( );
    if (!data || !data->getBank( ))
        return "this clan has no treasury";

    ClanBank::Pointer bank = data->getBank( );

    if (add_overflows( bank->gold, gold ) || add_overflows( bank->silver, silver )
            || add_overflows( bank->questpoints, qp ))
        return "the sum is too large";

    if (bank->gold + gold < 0 || bank->silver + silver < 0 || bank->questpoints + qp < 0)
        return "the treasury doesn't have that much";

    if (clan_is_reformed( clan )) {
        if (silver > 0)
            return "the treasury takes gold only";

        if (gold > 0 && bank->gold + gold > clan_gold_cap( clan ))
            return "the treasury can't hold more gold";
    }

    bank->gold += gold;
    bank->silver += silver;
    bank->questpoints += qp;
    data->save( );

    LogStream::sendNotice( ) << "Clan bank " << clan.getName( ) << ": gold " << gold
                             << " silver " << silver << " qp " << qp << endl;
    return DLString::emptyString;
}

static int rank_table( const XMLVectorBase<XMLInteger> &table, int rank, int fallback )
{
    if (rank < 0 || rank >= (int)table.size( ))
        return fallback;
    return table[rank].getValue( );
}

static const ClanMembership *reform_tables( const Clan &clan )
{
    return clan_is_reformed( clan ) ? clan.getMembership( ) : 0;
}

int clan_rank_cap( const Clan &clan, int rank )
{
    const ClanMembership *m = reform_tables( clan );
    int fallback = (rank >= 0 && rank < RANK_COUNT) ? DEFAULT_RANK_CAP[rank] : 100;
    return m ? rank_table( m->rankCap, rank, fallback ) : 100;
}

int clan_rank_level_bonus( const Clan &clan, int rank )
{
    const ClanMembership *m = reform_tables( clan );
    return m ? rank_table( m->rankLevelBonus, rank, 0 ) : 0;
}

int clan_rank_power( const Clan &clan, int rank )
{
    const ClanMembership *m = reform_tables( clan );
    return m ? rank_table( m->rankPower, rank, 100 ) : 100;
}

bool clan_owns( Clan &clan, const DLString &id )
{
    ClanData *data = clan.getData( );
    return data && data->purchases.find( id ) != data->purchases.end( );
}

DLString clan_purchase( Clan &clan, const DLString &id, const DLString &buyer )
{
    if (!clan_is_reformed( clan ))
        return "this clan has no catalog";

    const ClanMembership *m = clan.getMembership( );
    auto item = m->catalog.find( id );
    if (item == m->catalog.end( ))
        return "no such catalog item";

    if (clan_owns( clan, id ))
        return "the clan owns it already";

    const DLString &prereq = item->second.prerequisite.getValue( );
    if (!prereq.empty( ) && !clan_owns( clan, prereq ))
        return "buy " + prereq + " first";

    int price = item->second.price.getValue( );
    DLString error = clan_bank_add( clan, 0, 0, -price );
    if (!error.empty( ))
        return error;

    ClanPurchase &purchase = clan.getData( )->purchases[id];
    purchase.time.setValue( dreamland->getCurrentTime( ) );
    purchase.buyer.setValue( buyer );
    purchase.price.setValue( price );
    clan.getData( )->save( );

    LogStream::sendNotice( ) << "Clan " << clan.getName( ) << " bought " << id
                             << " for " << price << " qp, buyer " << buyer << endl;
    return DLString::emptyString;
}

DLString clan_donate( PCMemoryInterface *pcm, int qp )
{
    Clan &clan = *pcm->getClan( );

    if (!clan_is_reformed( clan ))
        return "not a member of a reformed clan";

    if (pcm->get_trust( ) >= LEVEL_IMMORTAL)
        return "immortals are honorary patrons and don't donate";

    if (qp <= 0)
        return "the sum must be positive";

    if (pcm->getQuestPoints( ) < qp)
        return "not enough quest points";

    XMLClanRecord *rec = clan_record( pcm, clan.getName( ), true );
    if (!rec)
        return "clanrec attribute is broken";

    DLString error = clan_bank_add( clan, 0, 0, qp );
    if (!error.empty( ))
        return error;

    pcm->setQuestPoints( pcm->getQuestPoints( ) - qp );

    int rank = pcm->getClanLevel( );
    if (rank < DONATION_TOP_RANK) {
        int progress = rec->donated.getValue( ) + qp;

        while (rank < DONATION_TOP_RANK && progress >= DONATION_STEP[rank]) {
            progress -= DONATION_STEP[rank];
            rank++;
        }

        // Past the last donation rank the rest is a plain gift to the treasury.
        rec->donated.setValue( rank < DONATION_TOP_RANK ? progress : 0 );

        if (rank != pcm->getClanLevel( )) {
            pcm->setClanLevel( rank );
            if (PCharacter *pc = pcm->getPlayer( ))
                pc->updateSkills( );
        }
    }

    PCharacterManager::saveMemory( pcm );
    return DLString::emptyString;
}
