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
    // The client is needed for its display language: short_descr is snapshotted
    // here and rendered verbatim later, so it has to be captured already
    // resolved for the reader who asked for the list.
    MixedEntry( Object *, int, Character * );
    
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
