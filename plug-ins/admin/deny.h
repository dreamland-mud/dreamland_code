/* $Id: deny.h,v 1.1.2.3.22.2 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef DENY_H
#define DENY_H

#include "xmlstring.h"
#include "xmlattributeplugin.h"
#include "admincommand.h"
#include "xmlattributeticker.h"

class PCMemoryInterface;

class Deny : public AdminCommand {
XML_OBJECT
public:
        typedef ::Pointer<Deny> Pointer;
    
        Deny( );

        virtual void run( Character *, const DLString & );

        static void action(const DLString &args, ostringstream &buf);
        
private:
        static void doShow( PCMemoryInterface *vict, ostringstream &buf );
        static void doRemove( PCMemoryInterface *vict, ostringstream &buf );
        static void doPlace( PCMemoryInterface *vict, const DLString &arg, ostringstream &buf );
        static void doUsage( ostringstream &buf );

        static const DLString COMMAND_NAME;
};

class XMLAttributeDeny : public XMLAttributeMemoryTicker {
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeDeny> Pointer;

        XMLAttributeDeny( );
        XMLAttributeDeny( int );

        virtual void start( PCMemoryInterface *pcm ) const;
        virtual void end( PCMemoryInterface *pcm ) const;
        
        inline void setResponsible( const DLString & );
        inline const DLString &getResponsible( ) const;

protected:                        
        XML_VARIABLE XMLString responsible;
};

inline void XMLAttributeDeny::setResponsible( const DLString& rp )
{
    responsible = rp;
}

inline const DLString & XMLAttributeDeny::getResponsible( ) const
{
    return responsible.getValue( );
}


#endif

