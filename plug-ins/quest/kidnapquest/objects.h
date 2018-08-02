/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __KIDNAP_OBJECTS_H__
#define __KIDNAP_OBJECTS_H__

#include "objquestbehavior.h"

class KidnapMark : public ObjQuestBehavior {
XML_OBJECT
public:
    typedef ::Pointer<KidnapMark> Pointer;

    virtual ~KidnapMark( );
};

#endif
