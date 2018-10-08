
/* $Id: twitlist.h,v 1.1.2.1.8.1 2007/06/26 07:12:08 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __TWITLIST_H__ 
#define __TWITLIST_H__ 

#include "commandplugin.h"
#include "defaultcommand.h"
#include "xmlattribute.h"
#include "xmllist.h"
#include "xmlstring.h"

class PCharacter;
class DLString;

class XMLAttributeTwitList : public XMLAttribute, public XMLListBase<XMLString> {
public:
    typedef ::Pointer<XMLAttributeTwitList> Pointer;

    XMLAttributeTwitList( );
    virtual ~XMLAttributeTwitList( );

    virtual const DLString &getType( ) const 
    {
        return TYPE;
    }
    
    static const DLString TYPE;                                             

    bool isAvailable( const DLString & ) const;
};


class CTwit : public CommandPlugin, public DefaultCommand {
public:
    typedef ::Pointer<CTwit> Pointer;

    CTwit( );

    virtual void run( Character *, const DLString & );
    
private:
    void doAdd( PCharacter *, DLString & );
    void doRemove( PCharacter *, DLString & );
    void doList( PCharacter * );
    void doUsage( PCharacter * );

    static const DLString COMMAND_NAME;
};

#endif

