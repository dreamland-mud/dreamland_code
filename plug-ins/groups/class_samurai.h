/* $Id: class_samurai.h,v 1.1.4.3.6.3 2008/01/09 12:19:25 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CLASS_SAMURAI_H 
#define CLASS_SAMURAI_H 

#include "basicmobilebehavior.h"
#include "objectbehaviormanager.h"

class Katana : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<Katana> Pointer;

    virtual void wear( Character * );
    virtual bool mayFloat( ); 
    virtual bool canEquip( Character * );
    virtual void get( Character * );
};

class OwnedKatana : public Katana {
XML_OBJECT
public:
    typedef ::Pointer<OwnedKatana> Pointer;

    virtual void get( Character * );
};

class SamuraiGuildmaster : public virtual BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<SamuraiGuildmaster> Pointer;
    
        virtual void give( Character *, Object * );
        virtual void tell( Character *, const char * );

protected:
        void giveBack( Character *, Object * );
        void doFirstEnchant( Character *, Object * );
        void doOwner( Character *, Object * );
        bool checkPrice( Character *, int );
};

#endif

