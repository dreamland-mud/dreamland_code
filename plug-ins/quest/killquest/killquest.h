/* $Id: killquest.h,v 1.1.2.15.6.2 2008/03/06 17:48:34 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef KILLQUEST_H
#define KILLQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"

#include "xmlshort.h"
#include "xmlstring.h"

class KillQuest : public VictimQuestModel {
XML_OBJECT
public:
    typedef ::Pointer<KillQuest> Pointer;
    
    KillQuest( );

    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual Reward::Pointer reward( PCharacter *, NPCharacter * );
    virtual Room * helpLocation( );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual void destroy( );

    XML_VARIABLE XMLShort mode;       
    XML_VARIABLE XMLString roomName;       
    XML_VARIABLE XMLString areaName;       
    XML_VARIABLE XMLString mobName;       
    
protected:
    virtual bool checkMobileVictim( PCharacter *, NPCharacter * );
};


#endif
