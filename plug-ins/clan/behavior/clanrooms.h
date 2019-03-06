/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLANROOMS_H__
#define __CLANROOMS_H__

#include "petshopstorage.h"
#include "clanobjects.h"

class ClanRoom : public virtual RoomBehavior {
XML_OBJECT
public:
        typedef ::Pointer<ClanRoom> Pointer;

protected:        
        ClanArea::Pointer getClanArea( );
};

class ClanPetShopStorage : public ClanRoom, public PetShopStorage {
XML_OBJECT
public:
        typedef ::Pointer<ClanPetShopStorage> Pointer;
        
protected:
        virtual bool canServeClient( Character * );
};

#endif
