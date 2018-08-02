/* $Id: mobiles.h,v 1.1.2.2 2005/11/26 16:59:51 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef RAINBOW_MOBILE_H
#define RAINBOW_MOBILE_H

#include "xmlinteger.h"
#include "basicmobilebehavior.h"
#include "mobilebehaviorplugin.h"

class RainbowGQuest;

class RainbowMob : public BasicMobileDestiny {
XML_OBJECT    
public:    
    typedef ::Pointer<RainbowMob> Pointer;
    
    RainbowMob( );
    virtual ~RainbowMob( );

    virtual void setChar( NPCharacter * );
    virtual void unsetChar( );

    virtual void entry( );
    virtual void greet( Character * );
    virtual bool death( Character * );

    XML_VARIABLE XMLInteger number;

protected:
    virtual bool specIdle( );

private:
    bool hasMyNumber( Character * );

};

#endif
