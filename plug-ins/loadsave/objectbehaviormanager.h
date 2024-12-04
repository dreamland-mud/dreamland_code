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
 * It deals mainly with the 'owned' item logic
 */
class BasicObjectBehavior : public virtual ObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<BasicObjectBehavior> Pointer;

        virtual void get( Character * );
        virtual bool canConfiscate( );
        virtual bool save( );
        virtual void delete_( Character * ); 
        virtual bool canSteal( Character * );
        virtual bool canEquip( Character * );      

protected:
        bool checkOwnership(Character *);                          
};

#endif
