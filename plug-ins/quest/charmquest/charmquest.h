/* Dream Land, 2026 */
#ifndef CHARMQUEST_H
#define CHARMQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"
#include "questregistrator.h"
#include "questscenario.h"

/** Charm quest: charm the target with your own ability and lead it, as a
 *  charmed follower, back to the questor who asked for it.
 *
 *  The type is Fenia-born: unlike the eight older types it never had a C++
 *  engine, so this class is only the registration anchor -- the name every
 *  tally is counted under, and the cpp-engine fallback that declines rather
 *  than hand out a quest no C++ code can run. All real logic lives under
 *  dreamland_fenia/autoquest/charm.
 */
class CharmQuest : public VictimQuestModel {
XML_OBJECT
public:
    typedef ::Pointer<CharmQuest> Pointer;

    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual QuestReward::Pointer reward( PCharacter *, NPCharacter * );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
};

class CharmQuestRegistrator : public QuestRegistrator<CharmQuest> {
XML_OBJECT
public:
    virtual bool applicable( PCharacter *, bool ) const;

    /** Charm-capable abilities, from CharmQuest.xml <skills>. Same rule as
     *  HealQuestRegistrator's "knows a remedy": the type is offered only to a
     *  hero who can actually charm, so nobody gets an unwinnable quest. The
     *  Fenia scenario keeps the same list inline (autoquest/charm/onCreate) to
     *  shape the target -- change one, change both. */
    XML_VARIABLE XMLStringVector skills;
};

#endif
