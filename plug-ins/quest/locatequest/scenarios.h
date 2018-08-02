/* $Id: scenarios.h,v 1.1.2.7.6.1 2007/09/29 19:34:06 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef LOCATEQUEST_SCENARIOS_H
#define LOCATEQUEST_SCENARIOS_H

#include "algorithms.h"
#include "questscenario.h"

class Object;
class PCharacter;
class NPCharacter;
class Character;
class Room;
class LocateQuest;

class LSItemData : public QuestItemAppearence {
XML_OBJECT
public:
    typedef ::Pointer<LSItemData> Pointer;

    XML_VARIABLE XMLString shortMlt;
};

class LocateScenario: public QuestScenario, 
                      public virtual XMLVariableContainer,
                      public virtual LocateAlgo 
{
XML_OBJECT
public:
    typedef VnumList Customers;
    typedef XMLVectorBase<LSItemData> Items;
    
    virtual bool applicable( PCharacter * );
    virtual int getCount( PCharacter * );

    virtual void getLegend( PCharacter *, ::Pointer<LocateQuest>, ostream & ) = 0;
    virtual void actTellStory( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> ) = 0;

    virtual void actWrongItem( NPCharacter *, PCharacter *, ::Pointer<LocateQuest>, Object * );
    virtual void actLastItem( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> );
    virtual void actAnotherItem( NPCharacter *, PCharacter *, ::Pointer<LocateQuest> );

    XML_VARIABLE Customers customers;
    XML_VARIABLE Items items;
};

#endif
