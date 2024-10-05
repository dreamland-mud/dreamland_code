/* $Id: xmlattributestatistic.h,v 1.1.2.6.6.2 2007/09/23 00:04:27 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef XMLATTRIBUTESTATISTIC_H
#define XMLATTRIBUTESTATISTIC_H

#include <list>

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmlboolean.h"

#include "xmlattribute.h"
#include "playerattributes.h"

class PCMemoryInterface;

class XMLAttributeStatistic : 
    public RemortAttribute,
    public XMLVariableContainer 
{
XML_OBJECT
public: 
        typedef pair<DLString, int> StatRecord;
        typedef list<StatRecord> StatRecordList;
        typedef map<DLString, StatRecordList > Statistic;

        typedef ::Pointer<XMLAttributeStatistic> Pointer;
        typedef XMLMapBase<XMLInteger> Victories;

        XMLAttributeStatistic( );
        virtual ~XMLAttributeStatistic( );

        virtual bool handle( const RemortArguments & ); 

        void rememberVictory( const DLString& );
        void rememberPenalty(const DLString &);
        int getVictories( const DLString& ) const;
        const Victories & getVictories( ) const;
        void setVictories( const DLString&, int );
        // Get total victories without penalties.
        int getAllVictoriesCount() const;
        // Get only victories that can give a bonus, i.e. don't have a penalty associated with them.
        int getBonusVictoriesCount() const;
        int getWasted( ) const;
        void setWasted( int );
    
        void gather( PCMemoryInterface *, Statistic & ) const;
        static Statistic gatherAll( const DLString& );
        
        static Scripting::Register toRankRegister(PCMemoryInterface *pci, const DLString &attrName);
        virtual Scripting::Register toRegister() const;
        Scripting::Register toRegister(PCMemoryInterface *pci, const DLString &attrName);

protected:
        // Record quest name / victory count pairs.
        XML_VARIABLE Victories victories;

        // Mark victories that cannot affect total victories count, e.g. personal quests gained while in a nopk clan.
        XML_VARIABLE Victories penalties;
        
        // Hide this player from 'q stat', 'gq stat'.
        XML_VARIABLE XMLBoolean shy;
        
        // How many of those victory bonuses are already used up (yes, spelling - but can't change as it's part of player profiles now).
        XML_VARIABLE XMLIntegerNoEmpty vasted;
};

#endif

