/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PERSONALCHANNEL_H__
#define __PERSONALCHANNEL_H__

#include "communicationchannel.h"

class PersonalChannel : public virtual CommunicationChannel {
XML_OBJECT
public:
    typedef ::Pointer<PersonalChannel> Pointer;
    
    PersonalChannel( );
    virtual void run( Character *, const DLString & );

protected:
    virtual bool canTalkPersonally( Character * ) const;
    virtual bool isPersonalListener( Character *, Character *, const DLString & ) const;
    virtual bool parseArguments( Character *, const DLString &, DLString &, DLString & ) const;
    virtual Character * findListener( Character *, const DLString & ) const;
    virtual void triggers( Character *, Character *, const DLString & ) const;
    
    virtual bool checkAFK( Character *, Character *, const DLString & ) const;
    virtual bool checkAutoStore( Character *, Character *, const DLString & ) const;
    virtual bool checkDisconnect( Character *, Character *, const DLString & ) const;
    virtual bool checkPosition( Character *, Character * ) const;
    virtual bool checkVictimDeaf( Character *, Character * ) const;

    virtual bool needOutputVict( Character *, Character * ) const;
    virtual bool needOutputChar( Character * ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;

    void tellToBuffer( Character *, Character *, const DLString &, const DLString & ) const;

    XML_VARIABLE XMLStringNoEmpty msgNoName, msgNoArg;
    XML_VARIABLE XMLStringNoEmpty msgAuto, msgChar, msgVict;
};

#endif
