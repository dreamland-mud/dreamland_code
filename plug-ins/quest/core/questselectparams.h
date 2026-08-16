/* Dream Land, 2026 */
#ifndef QUESTSELECTPARAMS_H
#define QUESTSELECTPARAMS_H

#include <set>

#include "dlstring.h"

/** The knobs a Fenia scenario turns before asking the engine to pick a target.
 *
 *  Everything here is off by default, and off means "exactly what the C++ quest
 *  models have always selected". So a type that wants no filtering at all passes
 *  nothing and gets the historical behaviour.
 *
 *  These are the per-type conditions that used to be hardcoded in the eight C++
 *  quest classes, turned into data. Each field below exists because some real
 *  code or some real player report needs it -- resist adding one in advance. An
 *  untested knob that silently does the wrong thing is worse than a missing one,
 *  and adding a field later costs a rebuild, which this migration is going to
 *  pay for anyway on its own schedule.
 */
struct QuestSelectParams {
    QuestSelectParams( )
        : levelDiffSet( false ), levelDiffMin( 0 ), levelDiffMax( 0 ),
          maxLevel( 0 ), noCaster( false ), visible( false ),
          carriedByNpc( false ), requireActFlag( 0 ), roomNoCast( false )
    {
    }

    /** Candidate's level minus the hero's modified level must land inside
     *  [levelDiffMin, levelDiffMax]. KillQuest's four difficulty modes are four
     *  such windows (-3..0, 0..5, 5..10, 10..15). */
    bool levelDiffSet;
    int levelDiffMin;
    int levelDiffMax;

    /** Hard cap on the candidate's own level, 0 for none. Report 3069: a level
     *  17 player sent to kill something in the Abyss, level 40-55. A relative
     *  window alone does not stop that when the hero is level-drained or the
     *  window is wide. */
    int maxLevel;

    /** Skip spellcasters. KillQuest's hardest mode refuses them. */
    bool noCaster;

    /** Require that the hero can actually see the candidate right now. Reports
     *  29108 and 30879: quest loot and quest targets nobody could see, so the
     *  quest was unfinishable from the moment it was handed out. */
    bool visible;

    /** Skip candidates carrying this named behavior WHILE they stand in a
     *  hometown area; empty for none. That is KillQuest's cityguard rule exactly:
     *  the guards of a town under law are not fair game, the same kind of mob
     *  outside one still is. */
    DLString noBehaviorInHometown;

    /** The item must be in an NPC's hands, and that NPC must be able to hand it
     *  straight back: it passes the client checks, it can see the item, and it
     *  is not already at its carry limit in either count or weight. All one
     *  condition because half of it is useless -- StealQuest::checkItem has
     *  always tested the four together, and an item held by someone who cannot
     *  give it away makes an unfinishable quest. */
    bool carriedByNpc;

    /** Candidate must be one of these prototype vnums; empty for no restriction.
     *  LocateQuest's scenarios name their customers this way, and so do the big
     *  and kidnap ones. Not expressible by retrying a random pick: a short list
     *  against a world of 10^4 mobs effectively never comes up. */
    std::set<int> vnums;

    /** Candidate must carry this act flag, or this behavior, or both if both are
     *  set -- ANY match passes, and a field left unset does not restrict.
     *
     *  That OR is not a design choice, it is StealQuest::isThief: a mob counts
     *  as a thief if it has ACT_THIEF, or the `thief` behavior, or its name is in
     *  the scenario's own list. The third is scenario data and stays in Fenia. */
    int requireActFlag;
    DLString requireBehavior;

    /** Room filters, applied wherever a client room is judged -- which covers
     *  both picking a room directly and picking a mob standing in one. */
    bool roomNoCast;        /**< reject ROOM_NO_CAST; HealQuest cannot heal there */
    DLString excludeAreaName;  /**< reject rooms in this area, by its RUSSIAN name.
                                *   Pinned to one language on purpose: LocateQuest
                                *   compares, it does not display, and a viewer's
                                *   language deciding whether a room matches would
                                *   make the same quest behave differently per
                                *   reader. */
};

#endif
