/* Dream Land, 2026 */
#include "charmquest.h"
#include "questexceptions.h"

#include "skillmanager.h"
#include "skill.h"
#include "pcharacter.h"
#include "npcharacter.h"

/*--------------------------------------------------------------------
 * CharmQuest -- the cpp-engine fallback only, see the header.
 *--------------------------------------------------------------------*/
void CharmQuest::create( PCharacter *pch, NPCharacter *questman )
{
    // This type was born with <engine>fenia</engine> and has no C++ engine to
    // fall back to. If the config is ever flipped to cpp, decline every request
    // rather than hand out a quest nothing can run -- QuestCannotStartException
    // is the sanctioned "pick another type" path (questmanager catches it).
    throw QuestCannotStartException( );
}

bool CharmQuest::isComplete( )
{
    return state == QSTAT_FINISHED;
}

QuestReward::Pointer CharmQuest::reward( PCharacter *, NPCharacter * )
{
    // Unreachable while create() declines; a valid empty reward keeps the
    // contract if a stray pfile quest of this class ever completes.
    QuestReward::Pointer r( NEW );
    return r;
}

void CharmQuest::info( std::ostream &, PCharacter * )
{
}

void CharmQuest::shortInfo( std::ostream &, PCharacter * )
{
}

/*--------------------------------------------------------------------
 * CharmQuestRegistrator
 *--------------------------------------------------------------------*/
bool CharmQuestRegistrator::applicable( PCharacter *pch, bool fAuto ) const
{
    if (!QuestRegistratorBase::applicable( pch, fAuto ))
        return false;

    // Only a hero who can actually charm gets this type. Mirrors
    // HealQuestRegistrator's remedy scan: any listed ability learned to 50%
    // qualifies. An empty <skills> list in CharmQuest.xml switches the type off.
    for (XMLStringVector::const_iterator s = skills.begin( ); s != skills.end( ); s++) {
        Skill *skill = skillManager->find( *s );

        if (skill && skill->getEffective( pch ) >= 50)
            return true;
    }

    return false;
}
