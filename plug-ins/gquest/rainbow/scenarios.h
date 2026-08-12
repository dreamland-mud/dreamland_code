/* $Id: scenarios.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef RAINBOWSCENARIOS_H
#define RAINBOWSCENARIOS_H

#include "xmlvariablecontainer.h"
#include "xmlvector.h"
#include "xmlstring.h"
#include "xmlmultistring.h"
#include "xmlinteger.h"
#include "plugin.h"

#include "gqexceptions.h"

class Room;
struct AreaIndexData;
class NPCharacter;
class PCharacter;
class Character;
class Object;
class MultiMessage;
class PCMemoryInterface;

/*---------------------------------------------------------------------------
 * RainbowScenario base class, and its registrator 
 *---------------------------------------------------------------------------*/
/* One rainbow piece, in every language.
 *
 * It used to be a bare XMLString, i.e. Russian and nothing else, so dressItem
 * could only ever fill an object's LANG_DEFAULT slot and the English and
 * Ukrainian short_descr of a quest piece stayed empty for good.
 *
 * A container wrapping an XMLMultiString, rather than an XMLMultiString put
 * straight into the vector: XMLVectorBase builds a NEW element for every <node>
 * child it meets, while XMLMultiString is written to have fromXML called three
 * times on ONE object (see the comment on XMLMultiString::fromXML). Swapping the
 * vector's element type would therefore have turned seven pieces into
 * twenty-one, each two thirds empty. Wrapped in a container, moc dispatches all
 * three <name l="..."> children onto the same member, which is the shape we
 * want. */
class PieceDescription : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<PieceDescription> Pointer;

    virtual void fromXML( const XMLNode::Pointer & );

    XML_VARIABLE XMLMultiString name;
};

class RainbowScenario : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<RainbowScenario> Pointer;
    typedef XMLVectorBase<PieceDescription> PieceDescriptions;

    virtual void canStart( ) const  = 0;
    virtual bool checkArea( AreaIndexData * ) const;
    virtual bool checkMobile( NPCharacter * ) const;
    virtual bool checkRoom( Room * ) const;

    inline const DLString& getInitMsg( ) const;
    inline const DLString& getDisplayName( ) const;
    inline const DLString& getStartMsg( ) const;
    inline const DLString& getRewardMsg( ) const;
    inline const DLString& getWinnerMsg( lang_t lang = LANG_DEFAULT ) const;
    inline const DLString& getNoWinnerMsg( ) const;
    inline const DLString& getInfoMsg( ) const;
    inline int  getInitTime( ) const;
    inline int getPieceVnum( ) const;
    inline int getPiecesCount( ) const;
    virtual void dressItem( Object*, int ) const = 0;

    virtual void printCount( int, ostringstream&, Character * ) const = 0;
    virtual void printTime( ostringstream&, Character * ) const = 0;
    /* Winner announcement seen by everyone else: the winner's name declines per
     * language, so the whole line is composed in all three at once. */
    virtual MultiMessage getWinnerMsgOther( PCMemoryInterface * ) const = 0;
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
    XML_VARIABLE XMLMultiString winnerMsg;
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
inline const DLString& RainbowScenario::getWinnerMsg( lang_t lang ) const
{
    return winnerMsg.getForLang( lang );
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

    virtual void canStart( ) const ;
    virtual bool checkRoom( Room * ) const;

    virtual void printCount( int, ostringstream&, Character * ) const;
    virtual void printTime( ostringstream&, Character * ) const;
    virtual MultiMessage getWinnerMsgOther( PCMemoryInterface * ) const;
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

    virtual void canStart( ) const ;
    virtual bool checkMobile( NPCharacter * ) const;

    virtual void printCount( int, ostringstream&, Character * ) const;
    virtual void printTime( ostringstream&, Character * ) const;
    virtual MultiMessage getWinnerMsgOther( PCMemoryInterface * ) const;
    virtual void onGivePiece( PCharacter *, NPCharacter * ) const;
    virtual void onQuestFinish( PCharacter * ) const;
    virtual bool canHearInitMsg( PCharacter * ) const;
    virtual void dressItem( Object*, int ) const;
};

#endif
