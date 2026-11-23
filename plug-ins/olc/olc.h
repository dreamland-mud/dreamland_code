// $Id: olc.h,v 1.1.2.14.6.4 2010-09-01 21:20:46 rufina Exp $

// The version info.  Please use this info when reporting bugs.
// It is displayed in the game by typing 'version' while editing.
// Do not remove these from the code - by request of Jason Dinkel
#ifndef __OLC_H__
#define __OLC_H__

#include <list>
#include <set>
#include "onlinecreation.h"
#include "descriptor.h"
#include "multimessage.h"
#include "xmlnode.h"

#define OLC_VERSION "ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r" \
                "     Port a ROM 2.4 v1.00\n\r"
#define OLC_AUTHOR  "     By Jason(jdinkelmines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbiifi.uio.no)\n\r" \
                "     Modificado para uso en ROM 2.4v4\n\r"     \
                "     Por Birdie (itoledoramses.centic.utem.cl)\n\r"
#define OLC_DATE    "     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r" \
                "     (Port a ROM 2.4 - Nov 2, 1996)\n\r"
#define OLC_CREDITS "     Original by Surreality(cxw197psu.edu) and Locke(lockelm.com)"

bool show_help(Character * ch, const char *argument);
int help_next_free_id();
DLString show_enum_array(const EnumerationArray &array);
DLString show_enum_array_web(const EnumerationArray &array);
void show_behaviors(PCharacter *ch, const GlobalBitvector &behaviors, const Json::Value &props);

#define stc(t, c) (c)->send_to((t))

const char * get_skill_name( int sn, bool verbose = true );
void ptc(Character *c, const char *fmt, ...);
/* Trilinguality (Trello 2594, Phase 4): resolve for the single viewer `c`. */
void ptc(Character *c, const MultiMessage &fmt, ...);

enum {
    OLC_NDX_ROOM,
    OLC_NDX_OBJ,
    OLC_NDX_MOB,
};

/** First free vnum of the given index type in pArea that ch can build in, or -1.
 *  Vnums in skip count as taken (a dry run's planned creates). */
int next_index_in_area( Character *ch, AreaIndexData *pArea, int ndx_type,
                        const std::set<int> *skip = 0 );

/** Resolve the argument of 'create <vnum>|next' for a new room, mob or object:
 *  a free vnum inside an area ch can build in, or -1 with the reason told to ch.
 *  'next' takes the first free vnum of nextArea. Every create path, in game and
 *  over the API, goes through here. */
int olc_create_vnum( Character *ch, AreaIndexData *nextArea, const char *argument, int ndx_type,
                     const std::set<int> *skip = 0 );

/** 'asave changed': write every changed area ch can build in. Returns the
 *  number of areas that failed to save, the saved ones go to 'saved'. */
int save_changed_areas( Character *ch, std::list<AreaIndexData *> &saved );

int next_obj_index( Character *ch, RoomIndexData *r );
int next_room( Character *ch, RoomIndexData *r );
int next_mob_index( Character *ch, RoomIndexData *r );

/** Two optional XML nodes are both absent or equal: a put leaves an
 *  unchanged behavior node alone, so its runtime state survives. */
bool olc_same_xml( const XMLNode::Pointer &a, const XMLNode::Pointer &b );

class XMLMultiString;

/** Case forms of a prototype short description, RU and UA (medit/oedit
 *  'cases'). Nothing declines a short on its own: the editor proposes the
 *  forms and the builder applies them with 'cases ok'. gender is the
 *  morphology sidecar's masc|femn|neut|plur, anim is true for mobiles. */
DLString olc_cases_gender_mob( int sex, bool plural );
DLString olc_cases_gender_obj( int multiGender );

/** The proposal for the RU and UA short that this edit run changed (it
 *  differs from 'before', the prototype's short when the run started; null
 *  for a new prototype) and that has Cyrillic and no '|'. A short nobody
 *  touched gets no line, however bare. Writes nothing; report gets one line
 *  per language and how to apply. True if anything was proposed. */
bool olc_cases_propose( const XMLMultiString &shortDescr, const XMLMultiString *before,
                        const DLString &gender, bool anim, DLString &report );

/** The 'cases' editor command: show the six cases (and the proposal for a
 *  short without them); 'cases ok' writes the forms of a short that has
 *  none, 'cases ok ru|ua' re-declines that language from its nominative
 *  even over existing forms. True if a value changed. */
bool olc_cases_command( PCharacter *ch, const char *argument, XMLMultiString &shortDescr,
                        const DLString &gender, bool anim );

class OLCState;
/** AI phase 1 (olcaigen.cpp): queue a request for the AI worker to write the
 *  empty text fields of a room, mob or obj prototype ("room"/"mob"/"obj").
 *  manual = 'gen' was typed: also extra descriptions and material, and never
 *  skipped by 'gen auto off'. Otherwise only an empty core text field queues
 *  one, and only while AI is on (.tmp.ai.enabled("olc_gen")). */
void olc_ai_gen( PCharacter *ch, const DLString &entity, int vnum, bool manual, const DLString &hint );
/** Called by every editor's 'done' after its commit: the request of a 'gen'
 *  typed in this editor, or the automatic one. */
void olc_ai_done( OLCState *state, PCharacter *ch, const DLString &entity, int vnum );
/** The editors' 'gen' subcommand: gen [hint] | gen auto on|off | gen revert [vnum]. */
bool olc_ai_gen_command( OLCState *state, PCharacter *ch, const DLString &entity, int vnum, const char *argument );

#endif /* __OLC_H__ */
