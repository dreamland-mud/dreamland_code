/* $Id: xmlattributequestreward.h,v 1.1.4.1 2005/07/30 14:50:09 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTEQUESTREWARD_H
#define XMLATTRIBUTEQUESTREWARD_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmlattribute.h"

class XMLAttributeQuestReward : public XMLAttribute, 
                                public XMLMapBase<XMLInteger>
{
public: 
        typedef ::Pointer<XMLAttributeQuestReward> Pointer;

        static const DLString TYPE;

        virtual const DLString & getType( ) const
        {
            return TYPE;
        }

        int getCount( int ) const;
        void setCount( int, int );

        // Paid upgrade tier per reward vnum, so a lost item re-created by
        // trouble() can be restamped to the tier the owner already paid for.
        // Stored in the same map under a "tier:<vnum>" key, which never
        // collides with the plain "<vnum>" count keys getCount/setCount use.
        int getTier( int ) const;
        void setTier( int, int );
};

#endif

