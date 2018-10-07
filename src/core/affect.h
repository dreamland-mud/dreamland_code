/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          affect.h  -  description
                             -------------------
    begin                : Thu Apr 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef AFFECT_H
#define AFFECT_H

#include "xmlvariablecontainer.h"
#include "xmlglobalbitvector.h"
#include "skillreference.h"

class Character;

/**
 * @author Igor S. Petrenko
 */
class Affect : public XMLVariableContainer
{
XML_OBJECT;
public:
	typedef ::Pointer<Affect> Pointer;

public:
	Affect* next;
	short where;
	XMLSkillReference type;
	short level;
	short duration;
	short location;
	short modifier;
	int bitvector;
	XMLGlobalBitvector global;
	DLString ownerName;
	
	Affect( );
	virtual ~Affect( );
	
	Affect* affect_find( int );
	Character *getOwner( ) const;
};

#endif
