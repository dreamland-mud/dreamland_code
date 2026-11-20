/* $Id$
 *
 * ruffina, 2004
 */

#include "olc.h"
#include "aedit.h"
#include "hedit.h"
#include "redit.h"
#include "medit.h"
#include "oedit.h"
#include "fedit.h"
#include "sedit.h"
#include "eeedit.h"
#include "reledit.h"
#include "skedit.h"
#include "raceedit.h"
#include "socedit.h"
#include "cmdedit.h"
#include "qedit.h"
#include "bedit.h"
#include "olcstate.h"
#include "onlinecreation.h"
#include "security.h"

#include "dreamland.h"
#include "dlscheduler.h"
#include "schedulertaskroundplugin.h"
#include "plugininitializer.h"
#include "logstream.h"
#include "so.h"
#include "plugin.h"
#include "xmlattributeplugin.h"
#include "mocregistrator.h"
#include "stringset.h"
#include "wiznet.h"
#include "olcops.h"
#include "olcservlet.h"
#include "capturebufferhandler.h"
#include "xmldocument.h"
#include "dlfileop.h"
#include "pcharacter.h"
#include "room.h"
#include "vnum.h"

#include <stdlib.h>
#include <fstream>
#include <sstream>

#include "merc.h"
#include "def.h"

bool save_xmlarea(struct area_file *af, Character *ch);
void save_xmlarea_list( );

// Automatically save all areas marked as 'changed' every minute.
class SaveChangedAreasTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<SaveChangedAreasTask> Pointer;

    virtual void run( ) 
    {
        StringSet savedAreas;

        for(auto &pArea: areaIndexes) {
            if (pArea->changed) {
                if (save_xmlarea(pArea->area_file, 0))
                    savedAreas.insert(pArea->area_file->file_name);
            }
        }

        if (!savedAreas.empty()) {
            wiznet(WIZ_SECURE, 0, 0, "Autosaved %d area(s): %s", savedAreas.size(), savedAreas.toString().c_str());
        }
    }

    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_MINUTE, Pointer( this ) );    
    }

    virtual int getPriority( ) const
    {
        return SCDP_IOWRITE + 10;
    }
};

/*
 * Replay mode of the round-trip gate, a fidelity test of whole-node put:
 * with DL_ROUNDTRIP_REPLAY=1 every node of every area file goes back in as a
 * /api/olc put before the save, so the byte diff shows what put loses.
 */
static DLString replay_node_xml(const XMLNode::Pointer &node)
{
    XMLDocument::Pointer doc(NEW);
    XMLNode::Pointer copy = node->clone(true);
    ostringstream os;

    doc->appendChild(copy);
    doc->save(os);
    return os.str();
}

static DLString replay_child_text(const XMLNode::Pointer &node, const DLString &name)
{
    for (auto &child: node->getNodeList())
        if (child->getName() == name) {
            XMLNode::Pointer text = child->getFirstNode();
            return text ? text->getCData() : DLString::emptyString;
        }

    return DLString::emptyString;
}

static void replay_op(Json::Value &ops, const char *kind, int vnum, const XMLNode::Pointer &node,
                      int area = -1)
{
    Json::Value op;
    op["kind"] = kind;
    op["action"] = "put";
    op["vnum"] = vnum;
    op["xml"] = replay_node_xml(node);
    if (area >= 0)
        op["area"] = area;
    op["continueOnError"] = true;
    ops.append(op);
}

