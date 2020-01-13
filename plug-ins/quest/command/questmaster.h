/* $Id: questmaster.h,v 1.1.2.3.6.3 2009/02/07 17:05:55 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef QUESTMASTER_H
#define QUESTMASTER_H

#include "questtrader.h"
#include "questor.h"

class QuestMaster : public QuestTrader, public Questor {
XML_OBJECT
public:
        typedef ::Pointer<QuestMaster> Pointer;

        QuestMaster( );
        
        virtual int getOccupation( );
        virtual bool canGiveQuest( Character * );
        virtual void speech( Character *victim, const char *speech );
        virtual void tell( Character *victim, const char *speech );

protected:
        virtual bool specIdle( );
};

class DefaultQuestMaster : public QuestMaster {
XML_OBJECT
public:
        typedef ::Pointer<DefaultQuestMaster> Pointer;
        
        virtual ~DefaultQuestMaster();
};

#endif

