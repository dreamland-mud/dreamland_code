/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __COMMUNICATIONCHANNEL_H__
#define __COMMUNICATIONCHANNEL_H__

#include "xmlinteger.h"
#include "xmlenumeration.h"
#include "command.h"

class CommunicationChannel : public virtual Command {
XML_OBJECT
public:
    typedef ::Pointer<CommunicationChannel> Pointer;
    
    CommunicationChannel( );
    virtual ~CommunicationChannel( );

    virtual bool saveCommand() const;

    inline long long getOff( ) const;
    bool canHear( Character * ) const;

protected:
    virtual bool checkIsolator( Character *, Character * ) const;

    virtual void applyGarble( Character *, DLString & ) const;

    // Neutralize player-injected web/command tags ({hc, {hh, ...) in channel text
    // so they can't reach other players' screens as clickable commands: the tag is
    // dropped, the label and colours are kept. Mirrors what title.cpp does for
    // player titles. Applied to the message only, never the server format.
    void stripWeb( Character *, DLString & ) const;

    /** Resolve a channels.xml message in the reader's own language. Every message
     *  taken straight from the XML has to go through here, not to pecho raw. */
    DLString localized( const DLString &format, Character *reader ) const;

    virtual DLString outputVict( Character *, Character *, const DLString &, const DLString & ) const;
    virtual DLString outputChar( Character *, Character *, const DLString &, const DLString & ) const;
    virtual DLString outputSelf( Character *, const DLString &, const DLString & ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;

    XML_VARIABLE XMLFlagsNoEmpty off;
    XML_VARIABLE XMLBooleanNoFalse garble, isolate, deafen, hook, ooc;
    XML_VARIABLE XMLIntegerNoEmpty trustSpeak, trustHear;
    XML_VARIABLE XMLEnumeration positionOther;
};

inline long long CommunicationChannel::getOff( ) const
{
    return off.getValue( );
}

#endif
