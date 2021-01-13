#ifndef GQ_OBJECTS_H
#define GQ_OBJECTS_H

#include "xmlinteger.h"
#include "xmlstring.h"
#include "objectbehavior.h"

class GlobalQuest;

class GlobalQuestObject: public ObjectBehavior {
XML_OBJECT    
public:
        typedef ::Pointer<GlobalQuestObject> Pointer;
        
        virtual bool save( );

        /** Check if the quest this item belongs to is still active. */
        bool myQuestIsRunning() const;
        
        /** Remember which quest this item belongs to. */
        void setQuest(const GlobalQuest &gquest);

        bool hasQuest() const;
        
protected:
        XML_VARIABLE XMLString  questID;
        XML_VARIABLE XMLInteger questStartTime;
};

#endif