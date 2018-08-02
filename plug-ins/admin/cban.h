/* $Id: cban.h,v 1.1.4.3.20.2 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CBAN_H
#define CBAN_H

#include "xmlstring.h"
#include "xmlattributeplugin.h"
#include "admincommand.h"
#include "xmlattributeticker.h"

class PCMemoryInterface;
class Ban;

class CBan : public AdminCommand {
XML_OBJECT
public:
    typedef ::Pointer<CBan> Pointer;

    CBan( );

    virtual void run( Character *, const DLString & );
    
private:
    void doList( Character * );
    void doSet( Character *, const DLString & );
    void doDel( Character *, const DLString & );
    void doUsage( Character * );
    void doBan( Character *, const DLString & );
    void doShow( Character *, const Ban & );
    void doKick( Character * );

    static const DLString COMMAND_NAME;
};

#endif

