/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ATTRACT_H__ 
#define __ATTRACT_H__ 

#include "xmlvector.h"
#include "xmllonglong.h"
#include "xmlattribute.h"
#include "behavior_utils.h"

class NPCharacter;
class PCharacter;

class XMLAttributeAttract: public XMLAttribute, public XMLVariableContainer 
{
XML_OBJECT
public: 
        typedef ::Pointer<XMLAttributeAttract> Pointer;

	XMLAttributeAttract( );
	virtual ~XMLAttributeAttract( );

	void addTarget( NPCharacter *, int );
	NPCharacter * findTarget( PCharacter *, int );

	XML_VARIABLE XMLVectorBase<XMLLongLong> targets;
};

/*
 * utils
 */
NPCharacter * find_attracted_mob( Character *, int );

template <typename Bhv>
::Pointer<Bhv> find_attracted_mob_behavior( Character *ch, int occType )
{
    NPCharacter *amob;
    ::Pointer<Bhv> behavior;
    
   amob = find_attracted_mob( ch, occType );

   if (amob && amob->behavior)
       behavior = amob->behavior.getDynamicPointer<Bhv>( );

   return behavior;
}

#endif

