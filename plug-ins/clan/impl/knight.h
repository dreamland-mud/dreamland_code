/* $Id: knight.h,v 1.1.6.5.4.4 2009/08/10 01:06:51 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef KNIGHT_H 
#define KNIGHT_H 

#include "clanmobiles.h"
#include "clantitles.h"
#include "clanorg.h"

#include "xmlglobalbitvector.h"
#include "commandplugin.h"
#include "defaultcommand.h"

class KnightOrder : public ClanOrder {
XML_OBJECT
public:
    typedef ::Pointer<KnightOrder> Pointer;
    
    KnightOrder( );

    virtual bool canInduct( PCMemoryInterface * ) const;
    virtual const DLString &getTitle( PCMemoryInterface * ) const;

    XML_VARIABLE XMLGlobalBitvector classes;
    XML_VARIABLE ClanTitlesByLevel  titles; 
};

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

class ClanGuardKnight: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardKnight> Pointer;
    
protected:        
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual void actInvited( PCharacter *, Object * );
        virtual void actIntruder( PCharacter * );
        virtual void actGhost( PCharacter * );
        virtual void actGiveInvitation( PCharacter *, Object * );

        virtual int getCast( Character * );
};


class COrden : public CommandPlugin, public DefaultCommand {
public:
    typedef ::Pointer<COrden> Pointer;

    COrden( );
    virtual void run( Character *, const DLString & );
    virtual bool visible( Character * ) const;

private:
    void doUsage( PCharacter * );

    static const DLString COMMAND_NAME;
};

#endif

