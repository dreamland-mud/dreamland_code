/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __GROUP_BEGUILING_H__
#define __GROUP_BEGUILING_H__

#include "objectbehaviormanager.h"

class MagicJar : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<MagicJar> Pointer;

    virtual void get( Character * );
    virtual bool extract( bool );
    virtual bool quit( Character *, bool );
    virtual bool area( );
};


#endif
