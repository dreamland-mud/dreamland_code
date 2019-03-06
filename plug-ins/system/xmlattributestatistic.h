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
        int getVictories( const DLString& ) const;
        const Victories & getVictories( ) const;
        void setVictories( const DLString&, int );
        int getAllVictoriesCount( ) const;
        int getVasted( ) const;
        void setVasted( int );
    
        void gather( PCMemoryInterface *, Statistic & ) const;
        static Statistic gatherAll( const DLString& );

protected:
        XML_VARIABLE Victories victories;
        XML_VARIABLE XMLBoolean shy;
        XML_VARIABLE XMLIntegerNoEmpty vasted;
};

#endif

