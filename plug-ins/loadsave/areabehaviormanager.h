/* $Id: areabehaviormanager.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef AREABEHAVIORMANAGER_H
#define AREABEHAVIORMANAGER_H

#include <stdio.h>

struct area_data;

class AreaBehaviorManager {
public:	
	static void parse( area_data *, FILE * );
	static void save( const area_data *, FILE * );
};

#endif
