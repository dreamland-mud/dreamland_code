/* $Id: questregistrator.h,v 1.1.4.5.6.1 2007/09/29 19:33:59 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef QUESTREGISTRATOR_H
#define QUESTREGISTRATOR_H

#include "class.h"
#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlattributeplugin.h"

#include "quest.h"
#include "questmanager.h"

class NPCharacter;
class PCharacter;

class QuestRegistratorBase : public XMLAttributePlugin, public virtual XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<QuestRegistratorBase> Pointer;

    virtual Quest::Pointer createQuest( PCharacter *, NPCharacter * ) const = 0;
    virtual const DLString& getName( ) const = 0;

    virtual bool applicable( PCharacter *, bool fAuto ) const;
    virtual int getPriority( ) const;
    const DLString& getShortDescr( ) const;
    const DLString& getDifficulty( ) const;
    
protected:
    XML_VARIABLE XMLString shortDesc;
    XML_VARIABLE XMLString difficulty;
    XML_VARIABLE XMLInteger priority;
    XML_VARIABLE XMLIntegerNoEmpty minAutoLevel;
};

template<typename C>
class QuestRegistrator : public QuestRegistratorBase {
public:
    typedef ::Pointer< QuestRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
        Class::regMoc<C>( );
        QuestManager::getThis( )->load( this );
        XMLAttributePlugin::initialization( );
    }
    
    virtual void destruction( ) 
    {
        XMLAttributePlugin::destruction( );
        QuestManager::getThis( )->unLoad( this );
        Class::unregMoc<C>( );
    }

    virtual Quest::Pointer createQuest( PCharacter *pch, NPCharacter *questor ) const
    {
        ::Pointer<C> quest( NEW );
        quest->create( pch, questor );
        return quest;
    }

    virtual const DLString& getName( ) const 
    {
        return C::MOC_TYPE;
    }
};

#endif
