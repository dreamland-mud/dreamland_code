#include "areaquestutils.h"
#include "logstream.h"
#include "util/regexp.h"
#include "xmlmap.h"
#include "fenia/register-impl.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "wrapperbase.h"
#include "wrappertarget.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "areaquest.h"
#include "xmlattributeareaquest.h"
#include "fenia_utils.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;
using namespace std;

/** Contains result of a method name like 'q3001_step2_begin_onGive' split into fields. */
struct MethodLabel {
    MethodLabel() : questId(0), step(-1), methodId("unknown") { 
    }
    MethodLabel(const DLString &method) : methodId(method) {
        this->methodName = method;
    }

    IdRef methodId;
    DLString methodName; // 'q2300_step1_end_onSpeech'
    DLString stage;    // 'end'
    int questId;       // 2300
    int step;          // 1
    DLString trigName; // 'onSpeech'
    DLString trigType; // 'Speech'
    DLString trigPrefix; // 'on'
};

/** maps stage name to a corresponding method label */
typedef map<DLString, MethodLabel> MethodsByStage;

/** maps step number to methods defined on this mob/obj/room for this step (grouped by stage) */
typedef map<int, MethodsByStage> MethodsByStep;

/** maps quest id to methods defined on this mob/obj/room for this quest (grouped by steps) */
typedef map<int, MethodsByStep> MethodsByQuest;

/** Finds all methods on a target for given trigger name. Will return both _onGive and _postGive for 'Give' trigType. */
static MethodsByQuest aquest_find_methods(WrapperBase *wrapperBase, const DLString &trigType)
{
    MethodsByQuest result;
    StringSet onAndPostTriggers, miscMethods;

    wrapperBase->collectTriggers(onAndPostTriggers, miscMethods);

    // Regexp: q<number> _ step<number> _ begin|end _ on|post Give|Use|Get|...  
    // Matches:    0              1           2         3          4
    RegExp methodIdRE("^q([0-9]+)_step([0-9]+)_(begin|end)_([a-z]+)([A-Z][A-Za-z]+)$", true);

    for (auto method: miscMethods) {
        RegExp::MatchVector matches = methodIdRE.subexpr(method.c_str());
        if (matches.empty())
            continue;

        DLString matchedTrigType = matches.at(4);
        if (matchedTrigType != trigType)
            continue;

        MethodLabel label(method);
        label.questId = matches.at(0).toInt();
        label.step = matches.at(1).toInt();
        label.stage = matches.at(2);
        label.trigName = matches.at(3) + matches.at(4);
        label.trigType = matches.at(4);
        label.trigPrefix = matches.at(3);

        result[label.questId][label.step][label.stage] = label;
    }

    return result;
}

// Grab (create if needed) quest info for this quest from player attributes
AreaQuestData & aquest_data(PCMemoryInterface *ch, const DLString &questId)
{
    auto areaQuestAttr = ch->getAttributes().getAttr<XMLAttributeAreaQuest>("areaquest");
    auto q = areaQuestAttr->find(questId);

    if (q == areaQuestAttr->end())
        (**areaQuestAttr)[questId] = AreaQuestData();

    return (**areaQuestAttr)[questId];
}

// Construct method name for given quest & step & stage and trigger name
DLString aquest_method_id(AreaQuest *q, int step, bool isBegin, const DLString &trigName)
{
    ostringstream buf;

    buf << "q" << q->vnum << "_step" << step << "_" << (isBegin ? "begin" : "end") << "_" << trigName;
    return buf.str();
}

AreaQuest *get_area_quest(const DLString &questId)
{
    Integer vnum;

    if (!Integer::tryParse(vnum, questId))
        return 0;

    return get_area_quest(vnum);
}

AreaQuest *get_area_quest(int vnum)
{
    auto q = areaQuests.find(vnum);
    if (q == areaQuests.end())
        return 0;

    return q->second;

}
AreaQuest *get_area_quest(const Integer &vnum)
{
    return get_area_quest(vnum.getValue());
}

/** If ch cannot take part in a quest - return non-empty string with a reason; otherwise - empty string. */
static DLString aqprog_canstart(PCharacter *ch, AreaQuest *q)
{
    FENIA_STR_CALL(q, "CanStart", "C", ch);
    return DLString::emptyString;
}

