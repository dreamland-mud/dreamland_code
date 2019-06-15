/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CHANNELS_H__
#define __CHANNELS_H__

#include "xmllist.h"

#include "commandplugin.h"
#include "defaultcommand.h"
#include "roomchannel.h"
#include "personalchannel.h"
#include "worldchannel.h"

class SpeechChannel : public RoomChannel {
XML_OBJECT    
public:
    typedef ::Pointer<SpeechChannel> Pointer;

    SpeechChannel( );

protected:
    virtual void triggers( Character *, const DLString & ) const;
};

class EmoteChannel : public RoomChannel {
XML_OBJECT    
public:
    typedef ::Pointer<EmoteChannel> Pointer;

    EmoteChannel( );

protected:
    virtual void triggers( Character *, const DLString & ) const;
    virtual bool canTalkGlobally( Character * ) const;
};

class TellChannel : public PersonalChannel {
XML_OBJECT
public:
    typedef ::Pointer<TellChannel> Pointer;
    
    TellChannel( );

protected:
    virtual Character * findListener( Character *, const DLString & ) const;
    virtual void triggers( Character *, Character *, const DLString & ) const;
};

class PageChannel : public PersonalChannel, public WorldChannel {
XML_OBJECT
public:
    typedef ::Pointer<PageChannel> Pointer;
    
    PageChannel( );

    virtual void run( Character *, const DLString & );

protected:
    virtual bool isGlobalListener( Character *, Character * ) const;
    virtual bool isPersonalListener( Character *, Character *, const DLString & ) const;
    virtual Character * findListener( Character *, const DLString & ) const;
    virtual DLString outputVict( Character *, Character *, const DLString &, const DLString & ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;
};

class ReplyChannel : public TellChannel {
XML_OBJECT
public:
    typedef ::Pointer<ReplyChannel> Pointer;
    
    ReplyChannel( );

protected:
    virtual Character * findListener( Character *, const DLString & ) const;
    virtual bool parseArguments( Character *, const DLString &, DLString &, DLString & ) const;
};

class ChannelsCommand : public CommandPlugin, public DefaultCommand {
XML_OBJECT    
public:
    typedef ::Pointer<ChannelsCommand> Pointer;
    typedef XMLListBase<XMLPointer<CommunicationChannel> > Channels;

    ChannelsCommand( );

    virtual void run( Character *, const DLString & );

protected:
    virtual void initialization( );
    virtual void destruction( );

    XML_VARIABLE Channels channels;
    static const DLString COMMAND_NAME;
};


#endif

