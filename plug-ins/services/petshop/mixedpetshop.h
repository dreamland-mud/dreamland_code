/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MIXEDPETSHOP_H__
#define __MIXEDPETSHOP_H__

#include <list>
#include "petshopstorage.h"
#include "petshoproom.h"

struct MixedEntry {
    MixedEntry( );
    MixedEntry( Pet::Pointer, Character * );
    MixedEntry( Object *, int );
    
    int level;
    int cost;
    DLString short_descr;
    DLString name;
    bool pet;
    int pos;
};
typedef list<MixedEntry> MixedList;

class MixedPetShopRoom : public PetShopRoom {
XML_OBJECT
public:
    typedef ::Pointer<MixedPetShopRoom> Pointer;

    virtual bool command( Character *, const DLString &, const DLString & );

protected:
    void doList( Character * );
    void doBuy( Character *, const DLString & );

    void createMixedList( MixedList &, Character * );
    bool lookupMixedList( MixedList &, MixedEntry &, Character *, DLString & );
};



#endif
