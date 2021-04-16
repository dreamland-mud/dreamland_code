/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          objectmanager.h  -  description
                             -------------------
    begin                : Tue Jun 5 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "oneallocate.h"
#include "dllist.h"

class Object;


/**
 * @author Igor S. Petrenko
 */
class ObjectManager : public OneAllocate
{
public:        
        typedef ::Pointer<ObjectManager> Pointer;
        typedef DLList<Object>        ExtractList;

public:
        ObjectManager( );
        virtual ~ObjectManager( );
        
        
        static void extract( Object* object );
        static Object* getObject( );
        static Object *find(long long ID);
        
        static inline ObjectManager* getThis( )
        {
                return thisClass;
        }
        
private:
        static ObjectManager* thisClass;
        static ExtractList extractList;
};

#endif
