/* $Id: train.h,v 1.1.2.2.6.3 2008/02/23 13:41:24 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef __TRAIN_H__ 
#define __TRAIN_H__ 

#include "basicmobilebehavior.h"

class PCharacter;

class Trainer : public BasicMobileDestiny {
XML_OBJECT
public:
	typedef ::Pointer<Trainer> Pointer;
    
	Trainer( );
	
	virtual int getOccupation( );

	void doGain( PCharacter *, DLString & );
	void doTrain( PCharacter *, DLString & );
};

#endif

