/* $Id: personalquestreward.h,v 1.1.2.6.22.1 2007/09/11 00:31:25 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef PERSONALQUESTREWARD_H
#define PERSONALQUESTREWARD_H

#include "objectbehavior.h"
#include "objectbehaviorplugin.h"
#include "questreward.h"

class PersonalQuestReward : public QuestReward {
XML_OBJECT
public:
        typedef ::Pointer<PersonalQuestReward> Pointer;
        
        virtual void get( Character * );
        virtual bool save( );
        virtual void delete_( Character * ); 
        virtual bool isLevelAdaptive( ); 
        virtual bool canSteal( Character * );
        virtual bool canEquip( Character * );
};

#endif

