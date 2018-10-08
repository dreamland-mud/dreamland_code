/* $Id: valentineprise.h,v 1.1.2.4.22.1 2007/09/11 00:31:25 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef VALENTINEPRISE_H
#define VALENTINEPRISE_H

#include "xmlstring.h"

#include "personalquestreward.h"
#include "objectbehaviorplugin.h"

class Affect;

class ValentinePrise : public PersonalQuestReward {
XML_OBJECT
public:
        typedef ::Pointer<ValentinePrise> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void equip( Character * );                           

private:
        void addAffect( Affect *af, int level );
        XML_VARIABLE XMLString msgChar;
        XML_VARIABLE XMLString msgRoom;
};

#endif

