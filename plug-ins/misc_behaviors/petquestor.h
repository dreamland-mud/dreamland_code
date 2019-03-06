/* $Id$
 *
 * ruffina, 2004
 */
#ifndef PETQUESTOR_H 
#define PETQUESTOR_H 

#include "pet.h"
#include "questor.h"

class PetQuestor : public Questor, public Pet {
XML_OBJECT
public:
        typedef ::Pointer<PetQuestor> Pointer;
        
        PetQuestor( );

        virtual int getOccupation( );

        virtual void stopfol( Character *master );
        virtual bool area( );

protected:
        virtual bool specIdle( );

        XML_VARIABLE XMLStringNoEmpty msgIdleMaster;
        XML_VARIABLE XMLStringNoEmpty msgIdleOther;
        XML_VARIABLE XMLStringNoEmpty msgIdleRoom;
        XML_VARIABLE XMLStringNoEmpty msgDisappear;
        XML_VARIABLE XMLInteger timer;
};

#endif

