/* $Id: mobiles.h,v 1.1.2.5.6.1 2007/09/11 00:33:54 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CARDMOBILES_H
#define CARDMOBILES_H

#include "basicmobilebehavior.h"
#include "mobilebehaviorplugin.h"

class CardStarterBehavior : public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<CardStarterBehavior> Pointer;
    
        CardStarterBehavior( );

        virtual bool death( Character *killer );
        
};

class CardSellerBehavior : public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<CardSellerBehavior> Pointer;
    
        CardSellerBehavior( );

        virtual void greet( Character *victim );
        virtual void speech( Character *victim, const char *speech );
        virtual int  getOccupation( );
};

#endif

