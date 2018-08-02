/* $Id: scenarios.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef RAINBOWSCENARIOS_H
#define RAINBOWSCENARIOS_H

#include "xmlvariablecontainer.h"
#include "xmlvector.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "plugin.h"

#include "gqexceptions.h"

class Room;
struct area_data;
class NPCharacter;
class PCharacter;
class Object;

/*---------------------------------------------------------------------------
 * RainbowScenario base class, and its registrator 
 *---------------------------------------------------------------------------*/
class RainbowScenario : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<RainbowScenario> Pointer;
    typedef XMLVectorBase<XMLString> PieceDescriptions;

    virtual void canStart( ) const throw ( GQCannotStartException ) = 0;
    virtual bool checkArea( area_data * ) const;
    virtual bool checkMobile( NPCharacter * ) const;
    virtual bool checkRoom( Room * ) const;

    inline const DLString& getInitMsg( ) const;
    inline const DLString& getDisplayName( ) const;
    inline const DLString& getStartMsg( ) const;
    inline const DLString& getRewardMsg( ) const;
    inline const DLString& getWinnerMsg( ) const;
    inline const DLString& getNoWinnerMsg( ) const;
    inline const DLString& getInfoMsg( ) const;
    inline int  getInitTime( ) const;
    inline int getPieceVnum( ) const;
    inline int getPiecesCount( ) const;
    virtual void dressItem( Object*, int ) const = 0;

    virtual void printCount( int, ostringstream& ) const = 0;
    virtual void printTime( ostringstream& ) const = 0;
    virtual void printWinnerMsgOther( const DLString &, ostringstream& ) const = 0;
    virtual void onGivePiece( PCharacter *, NPCharacter * ) const = 0;
    virtual void onQuestInit( ) const;
    virtual void onQuestFinish( PCharacter * ) const = 0;
    virtual bool canHearInitMsg( PCharacter * ) const = 0;

protected:    
    XML_VARIABLE PieceDescriptions pieces;
    XML_VARIABLE XMLString initMsg;
    XML_VARIABLE XMLString displayName;
    XML_VARIABLE XMLString startMsg;
    XML_VARIABLE XMLString infoMsg;
    XML_VARIABLE XMLString winnerMsg;
    XML_VARIABLE XMLString noWinnerMsg;
    XML_VARIABLE XMLInteger initTime;
    XML_VARIABLE XMLInteger pieceVnum;
};

template<typename C>
class RainbowScenarioRegistrator: public Plugin {
public:
    typedef ::Pointer<RainbowScenarioRegistrator<C> > Pointer;
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
inline const DLString& RainbowScenario::getInitMsg( ) const
{
    return initMsg.getValue( );
}
inline const DLString& RainbowScenario::getDisplayName( ) const
{
    return displayName;
}
inline const DLString& RainbowScenario::getStartMsg( ) const
{
    return startMsg.getValue( );
}
inline const DLString& RainbowScenario::getInfoMsg( ) const
{
    return infoMsg.getValue( );
}
inline const DLString& RainbowScenario::getNoWinnerMsg( ) const
{
    return noWinnerMsg.getValue( );
}
inline const DLString& RainbowScenario::getWinnerMsg( ) const
{
    return winnerMsg.getValue( );
}
inline int RainbowScenario::getInitTime( ) const
{
    return initTime.getValue( );
}
inline int RainbowScenario::getPieceVnum( ) const
{
    return pieceVnum.getValue( );
}
inline int RainbowScenario::getPiecesCount( ) const
{
    return pieces.size( );
}

/*---------------------------------------------------------------------------
 * default scenario 
 *---------------------------------------------------------------------------*/
class RainbowDefaultScenario : public RainbowScenario {
XML_OBJECT
public:
    typedef ::Pointer<RainbowDefaultScenario> Pointer;

    virtual void canStart( ) const throw ( GQCannotStartException );
    virtual bool checkRoom( Room * ) const;

    virtual void printCount( int, ostringstream& ) const;
    virtual void printTime( ostringstream& ) const;
    virtual void printWinnerMsgOther( const DLString &, ostringstream& ) const;
    virtual void onGivePiece( PCharacter *, NPCharacter * ) const;
    virtual void onQuestFinish( PCharacter * ) const;
    virtual bool canHearInitMsg( PCharacter * ) const;
    virtual void dressItem( Object*, int ) const;
};


/*---------------------------------------------------------------------------
 * Seven Deadly Sins scenario 
 *---------------------------------------------------------------------------*/
class RainbowSinsScenario : public RainbowScenario {
XML_OBJECT
public:
    typedef ::Pointer<RainbowSinsScenario> Pointer;

    virtual void canStart( ) const throw ( GQCannotStartException );
    virtual bool checkMobile( NPCharacter * ) const;

    virtual void printCount( int, ostringstream& ) const;
    virtual void printTime( ostringstream& ) const;
    virtual void printWinnerMsgOther( const DLString &, ostringstream& ) const;
    virtual void onGivePiece( PCharacter *, NPCharacter * ) const;
    virtual void onQuestFinish( PCharacter * ) const;
    virtual bool canHearInitMsg( PCharacter * ) const;
    virtual void dressItem( Object*, int ) const;
};

#endif
