/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PETSHOPROOM_H__
#define __PETSHOPROOM_H__

#include "xmlinteger.h"
#include "roombehavior.h"

class PetShopStorage;

class PetShopRoom : public virtual RoomBehavior {
XML_OBJECT
public:
    typedef ::Pointer<PetShopRoom> Pointer;

    PetShopRoom( );

    virtual bool command( Character *, const DLString &, const DLString & );

protected:
    ::Pointer<PetShopStorage> getStorage( );

    XML_VARIABLE XMLInteger storageVnum;
};


#endif
