#ifndef XMLATTRIBUTEAREAQUEST_H
#define XMLATTRIBUTEAREAQUEST_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "playerattributes.h"

/** Info about each area quest, as kept in player's attribute. Contains success counts and current quest info. */
class AreaQuestData : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<AreaQuestData> Pointer;

    AreaQuestData() : step(-1) { }
    virtual ~AreaQuestData();

	XML_VARIABLE XMLInteger step;
	XML_VARIABLE XMLIntegerNoEmpty timestart;
	XML_VARIABLE XMLIntegerNoEmpty timeend;
    XML_VARIABLE XMLIntegerNoEmpty timecancel;
	XML_VARIABLE XMLIntegerNoEmpty thisLife;
    XML_VARIABLE XMLIntegerNoEmpty total;

    Scripting::Register toRegister() const;

    void handleRemort();
    
	bool questActive() const;
    bool stepActive(int s) const;
	void start();
	void complete();
    void cancel();
	void advance();
    void rollback();
};

/** A permanent attribute (persisted after remort) that keeps track of all area quests ever done by this player,
 * as a map from quest ID to corresponding quest data.
 */
class XMLAttributeAreaQuest : public RemortAttribute, public XMLMapContainer<AreaQuestData> {
XML_OBJECT    
public:
    typedef ::Pointer<XMLAttributeAreaQuest> Pointer;

    virtual Scripting::Register toRegister() const;
    virtual bool handle(const RemortArguments &);
    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};



#endif