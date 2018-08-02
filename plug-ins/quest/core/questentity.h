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
    virtual bool ourHero( Character * );
    virtual bool ourMobile( NPCharacter * );
    virtual bool ourObject( Object * );
    
protected:
    void mandatoryExtract( );
    PCMemoryInterface * getHeroMemory( );
    PCharacter * getHeroWorld( );
    
    QuestPointer getQuest( );
    QuestPointer getQuest( PCMemoryInterface * );
    template <typename T> inline ::Pointer<T> getMyQuest( );
    template <typename T> inline ::Pointer<T> getMyQuest( PCMemoryInterface * );
    
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
inline ::Pointer<T> QuestEntity::getMyQuest( )
{
    return getMyQuest<T>( getHeroMemory( ) );
}

template <typename T> 
inline ::Pointer<T> QuestEntity::getMyQuest( PCMemoryInterface *pci )
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

