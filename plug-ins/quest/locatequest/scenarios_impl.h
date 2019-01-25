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
    virtual bool applicable( PCharacter * ) const;
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & ) const;
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> ) const;
};

/*
 * LocateSecretaryScenario
 */
class LocateSecretaryScenario : public LocateRadialAlgo, public LocateScenario {
XML_OBJECT    
public:
    virtual bool applicable( PCharacter * ) const;
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & ) const;
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> ) const;
};

/*
 * LocateAlchemistScenario
 */
class LocateAlchemistScenario: public LocateRadialAlgo, public LocateScenario {
XML_OBJECT    
public:
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & ) const;
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> ) const;
};

/*
 * LocateTorturerScenario
 */
class LocateTorturerScenario: public LocateUniformAlgo, public LocateScenario {
XML_OBJECT    
public:
    virtual bool applicable( PCharacter * ) const;
    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & ) const;
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> ) const;
};

#endif
