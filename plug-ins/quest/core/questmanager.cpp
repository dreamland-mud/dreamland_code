/* $Id: questmanager.cpp,v 1.1.4.4.6.1 2007/06/26 07:20:02 rufina Exp $
 *
 * ruffina, 2003
 */

#include <sstream>

#include "dbio.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "dreamland.h"

#include "questmanager.h"
#include "questexceptions.h"
#include "quest.h"
#include "questregistrator.h"

#include "mercdb.h"

using namespace std;

QuestManager* QuestManager::thisClass = NULL;
const DLString QuestManager::TABLE_NAME = "quests";
const DLString QuestManager::NODE_NAME = "Quest";

QuestManager::QuestManager( ) {
    thisClass = this;
}

QuestManager::~QuestManager( ) {
    thisClass = NULL;
}

void QuestManager::initialization( ) {
}

void QuestManager::destruction( ) {
}

DLString QuestManager::getNodeName( ) const
{
    return NODE_NAME;
}
DLString QuestManager::getTableName( ) const
{
    return TABLE_NAME;
}


static bool compare(QuestRegistratorBase::Pointer a, QuestRegistratorBase::Pointer b)
{
    return a->getPriority() > b->getPriority();
}

QuestList QuestManager::list(PCharacter *pch) const
{
    unsigned int i;
    QuestList result;

    for (i = 0; i < quests.size(); i++) {
        if (quests[i]->applicable( pch, false )) 
            result.push_back(quests[i]);
    }
    
    result.sort(compare);
    return result; 
    
}

void QuestManager::generate( PCharacter *pch, NPCharacter *questor ) const {
    unsigned int summ, i, dice;
    QuestList qlist;
    
    for (summ = 0, i = 0; i < quests.size( ); i++) {
        if (quests[i]->applicable( pch, true )) {
            summ += quests[i]->getPriority( );
            qlist.push_back( quests[i] );
        }
    }

    while (!qlist.empty( )) {
        QuestList::iterator ipos;

        dice = number_range( 0, summ - 1 );
        for (i = 0, ipos = qlist.begin( ); ipos != qlist.end( ); ipos++) {
            i += (*ipos)->getPriority( );

            if (i > dice) 
                break;
        }
        
        try {
            pch->getAttributes( ).addAttribute( 
                         (*ipos)->createQuest( pch, questor ), "quest" );
            return; 
        } 
        catch (const QuestCannotStartException &e) {
            summ -= (*ipos)->getPriority( );
            qlist.erase( ipos );

        } 
        catch (const Exception &e1) {
            LogStream::sendError( ) << e1.what( ) << endl;
            throw QuestCannotStartException( );
        }
    }
    
    throw QuestCannotStartException( );
}

void QuestManager::load( QuestRegistratorBase* reg ) {
    if (loadXML( reg, reg->getName( ) )) 
        quests.push_back( reg );
}

void QuestManager::unLoad( QuestRegistratorBase* reg ) {
//    saveXML( reg, reg->getName( ) );
    
    for (QuestRegistry::iterator i = quests.begin( ); i != quests.end( ); i++)
        if (**i == reg) {
            quests.erase( i );
            break;
        }
}

QuestRegistratorBase::Pointer
QuestManager::findQuestRegistrator( const DLString& carg )
{
    unsigned int i;
    DLString name, arg;

    arg = carg;
    arg.toLower( );
    
    for (i = 0; i < quests.size( ); i++) {
        name = quests[i]->getName( );
        name.toLower( );

        if (name == arg)
            return quests[i];
    }
    
    return QuestRegistratorBase::Pointer( );
}
