/* $Id: class_antipaladin.h,v 1.1.2.3.18.3 2008/02/24 23:04:36 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __CLASS_ANTIPALADIN_H__
#define __CLASS_ANTIPALADIN_H__

#include "objectbehavior.h"
#include "objectbehaviorplugin.h"
#include "basicmobilebehavior.h"

#include "xmlstring.h"
#include "xmlinteger.h"
#include "class.h"

class Character;

/*
 *  'shadow blade' behavior
 */
class ShadowBlade : public ObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<ShadowBlade> Pointer;
	
        ShadowBlade( );
	
	void adjustLevel( int level );

	virtual bool area( );
	virtual bool quit( Character *ch, bool count );
	virtual void fight( Character *victim );
	virtual bool canEquip( Character * );

	XML_VARIABLE XMLString owner;
	XML_VARIABLE XMLInteger castCnt;
	XML_VARIABLE XMLInteger castChance;
	XML_VARIABLE XMLIntegerNoEmpty bonus;
};

/*
 * antipaladin's guildmaster behavior
 */
class AntipaladinGuildmaster : public BasicMobileDestiny {
XML_OBJECT
public:
	typedef ::Pointer<AntipaladinGuildmaster> Pointer;
    
	virtual void give( Character *, Object * );
};

#endif
