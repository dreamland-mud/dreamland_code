/* $Id: shalafi.h,v 1.1.6.1.10.2 2007/06/26 07:10:43 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef SHALAFI_H 
#define SHALAFI_H 

#include "xmlmap.h"
#include "clanmobiles.h"
#include "defaultclan.h"

class ClanGuardShalafi: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardShalafi> Pointer;
    
protected:        
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual int getCast( Character * );
};

class ShalafiDemon : public ClanSummonedCreature {
XML_OBJECT
public:
    typedef ::Pointer<ShalafiDemon> Pointer;

    virtual void conjure( );
};

class ShalafiFaculty : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLString shortDescr;
    XML_VARIABLE XMLStringList classes;
};
class ShalafiClan : public DefaultClan {
XML_OBJECT
public:
    typedef ::Pointer<ShalafiClan> Pointer;

    virtual void onInduct(PCharacter *) const;

    XML_VARIABLE XMLMapBase<ShalafiFaculty> faculties;
};

#endif
