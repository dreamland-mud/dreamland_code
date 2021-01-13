#ifndef RELIGION_ATTRIBUTE_H
#define RELIGION_ATTRIBUTE_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmllong.h"
#include "xmldate.h"
#include "xmlattribute.h"
#include "xmlflags.h"
#include "playerattributes.h"
#include "bonus.h"

class XMLAttributeReligion: public XMLVariableContainer, 
                            public virtual AttributeEventHandler<ScoreArguments>
{
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeReligion> Pointer;

        XMLAttributeReligion( );
        virtual ~XMLAttributeReligion( );
        
        virtual bool handle( const ScoreArguments &args ); 

        XML_VARIABLE XMLLong attempts;
        XML_VARIABLE XMLLong successes;
        XML_VARIABLE XMLLong angers;
        XML_VARIABLE XMLBonusReference prevBonus;
        XML_VARIABLE XMLLong prevBonusEnds;
};

#endif
