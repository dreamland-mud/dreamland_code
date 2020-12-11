/* $Id$
 *
 * ruffina, 2004
 */
#ifndef FOLLOW_UTILS_H
#define FOLLOW_UTILS_H

#include <set>

using std::set;
class Character;
class PCharacter;
typedef set<Character *> GroupMembers;
class Room;

void follower_die( Character * );
void follower_add( Character *, Character * );
void follower_stop( Character *, bool verbose = true );
void follower_clear( Character * ch, bool verbose = true );

bool is_same_group( Character *, Character * );
Character *  follower_find( Character *, const char * );
GroupMembers group_members_room( Character *ch, Room *room = 0 );
GroupMembers group_members_world( Character *ch );
GroupMembers party_members_room( Character *ch, Room *room = 0 );
GroupMembers party_members_world( Character *ch );

void guarding_assert( Character * );
void guarding_nuke( Character *, Character * );
void guarding_stop( PCharacter *, PCharacter * );
void guarding_clear( Character * );

#endif
