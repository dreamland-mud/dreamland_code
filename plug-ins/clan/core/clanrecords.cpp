#include "clanrecords.h"
#include "clantypes.h"
#include "clanorg.h"
#include "clanreference.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "fenia/register-impl.h"
#include "idcontainer.h"
#include "lex.h"
#include "regcontainer.h"
#include "l10n.h"
#include "merc.h"

using namespace Scripting;

const char *CLAN_OFFICE_LEADER = "leader";
const char *CLAN_OFFICE_RECRUITER = "recruiter";

/** Tenure ranks 5..8 need this many hours in the clan. */
static const int TENURE_HOURS[] = { 100, 250, 500, 1000 };
static const int TENURE_FIRST_RANK = 5;
static const int TENURE_LAST_RANK = 8;
/** Tenure only climbs from the top donation rank. */
static const int DONATION_TOP_RANK = 4;
static const time_t DECAY_OFFLINE = 180 * 24 * 60 * 60;

XMLClanRecord::XMLClanRecord()
        : since(-1)
{
}

bool XMLAttributeClanRecords::handle( const RemortArguments &args )
{
    clan_freeze( args.pch );
    return RemortAttribute::handle( args );
}

Scripting::Register XMLAttributeClanRecords::toRegister() const
{
    Register recsReg = Register::handler<RegContainer>();
    RegContainer *recs = recsReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto &r: records) {
        Register recReg = Register::handler<IdContainer>();
        IdContainer *rec = recReg.toHandler().getDynamicPointer<IdContainer>();

        rec->setField(IdRef("rank"), r.second.rank.getValue());
        rec->setField(IdRef("tenure"), (int)(r.second.tenure.getValue() / 3600));
        rec->setField(IdRef("donated"), r.second.donated.getValue());
        rec->setField(IdRef("office"), r.second.office.getValue());
        recs->setField(r.first, recReg);
    }

    return recsReg;
}

/** A clan one can belong to: not 'none' or another dispersed pseudo-clan, not a dumb reference. */
static bool clan_is_real( const Clan &clan )
{
    return clan.getMembership( ) != 0 && !clan.isDispersed( );
}

bool clan_is_reformed( const Clan &clan )
{
    return clan_is_real( clan ) && clan.getMembership( )->reformed.getValue( );
}

static bool is_honorary( PCMemoryInterface *pcm )
{
    return pcm->get_trust( ) >= LEVEL_IMMORTAL;
}

XMLClanRecord * clan_record( PCMemoryInterface *pcm, const DLString &clanName, bool create )
{
    XMLAttributeClanRecords::Pointer attr;

    if (create)
        attr = pcm->getAttributes( ).getAttr<XMLAttributeClanRecords>( "clanrec" );
    else
        attr = pcm->getAttributes( ).findAttr<XMLAttributeClanRecords>( "clanrec" );

    if (!attr)
        return 0;

    XMLAttributeClanRecords::Records::iterator r = attr->records.find( clanName );
    if (r != attr->records.end( ))
        return &r->second;

    if (!create)
        return 0;

    return &attr->records[clanName];
}

long clan_tenure( PCMemoryInterface *pcm, const DLString &clanName )
{
    XMLClanRecord *rec = clan_record( pcm, clanName, false );
    if (!rec)
        return 0;

    long tenure = rec->tenure.getValue( );
    PCharacter *pc = dynamic_cast<PCharacter *>( pcm );

    if (pc && pc->getClan( )->getName( ) == clanName && rec->since.getValue( ) >= 0) {
        long now = pc->age.getTrueTime( );
        if (now > rec->since.getValue( ))
            tenure += now - rec->since.getValue( );
    }

    return tenure;
}

void clan_bank_tenure( PCharacter *pc )
{
    if (is_honorary( pc ) || !clan_is_real( *pc->getClan( ) ))
        return;

    XMLClanRecord *rec = clan_record( pc, pc->getClan( )->getName( ), true );
    if (!rec)
        return;

    long now = pc->age.getTrueTime( );

    // true_played restarts at remort, a stale stamp must not count backwards.
    if (rec->since.getValue( ) >= 0 && now > rec->since.getValue( ))
        rec->tenure.setValue( rec->tenure.getValue( ) + now - rec->since.getValue( ) );

    rec->since.setValue( now );
}

bool clan_promote_tenure( PCharacter *pc )
{
    const Clan &clan = *pc->getClan( );
    int rank = pc->getClanLevel( );

    if (is_honorary( pc ) || !clan_is_reformed( clan ))
        return false;

    if (rank < DONATION_TOP_RANK || rank >= TENURE_LAST_RANK)
        return false;

    long hours = clan_tenure( pc, clan.getName( ) ) / 3600;
    int target = rank;

    for (int i = 0; i <= TENURE_LAST_RANK - TENURE_FIRST_RANK; i++)
        if (hours >= TENURE_HOURS[i] && TENURE_FIRST_RANK + i > target)
            target = TENURE_FIRST_RANK + i;

    if (target <= rank)
        return false;

    pc->setClanLevel( target );
    pc->pecho( _("{WТвой стаж в клане растет: теперь у тебя клановый ранг %d.{x"), target );
    pc->updateSkills( );
    return true;
}

