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

typedef std::list< ::Pointer<QuestRegistratorBase> > QuestList;

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

        void generate( PCharacter *, NPCharacter * ) const;
        QuestList list(PCharacter *) const;

        /** Every registered type, unfiltered. list() above answers "what can
         *  this player be offered", which is a different question. */
        QuestList all( ) const;
        void load( QuestRegistratorBase* );
        void unLoad( QuestRegistratorBase* );

        /** Re-read every registered quest type's config from disk, returning how
         *  many files loaded. Registration, in-flight quests and the pick order
         *  are untouched: priority and applicability are read live off the
         *  registrator on every generate()/list() call.
         *
         *  This is a MERGE, not a replacement, and deliberately so. A node absent
         *  from the file is not read at all, so its member keeps the value it had
         *  -- true of every scalar, and true of the scenario map, which merges by
         *  key (xmlmap.h). Deleting something therefore needs a reboot; editing
         *  does not, which is the case this exists for.
         *
         *  Making scenarios the one thing reload could delete was tried and
         *  reverted: a quest already in flight looks its scenario up by name on
         *  every info()/reward() call and on mob spec ticks, getScenario throws
         *  QuestRuntimeException when the name is gone, and nothing between there
         *  and main() catches it. Dropping a key would have turned a config edit
         *  into a process exit.
         *
         *  A file that parses but throws part way through (unregistered scenario
         *  type, garbage where an integer belongs) still leaves that one type
         *  half-updated: loadXML only guards the read, not the per-variable
         *  assignment. Reload is for tuning values, not for rescuing a broken
         *  file. */
        int reload( );

        /** How many quest types are registered. Not the same as list()'s size,
         *  which is filtered by what a given player can be offered. */
        inline size_t size( ) const {
            return quests.size( );
        }
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
