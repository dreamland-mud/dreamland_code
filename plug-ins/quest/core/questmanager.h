/* $Id: questmanager.h,v 1.1.4.3.6.1 2007/06/26 07:20:02 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef QUESTMANAGER_H
#define QUESTMANAGER_H

#include <vector>

#include "plugin.h"
#include "dlxmlloader.h"

class PCharacter;
class NPCharacter;
class QuestRegistratorBase;

class QuestManager : public Plugin, public DLXMLLoader {
public:        
        typedef ::Pointer<QuestManager> Pointer;
        typedef std::vector< ::Pointer<QuestRegistratorBase> > QuestRegistry;
        
public:
        QuestManager( );
        virtual ~QuestManager( );
        
        virtual void initialization( );
        virtual void destruction( );
        
        virtual DLString getNodeName( ) const;
        virtual DLString getTableName( ) const;

        void generate( PCharacter *, NPCharacter * );
        void load( QuestRegistratorBase* );
        void unLoad( QuestRegistratorBase* );
        ::Pointer<QuestRegistratorBase> findQuestRegistrator( const DLString& );
        
        static inline QuestManager* getThis( ) {
            return thisClass;
        }

private:
        static QuestManager* thisClass;
        static const DLString TABLE_NAME;
        static const DLString NODE_NAME;
    
        QuestRegistry quests;
};

#endif
