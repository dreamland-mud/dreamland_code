/* $Id: cclan.h,v 1.1.6.4.6.1 2007/06/26 07:09:44 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
                          cclan.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef CCLAN_H
#define CCLAN_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;
class PCMemoryInterface;
class Clan;

/**
 * @author Igor S. Petrenko
 */
class CClan : public CommandPlugin
{
XML_OBJECT;
public:
        typedef ::Pointer<CClan> Pointer;

        CClan( );

        virtual void run( Character* ch, const DLString& command );
        
private:
        void usage( PCharacter* pc );
        void doInduct( PCMemoryInterface *, const Clan & );

        void clanList( PCharacter* pc );
        void clanCount( PCharacter* pc );
        void clanRating( PCharacter* pc );
        void clanStatus( PCharacter* pc );
        void clanInduct( PCharacter *, DLString & );
        void clanScan( PCharacter *pc );
        
        void clanBank( PCharacter* pc, DLString& argument );
        bool clanBankDeposit( PCharacter *, Clan *, int, int, ostringstream & );
        bool clanBankWithdraw( PCharacter *, PCharacter *, Clan *, Clan *, int, int, ostringstream & );
        void clanBankHelp( PCharacter* pc );

        void clanRemove( PCharacter* pc, DLString& argument );
        void clanRemoveHelp( PCharacter* pc );
        
        void clanLevel( PCharacter *pc, DLString& argument );
        void clanLevelSet( PCharacter *pc, PCMemoryInterface *victim, const DLString& arg );
        void clanLevelShow( PCharacter *pc, PCMemoryInterface *victim );
        void clanLevelList( PCharacter *pc );
        void clanLevelHelp( PCharacter* pc );

        void clanMember( PCharacter* pc, DLString& argument );
        void clanMemberHelp( PCharacter* pc );

        void clanPetition( PCharacter *pc, DLString& argument );
        void clanPetitionList( PCharacter *pc );
        void clanPetitionAccept( PCharacter *pc, DLString& argument );
        void clanPetitionReject( PCharacter *pc, DLString& argument );
        void clanPetitionHelp( PCharacter *pc );
        
        void clanDiplomacy( PCharacter *pc, DLString& argument );
        void clanDiplomacyShow( PCharacter *pc );
        void clanDiplomacyForBlindShow( PCharacter *pc );
        void clanDiplomacyProp( PCharacter *pc );
        void clanDiplomacySet( PCharacter *pc, DLString& argument );
        void clanDiplomacyList( PCharacter *pc );
        void clanDiplomacyHelp( PCharacter *pc );


private:
        static const DLString COMMAND_NAME;
};


#endif
