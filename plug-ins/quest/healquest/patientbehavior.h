/* $Id: patientbehavior.h,v 1.1.2.7.22.4 2008/03/06 17:48:32 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef PATIENTBEHAVIOR_H
#define PATIENTBEHAVIOR_H

#include "mobquestbehavior.h"

class HealQuest;

class PatientBehavior : public MandatoryMobile, 
                        public ProtectedClient,
                        public PeacefulClient,
                        public DedicatedMobile<HealQuest>
{
XML_OBJECT
public:
    typedef ::Pointer<PatientBehavior> Pointer;
    
    virtual bool spell( Character *caster, int sn, bool before );
    virtual bool canCancel( Character * );

protected:
    virtual void deadFromIdiot( PCMemoryInterface * );
    virtual void deadFromSuicide( PCMemoryInterface * );
    virtual void deadFromKill( PCMemoryInterface *, Character * );

    void healInfect( PCharacter * );
    void healSuccess( PCharacter * );
    void healComplete( PCharacter * );
    void healOtherSuccess( PCMemoryInterface * );
    void healOtherComplete( PCMemoryInterface * );
};

#endif