// Return true if player can satisfy all requirements at some point in life
bool aquest_can_participate_ever(PCMemoryInterface *pci, AreaQuest *q) 
{
    // Exclude onboarding quests
    if (q->flags.isSet(AQUEST_ONBOARDING))
        return false;

    // Wrong align?
    if (q->align != 0 && !q->align.isSetBitNumber(ALIGNMENT(pci)))
        return false;

    // Wrong hometown?
    if (!q->hometowns.empty() && !q->hometowns.isSet(pci->getHometown()))
        return false;

    // Wrong player class?
    if (!q->classes.empty() && !q->classes.isSet(pci->getProfession()))
        return false;

    // See if prerequisite quest would be available too 
    if (q->prereq > 0) {
        AreaQuest *prereq = get_area_quest(q->prereq);
        if (!prereq)
            return false;
        if (!aquest_can_participate_ever(pci, prereq))
            return false;
    }

    return true;        
}

// Return true if ch passes all requirements to participate in this quest
bool aquest_can_participate(PCMemoryInterface *ch, AreaQuest *q, const AreaQuestData &qdata) 
{
    // Too old?
    if (q->maxLevel < LEVEL_MORTAL && ch->getLevel() > q->maxLevel)
        return false;

    // Too young?
    if (q->minLevel > 0 && ch->getLevel() < q->minLevel)
        return false;

    // Have done too many times?
    if (q->limitPerLife > 0 && qdata.thisLife >= q->limitPerLife)
        return false;

    // A day hasn't passed yet?
    if (q->oncePerDay) {
        if (qdata.timeend > 0 && (dreamland->getCurrentTime() - qdata.timeend < Date::SECOND_IN_DAY))
            return false;
    }

    // Standard 1 hour delay after 'q cancel' for non-onboarding quests
    if (qdata.timecancel > 0 && !q->flags.isSet(AQUEST_ONBOARDING)) {
        if (dreamland->getCurrentTime() - qdata.timecancel < Date::SECOND_IN_HOUR)
            return false;
    }

    // Wrong align?
    if (q->align != 0 && !q->align.isSetBitNumber(ALIGNMENT(ch)))
        return false;

    // Wrong hometown?
    if (!q->hometowns.empty() && !q->hometowns.isSet(ch->getHometown()))
        return false;

    // Wrong player class?
    if (!q->classes.empty() && !q->classes.isSet(ch->getProfession()))
        return false;

    // See if prerequisite quest was completed by ch at least once in this life
    if (q->prereq > 0) {
        AreaQuestData &prereqQuestData = aquest_data(ch, q->prereq.toString());
        if (prereqQuestData.thisLife <= 0)
            return false;
    }

    return true;        
}

// Return true only if synchronous onXXX trigger exists and returned true -- meaning we can advance to next step
static bool aquest_method_call(WrapperBase *wrapperBase, MethodLabel &method, const RegisterList &progArgs)
{
    Register progFun;

    if (!wrapperBase->triggerFunction(method.methodId, progFun))
        return false;

    if (method.trigPrefix == "on") {
        Register rc;

        if (!wrapperBase->call(rc, method.methodId, progFun, progArgs))
            return false;

        return rc.type != Register::NONE && rc.toBoolean();
    }

    if (method.trigPrefix == "post") {
        wrapperBase->postpone(method.methodId, progFun, progArgs);
    }

    return false;
}

// Locate a MethodLabel structure for given step and stage (gotta love c++).
static MethodLabel * aquest_method_for_step_and_stage(MethodsByStep &methodsByStep, int step, const DLString &stage)
{
    auto ms = methodsByStep.find(step);

    if (ms != methodsByStep.end()) {
        auto &methodsByStage = ms->second;
        auto ss = methodsByStage.find(stage);
        if (ss != methodsByStage.end())
            return &ss->second;
    }

    return 0;
}

/** Main 'brains' for running Fenia triggers for area quests. Return true if at least one trigger on this mob/obj/room
 * was successful.
 */
