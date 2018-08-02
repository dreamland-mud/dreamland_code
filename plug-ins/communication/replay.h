/* $Id$
 *
 * ruffina, 2004
 */
#ifndef REPLAY_H
#define REPLAY_H

#include <sstream>

using namespace std;

class PCharacter;
class DLString;

void remember_history_public( PCharacter *ch, const DLString &msg );
void remember_history_private( PCharacter *ch, const DLString &msg );
void remember_history_near( PCharacter *ch, const DLString &msg );

bool replay_history_all( ostringstream &buf, PCharacter *ch, int limit );
bool replay_history_public( ostringstream &buf, PCharacter *ch, int limit );
bool replay_history_private( ostringstream &buf, PCharacter *ch, int limit );
bool replay_history_near( ostringstream &buf, PCharacter *ch, int limit );

extern const int MAX_HISTORY_SIZE;
extern const int DEFAULT_REPLAY_SIZE;

#endif
