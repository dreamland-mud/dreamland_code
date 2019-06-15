/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __COMMUNICATIONCHANNEL_H__
#define __COMMUNICATIONCHANNEL_H__

#include "xmlinteger.h"
#include "xmlenumeration.h"
#include "defaultcommand.h"

class CommunicationChannel : public DefaultCommand {
XML_OBJECT
public:
    typedef ::Pointer<CommunicationChannel> Pointer;
    
    CommunicationChannel( );
    virtual ~CommunicationChannel( );

    inline long long getOff( ) const;
    bool canHear( Character * ) const;

protected:
    virtual bool checkIgnore( Character *, Character * ) const;
    virtual bool checkIsolator( Character *, Character * ) const;

    virtual void applyGarble( Character *, DLString &, Character * ) const;

    virtual DLString outputVict( Character *, Character *, const DLString &, const DLString & ) const;
    virtual DLString outputChar( Character *, Character *, const DLString &, const DLString & ) const;
    virtual DLString outputSelf( Character *, const DLString &, const DLString & ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;

    XML_VARIABLE XMLFlagsNoEmpty off;
    XML_VARIABLE XMLBooleanNoFalse ignore, garble, isolate, deafen;
    XML_VARIABLE XMLIntegerNoEmpty trustSpeak, trustHear;
    XML_VARIABLE XMLEnumeration positionOther;
};

inline long long CommunicationChannel::getOff( ) const
{
    return off.getValue( );
}

#endif