static void replay_area_file(PCharacter *actor, area_file *af, int &total, int &failed)
{
    AreaIndexData *pArea = af->area;
    XMLDocument::Pointer doc(NEW);
    DLFileRead file(dreamland->getAreaDir(), af->file_name, ".xml");
    ifstream is(file.getPath().c_str());

    if (!pArea || !is) {
        LogStream::sendError() << "Round trip replay: " << af->file_name << ": cannot read" << endl;
        failed++;
        return;
    }

    doc->load(is);
    XMLNode::Pointer root = doc->getFirstNode();
    if (!root)
        return;

    // Sections in dependency order: rooms reset mobs and objects, quests and
    // helps hang on the header's vnum range.
    static const char *sections[] = { "areadata", "helps", "quests", "mobiles", "objects", "rooms", 0 };
    Json::Value ops(Json::arrayValue);

    for (int i = 0; sections[i]; i++)
        for (auto &section: root->getNodeList()) {
            if (section->getType() == XMLNode::XML_TEXT || section->getName() != sections[i])
                continue;

            if (i == 0) {
                replay_op(ops, "area", pArea->vnum, section);
                continue;
            }

            for (auto &node: section->getNodeList()) {
                if (node->getType() == XMLNode::XML_TEXT)
                    continue;

                if (i == 1)
                    replay_op(ops, "help", node->getAttribute("id").toInt(), node, pArea->vnum);
                else if (i == 2)
                    replay_op(ops, "quest", replay_child_text(node, "vnum").toInt(), node);
                else
                    replay_op(ops, i == 3 ? "mob" : i == 4 ? "obj" : "room",
                              node->getAttribute("name").toInt(), node);
            }
        }

    if (ops.empty())
        return;

    Json::Value result = OlcOps::apply(actor, ops, OLC_POLICY_IMMORTAL, false, false, false);
    total += ops.size();

    for (auto &err: result["errors"]) {
        int i = err["op"].asInt();
        if (i < 0) {
            LogStream::sendError() << "Round trip replay: " << af->file_name
                                   << " batch: " << err["error"].asString() << endl;
            continue;
        }

        failed++;
        LogStream::sendError() << "Round trip replay: " << af->file_name << " "
                               << ops[i]["kind"].asString() << " " << ops[i]["vnum"].asInt()
                               << ": " << err["error"].asString() << endl;
    }
}

static void roundtrip_replay()
{
    const char *env = getenv("DL_ROUNDTRIP_REPLAY");
    if (!env || DLString(env) != "1")
        return;

    Room *limbo = get_room_instance(ROOM_VNUM_LIMBO);
    if (!limbo) {
        LogStream::sendError() << "Round trip replay: no limbo room, skipped" << endl;
        return;
    }

    int total = 0, failed = 0;

    for (struct area_file *af = area_file_list; af; af = af->next) {
        // A fresh actor per file: its capture buffer only grows.
        CaptureBufferHandler *capture;
        PCharacter *actor = olc_servlet_setup_char(limbo, capture);

        try {
            replay_area_file(actor, af, total, failed);
        } catch (const std::exception &e) {
            LogStream::sendError() << "Round trip replay: " << af->file_name << ": " << e.what() << endl;
            failed++;
        }

        olc_servlet_cleanup_char(actor);
    }

    LogStream::sendNotice() << "Round trip replay: " << total << " ops, " << failed << " failed." << endl;
}

/*
 * Round-trip gate: with the 'roundtrip' boot option the server loads the
 * world, writes every area file back once and shuts down. A diff against
 * the loaded files shows what the serializer does not give back. Runs at
 * boot only, after the areas are loaded and fixed (SCDP_BOOT + 5, + 6) and
 * the item economy pass (+ 7), before players and saved rooms (+ 10, + 20).
 */
class RoundTripSaveTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<RoundTripSaveTask> Pointer;

    virtual void run( )
    {
        if (DLScheduler::getThis( )->getCurrentTick( ) != 0)
            return;

        if (!dreamland->hasOption( DL_ROUNDTRIP ))
            return;

        roundtrip_replay( );

        int total = 0, failed = 0;

        save_xmlarea_list( );

        // One broken area must not stop the rest: the gate wants every file.
        for (struct area_file *af = area_file_list; af; af = af->next) {
            total++;
            try {
                if (!save_xmlarea( af, 0 ))
                    failed++;
            } catch (const std::exception &e) {
                LogStream::sendError( )
                    << "Round trip: " << af->file_name << ": " << e.what( ) << endl;
                failed++;
            }
        }

        LogStream::sendNotice( )
            << "Round trip: saved " << (total - failed) << " of " << total
            << " area files, " << failed << " failed. Shutting down." << endl;

        dreamland->shutdown( );
    }

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 8;
    }
};

extern "C"
{
    SO::PluginList initialize_olc( )
    {
        SO::PluginList ppl;
        
        OnlineCreation::registerPlugin(ppl);
        Plugin::registerPlugin<OLCInterpretLayer>( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateBehavior> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateAreaQuest> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateSkillGroup> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateCommand> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateSocial> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateReligion> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateRace> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateSkill> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateRoom> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateArea> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateHelp> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateMobile> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateObject> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateExtraExit> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<OLCStateFile> >( ppl );
        Plugin::registerPlugin<MocRegistrator<XMLVnumRange> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeOLC> >( ppl );
        Plugin::registerPlugin<SaveChangedAreasTask>( ppl );
        Plugin::registerPlugin<RoundTripSaveTask>( ppl );

        return ppl;
    }
}
