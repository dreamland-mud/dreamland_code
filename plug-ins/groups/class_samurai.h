/* $Id: class_samurai.h,v 1.1.4.3.6.3 2008/01/09 12:19:25 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CLASS_SAMURAI_H 
#define CLASS_SAMURAI_H 

#include "basicmobilebehavior.h"
#include "objectbehavior.h"
#include "class_universal.h"

class Katana : public ObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<Katana> Pointer;

    virtual void wear( Character * );
    virtual bool mayFloat( ); 
};

class OwnedKatana : public Katana {
XML_OBJECT
public:
    typedef ::Pointer<OwnedKatana> Pointer;

    virtual void get( Character * );
    virtual bool isLevelAdaptive( ); 
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

class SamuraiUniclassAdept : public SamuraiGuildmaster, UniclassAdept {
XML_OBJECT
public:
        typedef ::Pointer<SamuraiUniclassAdept> Pointer;

        SamuraiUniclassAdept( );

        virtual void tell( Character *, const char * );
};

#endif

