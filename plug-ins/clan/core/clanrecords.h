#ifndef CLANRECORDS_H
#define CLANRECORDS_H

#include <time.h>
#include "xmlattribute.h"
#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmllong.h"
#include "xmlstring.h"
#include "xmlmap.h"
#include "playerattributes.h"

class Clan;
class PCharacter;
class PCMemoryInterface;

/**
 * What a player keeps per clan. For the clan they are in, the live rank is
 * clanLevel and 'rank' is only the value frozen when they last left.
 */
class XMLClanRecord : public XMLVariableContainer {
XML_OBJECT
public:
    XMLClanRecord();

    XML_VARIABLE XMLInteger rank;
    /** Banked seconds of true played time spent in this clan. */
    XML_VARIABLE XMLLong tenure;
    /** True played time at the last bank, -1 = stamp at the next bank. */
    XML_VARIABLE XMLLong since;
    /** qp paid toward the next donation rank. */
    XML_VARIABLE XMLInteger donated;
    /** CLAN_OFFICE_* or empty. Only meaningful for the current clan. */
    XML_VARIABLE XMLString office;
};

/** Per-clan records of a player (attribute 'clanrec'). Survives remort. */
class XMLAttributeClanRecords : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeClanRecords> Pointer;
    typedef XMLMapBase<XMLClanRecord> Records;

    /** Remort drops clan membership: freeze the current clan first, then carry over. */
    virtual bool handle( const RemortArguments & );
    virtual Scripting::Register toRegister() const;

    XML_VARIABLE Records records;
    /** Last time tenure was banked while online, 0 = never. Drives the login decay check. */
    XML_VARIABLE XMLLong lastSeen;
};

extern const char *CLAN_OFFICE_LEADER;
extern const char *CLAN_OFFICE_RECRUITER;

/** A clan one can belong to: not 'none' or another dispersed pseudo-clan, not a dumb reference. */
bool clan_is_real( const Clan &clan );

/** Reform scope switch: <membership><reformed>true</reformed></membership> in the clan XML. */
bool clan_is_reformed( const Clan &clan );

/** The record for this clan, created on demand. NULL if the attribute name is held by another type. */
XMLClanRecord * clan_record( PCMemoryInterface *pcm, const DLString &clanName, bool create );

/** Tenure in seconds, including the unbanked part of the current session. */
long clan_tenure( PCMemoryInterface *pcm, const DLString &clanName );

/** Move true played time since the last bank into the current clan's tenure. */
void clan_bank_tenure( PCharacter *pc );

/** Reformed clans: raise ranks 4-7 whose tenure crossed the next step. True if raised. */
bool clan_promote_tenure( PCharacter *pc );

/** Office held in the current clan, empty outside reformed clans. */
DLString clan_office( PCMemoryInterface *pcm );

/** Set or clear ("") an office in the current reformed clan. A new leader replaces the old one.
 *  Returns a refusal reason, empty on success. Saves every player it touches. */
DLString clan_set_office( PCMemoryInterface *pcm, const DLString &office );

/** Bank tenure and, for a reformed clan, freeze rank and drop the office. Call before setClan. */
void clan_freeze( PCMemoryInterface *pcm );

/** Put the player into a clan. A reformed clan gets the frozen rank back. Doesn't save or announce. */
void clan_induct( PCMemoryInterface *pcm, const Clan &clan );

/** Take the player out of their clan (removeSelf or removeBy). Doesn't save or announce. */
void clan_remove( PCMemoryInterface *pcm, bool bySelf );

/** Offline for 180 days: ranks 5-8 drop to 4, tenure 0 on every record, offices cleared.
 *  Immortals and online players are skipped. True if anything changed. */
bool clan_decay( PCMemoryInterface *pcm, time_t now );

/** On entering the game: apply decay the daily sweep hasn't reached yet, then bank
 *  (creating the current clan's record, so a quick remort can't lose the rank). */
void clan_login( PCharacter *pc );

#endif
