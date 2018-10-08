/* $Id: objectbehaviormanager.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef OBJECTBEHAVIORMANAGER_H
#define OBJECTBEHAVIORMANAGER_H

#include <stdio.h>

class Object;
struct obj_index_data;

class ObjectBehaviorManager {
public:        
        static void assign( Object * );
        static void parse( obj_index_data *, FILE * );
        static void parse( Object *, FILE * );
        static void save( const obj_index_data *, FILE * );
        static void save( const Object *, FILE * );
};

#endif
