/* $Id: prince.h,v 1.1.2.7.22.2 2008/03/06 17:48:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef KIDNAPPRINCE_H
#define KIDNAPPRINCE_H

#include "xmlinteger.h"
#include "kidnapmobile.h"

class PCMemoryInterface;

enum {
    STAT_INIT,
    STAT_FOLLOW,  
    STAT_LOST,    
    STAT_KIDNAPPED,
    STAT_REUNION,
};

class KidnapPrince : public KidnapMobile, 
                     public MandatoryMobile,
		     public ProtectedClient,
		     public ConfiguredMobile
{
XML_OBJECT
public:
    typedef ::Pointer<KidnapPrince> Pointer;

    KidnapPrince( );
    
    virtual void entry(  );
    virtual void greet( Character *victim );
    virtual void give( Character *victim, Object *obj );
    virtual bool spec( );

    XML_VARIABLE XMLInteger state;

protected:
    virtual void config( PCharacter * );
    virtual void deadAction( QuestPointer, PCMemoryInterface *, Character * );

private:
    void heroAttach( PCharacter * );
    void heroDetach( PCharacter * );
    void heroReattach( PCharacter * );
    
    void kingReunion( NPCharacter * );
    
    NPCharacter * getBanditRoom( );
    void banditsUnleash( PCharacter * );
};



#endif

