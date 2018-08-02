/* $Id: remortdata.h,v 1.1.2.6.4.3 2008/05/21 08:15:31 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef REMORTDATA_H
#define REMORTDATA_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmllong.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlvector.h"

class PCharacter;

class LifeData : public XMLVariableContainer {
XML_OBJECT;
public:
    typedef ::Pointer<LifeData> Pointer;
    
    LifeData( );
    
    XML_VARIABLE XMLString  race;
    XML_VARIABLE XMLString  classCh;
    XML_VARIABLE XMLLong    time;  
    XML_VARIABLE XMLBoolean bonus;
};

class Remorts : public XMLVectorContainer<LifeData>
{
XML_OBJECT
public:
    typedef ::Pointer<Remorts> Pointer;
    typedef XMLVectorBase<XMLInteger> Stats;
    
    Remorts( );

    void clear( );
    void clearBonuses( );
    void resetBonuses( );

    int countBonusLifes( ) const;
    int getHitPerLevel( int );
    int getManaPerLevel( int );
    int getSkillPointsPerLevel( int );
    static int bonusPerLevel( int, int );
    
    XML_VARIABLE XMLIntegerNoEmpty points;

    XML_VARIABLE Stats stats;
    XML_VARIABLE XMLBooleanNoFalse pretitle;
    XML_VARIABLE XMLIntegerNoEmpty hp;
    XML_VARIABLE XMLIntegerNoEmpty mana;
    XML_VARIABLE XMLIntegerNoEmpty level;
    XML_VARIABLE XMLIntegerNoEmpty skillPoints;

    XML_VARIABLE XMLIntegerNoEmpty owners;

    static const int POINT_PER_LIFE;
    static const int MAX_BONUS_LIFES;
};


#endif
