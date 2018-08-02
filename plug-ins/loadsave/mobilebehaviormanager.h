/* $Id: mobilebehaviormanager.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef MOBILEBEHAVIORMANAGER_H
#define MOBILEBEHAVIORMANAGER_H

#include <stdio.h>

class NPCharacter;
struct mob_index_data;

class MobileBehaviorManager {
public:	
	static void assign( NPCharacter * );
	static void assignBasic( NPCharacter * );
	static void parse( mob_index_data *, FILE * );
	static void parse( NPCharacter *, FILE * );
	static void save( const mob_index_data *, FILE * );
	static void save( const NPCharacter *, FILE * );
};

#endif
