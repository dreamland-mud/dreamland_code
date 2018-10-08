/* $Id: spellmanager.h,v 1.1.2.4.10.3 2008/04/14 20:12:37 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef SPELLMANAGER_H
#define SPELLMANAGER_H

#include <list>
#include "xmlreversevector.h"
#include "xmlstring.h"
#include "oneallocate.h"
#include "plugin.h"

class Spell;
class Character;

typedef ::Pointer<Spell> SpellPointer;

class SpellManager : public Plugin, public OneAllocate {
public:        
        typedef ::Pointer<SpellManager> Pointer;
        typedef std::list<SpellPointer> SpellList;
        typedef XMLReverseVector<XMLString> Priorities;
        
        SpellManager( );
        virtual ~SpellManager( );

        virtual void initialization( );
        virtual void destruction( );
        
        static void registrate( SpellPointer );
        static void unregistrate( SpellPointer );
        static SpellPointer lookup( const DLString &, Character * );

        static inline SpellManager* getThis( )        
        {
            return thisClass;
        }

private:
        static const DLString priorityFile;
        static void loadPriorities( );
        static Priorities priorities;
        static bool compare( SpellPointer, SpellPointer );

        static SpellManager* thisClass;

        static SpellList spells;
};


#endif
