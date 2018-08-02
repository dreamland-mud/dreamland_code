/* $Id: xmlattributequestdata.h,v 1.1.4.5.6.2 2007/09/12 02:48:39 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef XMLATTRIBUTEQUESTDATA_H
#define XMLATTRIBUTEQUESTDATA_H

#include "xmlinteger.h"

#include "xmlattributeticker.h"
#include "playerattributes.h"
#include "xmlattributestatistic.h"
#include "scheduledxmlattribute.h"
#include "xmlattributeplugin.h"

class PCharacter;

class XMLAttributeQuestData : 
    public XMLAttributeTimer, 
    public ScheduledXMLAttribute, 
    public EventHandler<DeathArguments>,
    public EventHandler<PromptArguments>,
    public XMLAttributeStatistic
{
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeQuestData> Pointer;
    
    virtual bool handle( const DeathArguments & ); 
    virtual bool handle( const RemortArguments & ); 
    virtual bool handle( const PromptArguments & ); 

    virtual bool pull( PCharacter * );

    virtual int getTime( ) const;
    virtual void setTime( int );

protected:
    XML_VARIABLE XMLInteger countdown;
};

#endif
