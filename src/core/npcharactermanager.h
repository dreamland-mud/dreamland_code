/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          charactermanager.h  -  description
                             -------------------
    begin                : Sun May 20 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef NPCHARACTERMANAGER_H
#define NPCHARACTERMANAGER_H

#include "dllist.h"
#include "oneallocate.h"

class NPCharacter;


/**
 * @author Igor S. Petrenko
 */
class NPCharacterManager : public OneAllocate
{
public:	
	typedef ::Pointer<NPCharacterManager> Pointer;
	typedef DLList<NPCharacter>	ExtractList;

public:
	NPCharacterManager( );
	virtual ~NPCharacterManager( );
	
	static void extract( NPCharacter* ch );
	static NPCharacter* getNPCharacter( );
	
	static inline NPCharacterManager* getThis( )
	{
		return thisClass;
	}

private:
	static NPCharacterManager* thisClass;
	static ExtractList extractList;
};

#endif
