/* $Id: ban.h,v 1.1.2.3 2009/09/24 14:09:12 rufina Exp $
 *
 * ruffina, 2005 
 */

#ifndef BAN_H
#define BAN_H

#include <iostream>

#include "oneallocate.h"
#include "xmlvariablecontainer.h"
#include "xmlvector.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmldate.h"
#include "schedulertaskroundplugin.h"
#include "xmlfile.h"
#include "banflags.h"

class Descriptor;
class BanManager;

extern BanManager *banManager;

class Ban : public XMLVariableContainer {
XML_OBJECT
public:
    Ban( );

    bool match( const DLString & ) const;
    
    XML_VARIABLE XMLString pattern;
    XML_VARIABLE XMLString responsible;
    XML_VARIABLE XMLString comment;
    XML_VARIABLE XMLFlags flags;
    XML_VARIABLE XMLDate expire;
};

class BanManager : public XMLVectorBase<Ban>, 
                   public SchedulerTaskRoundPlugin,
		   public OneAllocate
{
public:
    typedef ::Pointer<BanManager> Pointer;

    BanManager( );
    virtual ~BanManager( );
    
    virtual void run( );
    virtual void after( );
    virtual int getPriority( ) const;
	
    virtual void initialization( );
    virtual void destruction( );
    
    void save( ) const;
    bool set( const DLString &, int, const Date & );
    bool del( const DLString & );
    bool del( int );

    bool check( const DLString &, int ) const;
    bool check( Descriptor *, int ) const;
    bool checkVerbose( Descriptor *, int ) const;
    
    inline static BanManager *getThis( );
    
private:
    static bool checkExpire( const Ban & );
    
    XMLFile banFile;
    
    static const DLString FILE_NAME, NODE_NAME;
};

inline BanManager *BanManager::getThis( )
{
    return banManager;
}

#endif
