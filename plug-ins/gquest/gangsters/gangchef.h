/* $Id: gangchef.h,v 1.1.2.1.6.1 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef GANGCHEF_H
#define GANGCHEF_H

#include "gangmob.h"

class GangChef : public GangMob {
XML_OBJECT    
public:    
    typedef ::Pointer<GangChef> Pointer;

    virtual bool spec( );
    virtual void fight( Character * );
    virtual bool death( Character * );
    virtual void greet( Character * );

protected:
    void createBounty(Character *killer);
};

#endif
