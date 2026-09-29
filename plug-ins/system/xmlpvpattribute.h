#ifndef XMLPVPATTRIBUTE_H
#define XMLPVPATTRIBUTE_H

#include <time.h>
#include "xmlattribute.h"
#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmllong.h"
#include "xmlmap.h"
#include "playerattributes.h"

/** Counted kills of one victim and when the last one was counted. */
class XMLPvpVictim : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger count;
    XML_VARIABLE XMLLong last;
};

/**
 * Player-kill statistics used by clan entry rules. Only kills that pass the
 * anti-farm filters (see fight/pvp.h) get here. Survives remort.
 */
class XMLPvpAttribute : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLPvpAttribute> Pointer;
    typedef XMLMapBase<XMLPvpVictim> Victims;

    /** The same victim counts at most once per this many seconds. */
    static const time_t REPEAT_WINDOW;

    /** Count a kill of this victim unless one was counted within REPEAT_WINDOW. */
    bool record(const DLString &victimName, time_t when);

    virtual Scripting::Register toRegister() const;

    XML_VARIABLE XMLInteger kills;
    XML_VARIABLE Victims victims;
};

#endif
