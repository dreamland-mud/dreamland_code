/* $Id: objectbehaviormanager.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef OBJECTBEHAVIORMANAGER_H
#define OBJECTBEHAVIORMANAGER_H

#include <stdio.h>
#include "objectbehavior.h"

class DLString;
class Object;
struct obj_index_data;

class ObjectBehaviorManager {
public:        
        static void assign( Object * );
        static void assignBasic( Object * );
        static void assign( Object *obj, const DLString &behaviorClassName );
        static void clear( Object * );
        static void parse( obj_index_data *, FILE * );
        static void parse( Object *, FILE * );
        static void save( const obj_index_data *, FILE * );
        static void save( const Object *, FILE * );
};

/**
 * This behavior is assigned by default to all items. All other object behaviors enhance this class.
 * The 'owned' item logic it used to carry now lives in obj_owner_allows and
 * obj_owner_enforce (loadsave.h), keyed on the owner field rather than on this
 * class being present.
 */
class BasicObjectBehavior : public virtual ObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<BasicObjectBehavior> Pointer;

        virtual bool canConfiscate( );
};

#endif