static bool aquest_trigger(WrapperBase *wrapperBase, PCharacter *ch, const DLString &trigType, const RegisterList &progArgs)
{
    bool calledOnce = false;
    MethodsByQuest methodsForQuest = aquest_find_methods(wrapperBase, trigType);

    // Handle all quests this player participates or can start participating in
    for (auto mq: methodsForQuest) {
        int questId = mq.first;
        MethodsByStep &methodsByStep = mq.second;

        // Find corresponding area quest structure.
        AreaQuest *q = get_area_quest(questId);
        if (!q) {
            LogStream::sendError() << "AreaQuest: unknown quest id " << questId << endl;
            continue;
        }

        // Find (or create) quest info structure from player attributes, for this quest.
        AreaQuestData &qdata = aquest_data(ch, questId);
        int myStep = qdata.step;

        // Check if this inactive quest can be started
        if (!qdata.questActive()) {
            // only limit participation on quest start - otherwise ch can grow out of quest between steps
            if (!aquest_can_participate(ch, q, qdata)) 
                continue;

            // Launch quest's onCanStart trigger if defined
            if (aqprog_canstart(ch, q) != DLString::emptyString)
                continue;

            myStep = 0;
        }

        // Anything happens for this mob/obj/room on our step?
        MethodLabel *beginMethod = aquest_method_for_step_and_stage(methodsByStep, myStep, "begin");
        MethodLabel *endMethod = aquest_method_for_step_and_stage(methodsByStep, myStep, "end");

        // Only run begin trigger for new quest participants
        if (beginMethod && !qdata.questActive()) {
            aquest_method_call(wrapperBase, *beginMethod, progArgs);

            qdata.start();

            // Call global Fenia trigger to show hints about quest commands
            gprog("onQuestStart", "CQ", ch, q);           
            
            calledOnce = true;
            continue;
        }

        // Only run end trigger for active quests
        if (endMethod && qdata.questActive()) {
            // Call the end trigger to see if player has done what's needed to finish the step
            bool stepEnded = aquest_method_call(wrapperBase, *endMethod, progArgs);

			if (!stepEnded)
				continue;

            calledOnce = true;

            // Call global Fenia trigger to distribute rewards
            gprog("onQuestStepEnd", "CQi", ch, q, qdata.step); 

			qdata.advance();

			if (qdata.step >= q->steps.size()) {
				qdata.complete();
                // Call global Fenia trigger to say congrats etc
                gprog("onQuestComplete", "CQ", ch, q);           
                continue;
            }

            // Call matching post trigger type from the next step
            MethodLabel *nextBeginMethod = aquest_method_for_step_and_stage(methodsByStep, qdata.step, "begin");
            if (nextBeginMethod) {
                if (nextBeginMethod->trigType == trigType && nextBeginMethod->trigPrefix == "post")
                    aquest_method_call(wrapperBase, *nextBeginMethod, progArgs);
                else
                    LogStream::sendError() << "aquest: begin/end trigger type mismatch " 
                        << q->vnum << ", " << endMethod->methodName << ", " << nextBeginMethod->methodName << endl;
            }

            continue;
        }

        // Done with this quest for this player
    }

    return calledOnce;
}


/** Call all area quest triggers defined on mob for this trigType. */
bool aquest_trigger(Character *mob, Character *ch, const DLString &trigType, const char *fmt, ...)
{
    // Call quest triggers on mobs, for players
    if (!ch || !mob->is_npc() || ch->is_npc())
        return false;

    WrapperBase *wrapperBase = get_wrapper(mob->getNPC()->pIndexData->wrapper);
    if (wrapperBase == 0)
        return false;

    bool rc = false;
    va_list ap;    
    RegisterList progArgs;

    va_start(ap, fmt);
    wrapperBase->triggerArgs(progArgs, fmt, ap);
    rc = aquest_trigger(wrapperBase, ch->getPC(), trigType, progArgs);
    va_end(ap);

    return rc;
}

/** Call all area quest triggers defined on obj for this trigType. */
bool aquest_trigger(::Object *obj, Character *ch, const DLString &trigType, const char *fmt, ...)
{
    // Call quest triggers for players
    if (!ch || ch->is_npc())
        return false;

    WrapperBase *wrapperBase = get_wrapper(obj->pIndexData->wrapper);
    if (wrapperBase == 0)
        return false;

    bool rc = false;
    va_list ap;    
    RegisterList progArgs;
    
    va_start(ap, fmt);
    wrapperBase->triggerArgs(progArgs, fmt, ap);
    rc = aquest_trigger(wrapperBase, ch->getPC(), trigType, progArgs);
    va_end(ap);

    return rc;
}

/** Call all area quest triggers defined on room for this trigType. */
bool aquest_trigger(Room *room, Character *ch, const DLString &trigType, const char *fmt, ...)
{
    // Call quest triggers for players
    if (!ch || ch->is_npc())
        return false;

    WrapperBase *wrapperBase = get_wrapper(room->wrapper);
    if (wrapperBase == 0)
        return false;

    bool rc = false;
    va_list ap;    
    RegisterList progArgs;
    
    va_start(ap, fmt);
    wrapperBase->triggerArgs(progArgs, fmt, ap);
    rc = aquest_trigger(wrapperBase, ch->getPC(), trigType, progArgs);
    va_end(ap);

    return rc;
}

