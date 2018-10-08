/* $Id: kidnapquestregistrator.h,v 1.1.2.4.10.1 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef KIDNAPQUESTREGISTRATOR_H
#define KIDNAPQUESTREGISTRATOR_H

#include "scenario.h"
#include "kidnapquest.h"
#include "questregistrator.h"

class KidnapQuestRegistrator : public QuestRegistrator<KidnapQuest>,
                               public QuestScenariosContainer
{
XML_OBJECT
public:
    KidnapQuestRegistrator( ); 
    virtual ~KidnapQuestRegistrator( );
    
    static inline KidnapQuestRegistrator * getThis( ) {
        return thisClass;
    }

    XML_VARIABLE XMLInteger markVnum;
    XML_VARIABLE XMLInteger banditVnum;
    XML_VARIABLE XMLInteger princeVnum;

private:
    static KidnapQuestRegistrator * thisClass;
};

#endif
