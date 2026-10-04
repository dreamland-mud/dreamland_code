#ifndef QUESTSCROLLHOOK_H
#define QUESTSCROLLHOOK_H

class Object;
class PCharacter;

/**
 * Questor's skill scroll for code that can't link quest_command (Fenia root).
 * quest_command sets the hook on load and must reset it to NULL on unload.
 */
typedef Object * (*QuestScrollHook)( PCharacter * );
extern QuestScrollHook quest_scroll_hook;

/** A new scroll bound to ch, not placed anywhere; NULL if no skill qualifies or quest_command is unloaded. */
Object * quest_scroll_create( PCharacter *ch );

#endif
