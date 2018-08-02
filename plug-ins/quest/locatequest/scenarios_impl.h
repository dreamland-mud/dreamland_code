/* $Id$
 *
 * ruffina, 2004
 */
#ifndef LOCATESCENARIOS_IMPL_H
#define LOCATESCENARIOS_IMPL_H

#include "scenarios.h"

/*
 * LocateMousesScenario
 */
class LocateMousesScenario : public LocateRadialAlgo, public LocateScenario {
XML_OBJECT    
public:
    virtual bool applicable( PCharacter * );
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & );
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> );
};

/*
 * LocateSecretaryScenario
 */
class LocateSecretaryScenario : public LocateRadialAlgo, public LocateScenario {
XML_OBJECT    
public:
    virtual bool applicable( PCharacter * );
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & );
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> );
};

/*
 * LocateAlchemistScenario
 */
class LocateAlchemistScenario: public LocateRadialAlgo, public LocateScenario {
XML_OBJECT    
public:
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & );
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> );
};

/*
 * LocateTorturerScenario
 */
class LocateTorturerScenario: public LocateUniformAlgo, public LocateScenario {
XML_OBJECT    
public:
    virtual bool applicable( PCharacter * );
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & );
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> );
};

#endif
