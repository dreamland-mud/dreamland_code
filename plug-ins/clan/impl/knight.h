/* $Id: knight.h,v 1.1.6.5.4.4 2009/08/10 01:06:51 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef KNIGHT_H 
#define KNIGHT_H 

#include "clanmobiles.h"
#include "clantitles.h"

#include "xmlglobalbitvector.h"

class ClanItemKnight : public ClanItem {
XML_OBJECT
public:
        typedef ::Pointer<ClanItemKnight> Pointer;
    
        virtual void actDisappear( );
};

class ClanAltarKnight : public ClanAltar {
XML_OBJECT
public:
        typedef ::Pointer<ClanAltarKnight> Pointer;

        virtual void actAppear( );
        virtual void actDisappear( );
        virtual void actNotify( Character * );
};

#endif

