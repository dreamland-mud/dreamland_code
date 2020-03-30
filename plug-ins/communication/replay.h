/* $Id$
 *
 * ruffina, 2004
 */
#ifndef REPLAY_H
#define REPLAY_H

#include <sstream>
#include "playerattributes.h"
#include "xmllist.h"
#include "xmlstring.h"
#include "xmllonglong.h"

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

class ReplayAttribute : public EventHandler<StopFightArguments>, 
                        public EventHandler<AfkArguments>,
                        public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ReplayAttribute> Pointer;

    virtual bool handle( const StopFightArguments &args );
    virtual bool handle( const AfkArguments &args );

    void addMessage(const DLString &msg);
    static bool playAndErase(ostringstream &buf, PCharacter *ch);

    XML_VARIABLE XMLListBase<XMLString> tells;
    XML_VARIABLE XMLLongLong lastNotified;

protected:
    void notify(PCharacter *ch) const;
};


#endif
