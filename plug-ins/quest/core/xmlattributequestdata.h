/* $Id: xmlattributequestdata.h,v 1.1.4.5.6.2 2007/09/12 02:48:39 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef XMLATTRIBUTEQUESTDATA_H
#define XMLATTRIBUTEQUESTDATA_H

#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmllonglong.h"

#include "xmlattributeticker.h"
#include "playerattributes.h"
#include "xmlattributestatistic.h"
#include "scheduledxmlattribute.h"
#include "xmlattributeplugin.h"

class PCharacter;
class PCMemoryInterface;

class XMLAttributeQuestData : 
    public XMLAttributeTimer, 
    public ScheduledXMLAttribute, 
    public ScheduledPCMemoryXMLAttribute,
    public AttributeEventHandler<DeathArguments>,
    public AttributeEventHandler<PromptArguments>,
    public XMLAttributeStatistic
{
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeQuestData> Pointer;
    
    virtual bool handle( const DeathArguments & ); 
    virtual bool handle( const RemortArguments & ); 
    virtual bool handle( const PromptArguments & ); 

    virtual bool pull( PCharacter * );
    virtual bool pull( PCMemoryInterface * );

    virtual int getTime( ) const;
    virtual void setTime( int );
    void rememberLastQuest(const DLString &);
    int getLastQuestCount(const DLString &) const;
    void setStartTime();
    bool takesTooLong() const;

protected:
    XML_VARIABLE XMLInteger countdown;
    XML_VARIABLE XMLString lastQuestType;
    XML_VARIABLE XMLInteger lastQuestCount;    
    XML_VARIABLE XMLLongLongNoEmpty startTime;
};

#endif
