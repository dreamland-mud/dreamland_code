/* $Id: areabehaviormanager.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef AREABEHAVIORMANAGER_H
#define AREABEHAVIORMANAGER_H

#include <stdio.h>

struct AreaIndexData;

class AreaBehaviorManager {
public:        
        static void parse( AreaIndexData *, FILE * );
        static void save( const AreaIndexData *, FILE * );
};

#endif
