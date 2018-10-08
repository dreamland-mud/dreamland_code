/* $Id: questreward.h,v 1.1.2.4.22.1 2007/05/02 02:32:37 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef QUESTREWARD_H
#define QUESTREWARD_H

#include "objectbehavior.h"
#include "objectbehaviorplugin.h"

class QuestReward : public virtual ObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<QuestReward> Pointer;
        
        virtual bool mayFloat( );
        virtual bool canConfiscate( );
                        
};

#endif

