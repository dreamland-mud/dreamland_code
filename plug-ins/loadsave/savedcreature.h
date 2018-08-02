/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SAVEDCREATURE_H__
#define __SAVEDCREATURE_H__

#include "xmlboolean.h"
#include "mobilebehavior.h"

class SavedCreature : public virtual MobileBehavior {
XML_OBJECT
public:
    typedef ::Pointer<SavedCreature> Pointer;
    
    virtual void save( );
    virtual bool extract( bool );
    virtual void stopfol( Character * );

protected:
    XML_VARIABLE XMLBoolean saved;
};

#endif
