/* $Id: questmanager.cpp,v 1.1.4.4.6.1 2007/06/26 07:20:02 rufina Exp $
 *
 * ruffina, 2003
 */

#include <sstream>

#include "dbio.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "dreamland.h"
#include "wiznet.h"

#include "questmanager.h"
#include "questexceptions.h"
#include "quest.h"
#include "questregistrator.h"
#include "def.h"



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

QuestList QuestManager::all( ) const
{
    QuestList result;

    for (unsigned int i = 0; i < quests.size( ); i++)
        result.push_back( quests[i] );

    return result;
}

void QuestManager::generate( PCharacter *pch, NPCharacter *questor ) const {
    unsigned int summ, i, dice;
    QuestList qlist;
    DLString declined;

    for (summ = 0, i = 0; i < quests.size( ); i++) {
        if (quests[i]->applicable( pch, true )) {
            summ += quests[i]->getPriority( );
            qlist.push_back( quests[i] );
        }
    }

    // Nothing applicable, or every applicable type has zero weight. Zero weight is
    // fatal for the weighted pick below: 'i > dice' never becomes true, so ipos is
    // left at end() and dereferenced. Priority defaults to 0 for a registrator whose
    // XML omits <priority>, so this is one config edit away rather than impossible.
    if (qlist.empty( ) || summ == 0) {
        wiznet( WIZ_QUEST, 0, 0, "Failed to start quest for %s: %d of %d types applicable, total weight %d",
                pch->getNameC( ), (int)qlist.size( ), (int)quests.size( ), (int)summ );
        throw QuestCannotStartException( );
    }

    while (!qlist.empty( ) && summ > 0) {
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
            if (!declined.empty( ))
                declined += ", ";
            declined += (*ipos)->getName( );

            summ -= (*ipos)->getPriority( );
            qlist.erase( ipos );

        }
        catch (const Exception &e1) {
            LogStream::sendError( ) << e1.what( ) << endl;
            wiznet( WIZ_QUEST, 0, 0, "Failed to start quest %s for %s: %s",
                    (*ipos)->getName( ).c_str( ), pch->getNameC( ), e1.what( ) );
            throw QuestCannotStartException( );
        }
    }

    wiznet( WIZ_QUEST, 0, 0, "Failed to start quest for %s: declined by %s; %d type(s) left unpicked with zero weight",
            pch->getNameC( ), declined.c_str( ), (int)qlist.size( ) );
    throw QuestCannotStartException( );
}

void QuestManager::load( QuestRegistratorBase* reg ) {
    if (!loadXML( reg, reg->getName( ) ))
        return;

    // Two types sharing a feniaId key one Fenia DB entry between them, and one
    // type's scripts then run for the other -- the exact collision the explicit
    // id exists to prevent, moved into a config typo. Loud, but not fatal: the
    // type still loads and plays, it just cannot safely carry Fenia logic.
    int id = reg->getFeniaId( );

    if (id > 0)
        for (unsigned int i = 0; i < quests.size( ); i++)
            if (quests[i]->getFeniaId( ) == id)
                LogStream::sendError( )
                    << "Quest type " << reg->getName( ) << " shares feniaId " << id
                    << " with " << quests[i]->getName( )
                    << " -- their Fenia scripts will collide" << endl;

    quests.push_back( reg );
}

int QuestManager::reload( ) {
    int count = 0;

    for (QuestRegistry::iterator i = quests.begin( ); i != quests.end( ); i++) {
        const DLString &name = (*i)->getName( );

        if (loadXML( i->getPointer( ), name ))
            count++;
        else
            LogStream::sendError( ) << "Quest config reload failed for " << name << endl;
    }

    return count;
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
