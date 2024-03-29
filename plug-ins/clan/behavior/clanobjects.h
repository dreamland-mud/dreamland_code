/* $Id: clanobjects.h,v 1.1.2.2 2007/09/15 09:24:10 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CLANOBJECTS_H
#define CLANOBJECTS_H

#include "objectbehaviormanager.h"

#include "clanreference.h"
#include "clanarea.h"

class ClanObject: public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<ClanObject> Pointer;
    
        ClanObject( );
        virtual ~ClanObject( );
        
        ClanArea::Pointer getClanArea( );
        XML_VARIABLE XMLClanReference clan;
};

class ClanItem : public ClanObject {
XML_OBJECT
public:
        typedef ::Pointer<ClanItem> Pointer;
    
        ClanItem( );

        virtual void get( Character *victim );
        virtual bool sac( Character *victim );
        virtual void give( Character *from, Character *to );
        virtual bool area( );
        virtual bool extract( bool );

        virtual void actDisappear( );

private:
        bool isHeld( ) const;
};

class ClanAltar : public ClanObject {
XML_OBJECT
public:
        typedef ::Pointer<ClanAltar> Pointer;
    
        ClanAltar( );

        virtual bool fetch( Character *ch, Object *item );
        virtual void actAppear( );
        virtual void actDisappear( );
        virtual void actNotify( Character * );
};

#endif
