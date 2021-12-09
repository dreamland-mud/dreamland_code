/* $Id: masquerade.h,v 1.1.2.7 2005/11/21 19:07:11 rufina Exp $
 *
 * ruffina, cradya, 2003
 */

#ifndef MASQUERADE_H
#define MASQUERADE_H

#include "xmlinteger.h"
#include "schedulertask.h"

#include "areabehavior.h"
#include "mobilebehavior.h"
#include "objectbehaviormanager.h"
#include "objectbehaviorplugin.h"

class Masquer : public MobileBehavior {
XML_OBJECT
public:
        typedef ::Pointer<Masquer> Pointer;
    
protected:
        virtual void speech( Character *victim, const char *speech );
        virtual void tell( Character *victim, const char *speech );
};

class RoamingPortal : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<RoamingPortal> Pointer;
        
        RoamingPortal( );

        virtual bool area( );
      
protected:
        XML_VARIABLE XMLInteger frequency;
        XML_VARIABLE XMLInteger current;
        XML_VARIABLE XMLInteger lowlevel;
        XML_VARIABLE XMLInteger highlevel;
};

class CatsEye : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<CatsEye> Pointer;
        
        virtual bool drop( Character *victim );
        virtual void get( Character *victim );
        
protected:
        XML_VARIABLE XMLInteger recall;
};

#endif

