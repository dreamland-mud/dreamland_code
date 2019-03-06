/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __QUEST_ENTITY_H__
#define __QUEST_ENTITY_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "pcharacter.h"
#include "quest.h"

class Character;
class PCharacter;
class PCMemoryInterface;
class Quest;

class QuestEntity : public virtual DLObject {
public:
    typedef ::Pointer<QuestEntity> Pointer;
    typedef ::Pointer<Quest> QuestPointer;

    inline const DLString & getHeroName( ) const;
    inline void setHeroName( const DLString & );
    virtual bool ourHero( Character * ) const;
    virtual bool ourMobile( NPCharacter * ) const;
    virtual bool ourObject( Object * ) const;
    virtual bool ourHeroGroup( Character * ) const;
    
protected:
    void mandatoryExtract( );
    PCMemoryInterface * getHeroMemory( ) const;
    PCharacter * getHeroWorld( ) const;
    
    QuestPointer getQuest( ) const;
    QuestPointer getQuest( PCMemoryInterface * ) const;
    template <typename T> inline ::Pointer<T> getMyQuest( ) const;
    template <typename T> inline ::Pointer<T> getMyQuest( PCMemoryInterface * ) const;
    
    XML_VARIABLE XMLString heroName;
};

inline const DLString & QuestEntity::getHeroName( ) const
{
    return heroName.getValue( );
}

inline void QuestEntity::setHeroName( const DLString &name )
{
    heroName.setValue( name );
}

template <typename T> 
inline ::Pointer<T> QuestEntity::getMyQuest( ) const
{
    return getMyQuest<T>( getHeroMemory( ) );
}

template <typename T> 
inline ::Pointer<T> QuestEntity::getMyQuest( PCMemoryInterface *pci ) const
{
    Quest::Pointer quest;
    ::Pointer<T> myQuest, null;

    if (pci
        && ( quest = getQuest( pci ) )
        && ( myQuest = dynamic_cast<T *>( quest.getPointer( ) ) )
        && quest->charName == heroName)
        return myQuest;
    else
        return null;
}

#endif

