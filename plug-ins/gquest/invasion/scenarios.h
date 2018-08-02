/* $Id: scenarios.h,v 1.1.2.1.6.1 2008/02/22 09:26:02 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef INVASIONSCENARIOS_H
#define INVASIONSCENARIOS_H

#include "xmlvariablecontainer.h"
#include "xmlvector.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "plugin.h"

#include "objects.h"
#include "mobiles.h"

class Room;
struct area_data;

/*---------------------------------------------------------------------------
 * InvasionScenario 
 *---------------------------------------------------------------------------*/
class InvasionScenario : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<InvasionScenario> Pointer;
    typedef XMLVectorBase<XMLInteger> VnumList;
    
    virtual bool canStart( );
    virtual bool checkRoom( Room * );
    virtual bool checkArea( area_data * );
    virtual void collectRooms( vector<Room *> &, int ) = 0;

    inline const DLString& getStartMsg( );
    inline const DLString& getInfoMsg( );
    inline const DLString& getRewardMsg( );
    inline const DLString& getWinnerMsg( );
    inline const DLString& getWinnerMsgOther( );
    inline const DLString& getWinnerMsgOtherMlt( );
    inline const DLString& getNoWinnerMsg( );
    
    XML_VARIABLE VnumList vnumMobs;
    XML_VARIABLE VnumList vnumHelpers;
    XML_VARIABLE VnumList vnumObjs;
    XML_VARIABLE VnumList vnumInstruments;

protected:
    XML_VARIABLE XMLString startMsg;
    XML_VARIABLE XMLString infoMsg;
    XML_VARIABLE XMLString rewardMsg;
    XML_VARIABLE XMLString winnerMsg;
    XML_VARIABLE XMLString winnerMsgOther;
    XML_VARIABLE XMLString winnerMsgOtherMlt;
    XML_VARIABLE XMLString noWinnerMsg;
};

template<typename C>
class InvasionScenarioRegistrator: public Plugin {
public:
    typedef ::Pointer<InvasionScenarioRegistrator<C> > Pointer;
    virtual void initialization( )
    {
        Class::regMoc<C>( );
    }
    virtual void destruction( )
    {
        Class::unregMoc<C>( );
    }
    virtual const DLString& getName( ) const
    {
        return C::MOC_TYPE;
    }
};

/*---------------------------------------------------------------------------
 * inline get methods
 *---------------------------------------------------------------------------*/
inline const DLString& InvasionScenario::getStartMsg( )
{
    return startMsg.getValue( );
}
inline const DLString& InvasionScenario::getInfoMsg( )
{
    return infoMsg.getValue( );
}
inline const DLString& InvasionScenario::getRewardMsg( )
{
    return rewardMsg.getValue( );
}
inline const DLString& InvasionScenario::getWinnerMsg( )
{
    return winnerMsg.getValue( );
}
inline const DLString& InvasionScenario::getWinnerMsgOther( )
{
    return winnerMsgOther.getValue( );
}
inline const DLString& InvasionScenario::getWinnerMsgOtherMlt( )
{
    return winnerMsgOtherMlt.getValue( );
}
inline const DLString& InvasionScenario::getNoWinnerMsg( )
{
    return noWinnerMsg.getValue( );
}

/*---------------------------------------------------------------------------
 * base scatter algorithms 
 *---------------------------------------------------------------------------*/
class InvasionDenseScenario : public InvasionScenario {
XML_OBJECT
public:
    typedef ::Pointer<InvasionDenseScenario> Pointer;
    
    virtual void collectRooms( vector<Room *> &, int );
};

class InvasionSparseScenario : public InvasionScenario {
XML_OBJECT
public:
    typedef ::Pointer<InvasionSparseScenario> Pointer;
    
    virtual void collectRooms( vector<Room *> &, int );
};

/*---------------------------------------------------------------------------
 * scenarios implementation
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 * Locust 
 *---------------------------------------------------------------------------*/
class InvasionLocustScenario : public InvasionDenseScenario {
XML_OBJECT
public:
    typedef ::Pointer<InvasionLocustScenario> Pointer;
    virtual bool checkRoom( Room * );
};

/*---------------------------------------------------------------------------
 * Bubbles 
 *---------------------------------------------------------------------------*/
class InvasionBubblesMob : public InvasionMob {
XML_OBJECT    
public:    
    typedef ::Pointer<InvasionBubblesMob> Pointer;
protected:    
    virtual void actDeath( Character * );
};


/*---------------------------------------------------------------------------
 * Football
 *---------------------------------------------------------------------------*/
class InvasionFootballScenario : public InvasionSparseScenario {
XML_OBJECT
public:
    typedef ::Pointer<InvasionFootballScenario> Pointer;
    virtual bool checkRoom( Room * );
};

#endif
