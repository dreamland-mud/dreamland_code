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
    
    static void action(const DLString &args, ostringstream &buf);
    
private:
    static void doList( ostringstream &buf );
    static void doSet( const DLString &args, ostringstream &buf  );
    static void doDel( const DLString &args, ostringstream &buf  );
    static void doUsage( ostringstream &buf );
    static void doBan( const DLString &args, ostringstream &buf  );
    static void doShow( const Ban &ban, ostringstream &buf );
    static void doKick( ostringstream &buf );

    static const DLString COMMAND_NAME;
};

#endif