DLString clan_office( PCMemoryInterface *pcm )
{
    const Clan &clan = *pcm->getClan( );

    if (!clan_is_reformed( clan ))
        return DLString::emptyString;

    XMLClanRecord *rec = clan_record( pcm, clan.getName( ), false );
    return rec ? rec->office.getValue( ) : DLString::emptyString;
}

DLString clan_set_office( PCMemoryInterface *pcm, const DLString &office )
{
    const Clan &clan = *pcm->getClan( );

    if (!clan_is_reformed( clan ))
        return "not a member of a reformed clan";

    if (is_honorary( pcm ))
        return "immortals are honorary patrons and hold no office";

    if (!office.empty( ) && office != CLAN_OFFICE_LEADER && office != CLAN_OFFICE_RECRUITER)
        return "unknown office";

    XMLClanRecord *rec = clan_record( pcm, clan.getName( ), true );
    if (!rec)
        return "clanrec attribute is broken";

    if (office == CLAN_OFFICE_LEADER) {
        for (auto &p: PCharacterManager::getPCM( )) {
            PCMemoryInterface *other = p.second;

            if (other == pcm || other->getClan( ) != clan)
                continue;

            XMLClanRecord *otherRec = clan_record( other, clan.getName( ), false );
            if (otherRec && otherRec->office.getValue( ) == CLAN_OFFICE_LEADER) {
                otherRec->office.setValue( DLString::emptyString );
                PCharacterManager::saveMemory( other );
            }
        }
    }

    rec->office.setValue( office );
    PCharacterManager::saveMemory( pcm );
    return DLString::emptyString;
}

void clan_freeze( PCMemoryInterface *pcm )
{
    const Clan &clan = *pcm->getClan( );

    if (is_honorary( pcm ) || !clan_is_real( clan ))
        return;

    if (PCharacter *pc = dynamic_cast<PCharacter *>( pcm ))
        clan_bank_tenure( pc );

    XMLClanRecord *rec = clan_record( pcm, clan.getName( ), true );
    if (!rec)
        return;

    if (clan_is_reformed( clan )) {
        rec->rank.setValue( pcm->getClanLevel( ) );
        rec->office.setValue( DLString::emptyString );
    }

    rec->since.setValue( -1 );
}

void clan_induct( PCMemoryInterface *pcm, const Clan &clan )
{
    bool sameClan = (pcm->getClan( ) == clan);
    int rank = 0;

    if (!sameClan)
        clan_freeze( pcm );

    if (sameClan && clan_is_reformed( clan ))
        rank = pcm->getClanLevel( );
    else if (clan_is_reformed( clan ) && !is_honorary( pcm )) {
        XMLClanRecord *rec = clan_record( pcm, clan.getName( ), false );
        if (rec)
            rank = rec->rank.getValue( );
    }

    pcm->setClan( clan.getName( ) );
    pcm->setPetition( DLString( "none" ) );
    pcm->setClanLevel( rank );

    // Start counting from now: stamp the session clock, or leave it to the first bank if offline.
    if (!sameClan && clan_is_real( clan ) && !is_honorary( pcm )) {
        XMLClanRecord *rec = clan_record( pcm, clan.getName( ), true );
        if (rec) {
            PCharacter *pc = dynamic_cast<PCharacter *>( pcm );
            rec->since.setValue( pc ? pc->age.getTrueTime( ) : -1 );
        }
    }
}

void clan_remove( PCMemoryInterface *pcm, bool bySelf )
{
    const Clan &clan = *pcm->getClan( );
    const ClanMembership *m = clan.getMembership( );

    clan_freeze( pcm );

    if (m)
        pcm->setClan( bySelf ? m->removeSelf : m->removeBy );
    else
        pcm->setClan( DLString( "none" ) );

    pcm->setClanLevel( 0 );
    ClanOrgs::delAttr( pcm );
}

bool clan_decay( PCMemoryInterface *pcm, time_t now )
{
    if (is_honorary( pcm ) || pcm->isOnline( ))
        return false;

    if (now - pcm->getLastAccessTime( ).getTime( ) < DECAY_OFFLINE)
        return false;

    bool changed = false;
    XMLAttributeClanRecords::Pointer attr = pcm->getAttributes( ).findAttr<XMLAttributeClanRecords>( "clanrec" );

    if (attr) {
        for (auto &r: attr->records) {
            XMLClanRecord &rec = r.second;

            if (rec.tenure.getValue( ) != 0) {
                rec.tenure.setValue( 0 );
                changed = true;
            }
            if (rec.rank.getValue( ) > DONATION_TOP_RANK) {
                rec.rank.setValue( DONATION_TOP_RANK );
                changed = true;
            }
            if (!rec.office.getValue( ).empty( )) {
                rec.office.setValue( DLString::emptyString );
                changed = true;
            }
        }
    }

    if (clan_is_reformed( *pcm->getClan( ) ) && pcm->getClanLevel( ) > DONATION_TOP_RANK) {
        pcm->setClanLevel( DONATION_TOP_RANK );
        changed = true;
    }

    return changed;
}
