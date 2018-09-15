/* $Id$
 *
 * ruffina, 2004
 */
#include "structwrappers.h"

#include "hometown.h"
#include "skill.h"
#include "profession.h"
#include "subprofession.h"
#include "craftattribute.h"
#include "room.h"
#include "pcharacter.h"
#include "pcrace.h"
#include "desire.h"
#include "clan.h"
#include "clantypes.h"

#include "nativeext.h"
#include "regcontainer.h"
#include "reglist.h"
#include "wrappermanager.h"
#include "subr.h"
#include "schedulerwrapper.h"
#include "characterwrapper.h"
#include "wrap_utils.h"

#include "handler.h"
#include "gsn_plugin.h"
#include "profflags.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

using namespace std;

DESIRE(thirst);
DESIRE(hunger);
DESIRE(full);

/*----------------------------------------------------------------------
 * Area
 *----------------------------------------------------------------------*/
NMI_INIT(AreaWrapper, "area");

static AREA_DATA *find_area( const DLString &filename )
{
    AREA_DATA *area;

    for (area = area_first; area; area = area->next)
        if (filename == area->area_file->file_name)
            return area;

    throw Scripting::Exception( "Area " + filename + " not found." );
}

AreaWrapper::AreaWrapper( const DLString &n )
                  : filename( n )
{
}

Scripting::Register AreaWrapper::wrap( const DLString &filename )
{
    AreaWrapper::Pointer aw( NEW, filename );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( aw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( AreaWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<AreaWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( AreaWrapper, filename, "название файла зоны" ) 
{
    return Scripting::Register( filename );
}

NMI_GET( AreaWrapper, name, "имя зоны (как видно по 'where')" ) 
{
    return Scripting::Register( find_area( filename )->name );
}


/*----------------------------------------------------------------------
 * Hometown
 *----------------------------------------------------------------------*/
NMI_INIT(HometownWrapper, "hometown");

HometownWrapper::HometownWrapper( const DLString &n )
                  : name( n )
{
}

Scripting::Register HometownWrapper::wrap( const DLString &name )
{
    HometownWrapper::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( HometownWrapper, isAllowed, "" )
{
    Hometown *ht = hometownManager->find( name );
    CharacterWrapper *charWrap;

    if (!ht)
	return Scripting::Register( false );

    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    charWrap = wrapper_cast<CharacterWrapper>( args.front( ) );
    
    if (charWrap->getTarget( )->is_npc( ))
	throw Scripting::Exception( "PC field requested on NPC" ); 

    return Scripting::Register( ht->isAllowed( charWrap->getTarget( )->getPC( ) ) );
}

NMI_INVOKE( HometownWrapper, empty, "" )
{
    return !name.empty( ) && name != "none";
}

NMI_INVOKE( HometownWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<HometownWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( HometownWrapper, name, "" ) 
{
    return Scripting::Register( name );
}

NMI_GET( HometownWrapper, landing, "" ) 
{
    return Scripting::Register( hometownManager->find( name )->getLanding( ) );
}

NMI_GET( HometownWrapper, recall, "vnum комнаты возврата (recall)" ) 
{
    return Scripting::Register( hometownManager->find( name )->getRecall( ) );
}

NMI_GET( HometownWrapper, areaname, "" ) 
{
    Room *room = get_room_index( hometownManager->find( name )->getAltar( ) );

    if (room)
	return Scripting::Register( room->area->name );
    else
	return Scripting::Register( DLString::emptyString );
}

NMI_GET( HometownWrapper, altname, "" ) 
{
    Room *room = get_room_index( hometownManager->find( name )->getAltar( ) );

    if (room)
	return Scripting::Register( room->area->altname );
    else
	return Scripting::Register( DLString::emptyString );
}

NMI_GET( HometownWrapper, credits, "Оригинальное англ название зоны хометауна" ) 
{
    Room *room = get_room_index( hometownManager->find( name )->getAltar( ) );

    if (room)
	return Scripting::Register( room->area->credits );
    else
	return Scripting::Register( DLString::emptyString );
}

/*----------------------------------------------------------------------
 * Profession
 *----------------------------------------------------------------------*/
NMI_INIT(ProfessionWrapper, "profession");

ProfessionWrapper::ProfessionWrapper( const DLString &n )
                  : name( n )
{
}

Scripting::Register ProfessionWrapper::wrap( const DLString &name )
{
    ProfessionWrapper::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( ProfessionWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<ProfessionWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( ProfessionWrapper, points, "" ) 
{
    return professionManager->find( name )->getPoints( );
}

static int weapon_vnum( int wclass )
{
    switch (wclass) {
        case WEAPON_SWORD:
            return 40102;
        case WEAPON_DAGGER:
            return 40101;
        case WEAPON_AXE:
            return 40119;
        case WEAPON_MACE:
            return 40117;
    }
    return -1;
}

NMI_INVOKE( ProfessionWrapper, bestWeapon, "внум лучшего новичкового оружия для расы и класса этого персонажа" )
{
    CharacterWrapper *ch;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    ch = wrapper_cast<CharacterWrapper>(args.front( ));
    if (ch->getTarget( )->getSkill( gsn_axe ) == 100)
        return weapon_vnum( WEAPON_AXE );
    if (ch->getTarget( )->getSkill( gsn_sword ) == 100)
        return weapon_vnum( WEAPON_SWORD );
    if (ch->getTarget( )->getSkill( gsn_dagger ) == 100)
        return weapon_vnum( WEAPON_DAGGER );
    if (ch->getTarget( )->getSkill( gsn_spear ) == 100)
        return weapon_vnum( WEAPON_SPEAR );

    return professionManager->find( name )->getWeapon( );
}

NMI_GET( ProfessionWrapper, weapon, "" ) 
{
    return professionManager->find( name )->getWeapon( );
}

NMI_GET( ProfessionWrapper, name, "" ) 
{
    return professionManager->find( name )->getName( );
}

NMI_GET( ProfessionWrapper, nameRus, "" ) 
{
    return professionManager->find( name )->getRusName( );
}

NMI_GET( ProfessionWrapper, nameMlt, "" ) 
{
    return professionManager->find( name )->getMltName( );
}

NMI_GET( ProfessionWrapper, ethos, "" ) 
{
    return professionManager->find( name )->getEthos( ).names( );
}

NMI_GET( ProfessionWrapper, alignName, "" ) 
{
    const Flags &a = professionManager->find( name )->getAlign( );
    
    if (a.equalsToBitNumber( N_ALIGN_EVIL ))
	return "злой";
    if (a.equalsToBitNumber( N_ALIGN_GOOD ))
	return "добрый";
    if (a.equalsToBitNumber( N_ALIGN_NEUTRAL ))
	return "нейтр.";

    return "любой";
}

NMI_GET( ProfessionWrapper, statPlus, "" ) 
{
    Profession *prof = professionManager->find( name );
    int stat;
    ostringstream buf;
    
    for (int s = 0; s < stat_table.size; s++) {
	if (s == STAT_CHA)
	    continue;

	stat = prof->getStat( s );
	
	if (stat <= 0)
	    continue;
	
	if (!buf.str( ).empty( ))
	    buf << ",";

	buf <<  stat_table.name( s );
    }

    return buf.str( );
}

NMI_INVOKE( ProfessionWrapper, goodSex, "проверить ограничения по полу на профессию для персонажа" )
{
    CharacterWrapper *ch;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
   
    Profession *prof =  professionManager->find( name );
    ch = wrapper_cast<CharacterWrapper>(args.front( ));
    return prof->getSex( ).isSetBitNumber( ch->getTarget( )->getSex( ) );
}

NMI_INVOKE( ProfessionWrapper, goodRace, "проверить ограничения по расе на профессию для персонажа" )
{
    CharacterWrapper *ch;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    Profession *prof =  professionManager->find( name );
    if (prof->getFlags( ).isSet( PROF_NEWLOCK ))
	return false;

    ch = wrapper_cast<CharacterWrapper>(args.front( ));
    return ch->getTarget( )->getRace( )->getPC( )->getClasses( )[prof->getIndex( )] > 0;
}

NMI_INVOKE( ProfessionWrapper, goodPersonality, "проверить ограничение на характер и этос на профессию для персонажа" )
{
    CharacterWrapper *ch;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    ch = wrapper_cast<CharacterWrapper>(args.front( ));
    Profession *prof = professionManager->find( name );
    if (!prof->getEthos( ).isSetBitNumber( ch->getTarget( )->ethos ))
        return false;
    if (!prof->getAlign( ).isSetBitNumber( ALIGNMENT(ch->getTarget( )) ))
        return false;
    return true;
}

/*----------------------------------------------------------------------
 * Race
 *----------------------------------------------------------------------*/
NMI_INIT(RaceWrapper, "race");

RaceWrapper::RaceWrapper( const DLString &n )
                  : name( n )
{
}

Scripting::Register RaceWrapper::wrap( const DLString &name )
{
    RaceWrapper::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( RaceWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<RaceWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( RaceWrapper, name, "" ) 
{
    return raceManager->find( name )->getName( );
}

NMI_GET( RaceWrapper, nameMlt, "" ) 
{
    return raceManager->find( name )->getPC( )->getMltName( );
}

NMI_GET( RaceWrapper, nameMale, "" ) 
{
    return raceManager->find( name )->getPC( )->getMaleName( );
}

NMI_GET( RaceWrapper, nameFemale, "" ) 
{
    return raceManager->find( name )->getPC( )->getFemaleName( );
}

NMI_INVOKE( RaceWrapper, nameRus, "" ) 
{
    CharacterWrapper *ch;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    ch = wrapper_cast<CharacterWrapper>(args.front( ));

    if (ch->getTarget( )->getSex( ) == SEX_FEMALE)
	return raceManager->find( name )->getPC( )->getFemaleName( );
    else
	return raceManager->find( name )->getPC( )->getMaleName( );
}

NMI_GET( RaceWrapper, hpBonus, "" ) 
{
    return raceManager->find( name )->getPC( )->getHpBonus( );
}

NMI_GET( RaceWrapper, manaBonus, "" ) 
{
    return raceManager->find( name )->getPC( )->getManaBonus( );
}

NMI_GET( RaceWrapper, pracBonus, "" ) 
{
    return raceManager->find( name )->getPC( )->getPracBonus( );
}

NMI_GET( RaceWrapper, det, "" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getDet( ).getValue( ) );
}

NMI_GET( RaceWrapper, aff, "" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getAff( ).getValue( ) );
}

NMI_GET( RaceWrapper, vuln, "" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getVuln( ).getValue( ) );
}

NMI_GET( RaceWrapper, res, "" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getRes( ).getValue( ) );
}

NMI_GET( RaceWrapper, imm, "" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getImm( ).getValue( ) );
}

NMI_GET( RaceWrapper, form, "" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getForm( ).getValue( ) );
}

NMI_GET( RaceWrapper, parts, "" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getParts( ).getValue( ) );
}

NMI_GET( RaceWrapper, size, "" ) 
{
    return raceManager->find( name )->getSize( ).getValue( );
}

NMI_GET( RaceWrapper, wearloc, "" ) 
{
    return raceManager->find( name )->getWearloc( ).toString( );
}

/*----------------------------------------------------------------------
 * Liquid
 *----------------------------------------------------------------------*/
NMI_INIT(LiquidWrapper, "liquid");

LiquidWrapper::LiquidWrapper( const DLString &n )
                  : name( n )
{
}

Scripting::Register LiquidWrapper::wrap( const DLString &name )
{
    LiquidWrapper::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( LiquidWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<LiquidWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}


NMI_GET( LiquidWrapper, name, "" ) 
{
    return liquidManager->find( name )->getName( );
}
NMI_GET( LiquidWrapper, short_descr, "" ) 
{
    return liquidManager->find( name )->getShortDescr( );
}
NMI_GET( LiquidWrapper, color, "" ) 
{
    return liquidManager->find( name )->getColor( );
}
NMI_GET( LiquidWrapper, sip_size, "" ) 
{
    return liquidManager->find( name )->getSipSize( );
}
NMI_GET( LiquidWrapper, flags, "" ) 
{
    return Scripting::Register( (int)liquidManager->find( name )->getFlags( ).getValue( ) );;
}
NMI_GET( LiquidWrapper, index, "" ) 
{
    return liquidManager->find( name )->getIndex( );
}
NMI_GET( LiquidWrapper, hunger, "" ) 
{
    return liquidManager->find( name )->getDesires( )[desire_hunger];
}
NMI_GET( LiquidWrapper, thirst, "" ) 
{
    return liquidManager->find( name )->getDesires( )[desire_thirst];
}
NMI_GET( LiquidWrapper, full, "" ) 
{
    return liquidManager->find( name )->getDesires( )[desire_full];
}

/*----------------------------------------------------------------------
 * Clan
 *----------------------------------------------------------------------*/
NMI_INIT(ClanWrapper, "clan");

ClanWrapper::ClanWrapper( const DLString &n )
                  : name( n )
{
}

Scripting::Register ClanWrapper::wrap( const DLString &name )
{
    ClanWrapper::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( ClanWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<ClanWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}


NMI_GET( ClanWrapper, name, "" ) 
{
    return clanManager->find( name )->getName( );
}
NMI_GET( ClanWrapper, index, "" ) 
{
    return clanManager->find( name )->getIndex( );
}
NMI_GET( ClanWrapper, color, "" ) 
{
    return clanManager->find( name )->getColor( );
}

static const char *diplomacy_names [] = {
    "alliance",
    "peace",
    "truce",
    "distrust",
    "aggression",
    "war",
    "subordination",
    "oppress",
    "none"
};

static const int diplomacy_count = sizeof(diplomacy_names) / sizeof(char *);

static int diplomacy_number( Clan *clan, Clan *otherClan )
{
    if (!otherClan || !clan)
	throw Scripting::CustomException( "No such clan" );
    
    if (!clan->hasDiplomacy( ) || !otherClan->hasDiplomacy( ))
	return diplomacy_count - 1;

    int dnum = clan->getData( )->getDiplomacy( otherClan );
    return URANGE( 0, dnum, diplomacy_count - 1 );
}

NMI_INVOKE( ClanWrapper, diplomacy, "" ) 
{
    DLString otherName;
    const Register &arg = get_unique_arg( args );

    if (arg.type == Register::STRING)
	otherName = arg.toString( );
    else 
	otherName = wrapper_cast<ClanWrapper>( arg )->name;

    return diplomacy_names
	      [ diplomacy_number( clanManager->find( name ),
	                          clanManager->findExisting( otherName ) )
	      ];
}

/*----------------------------------------------------------------------
 * CraftProfession
 *----------------------------------------------------------------------*/
NMI_INIT(CraftProfessionWrapper, "craftprofession");

CraftProfessionWrapper::CraftProfessionWrapper( const DLString &n )
                  : name( n )
{
}

Scripting::Register CraftProfessionWrapper::wrap( const DLString &name )
{
    CraftProfessionWrapper::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( CraftProfessionWrapper, api, "печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<CraftProfessionWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( CraftProfessionWrapper, name, "название профессии" ) 
{
    return craftProfessionManager->get( name )->getName( );
}

NMI_GET( CraftProfessionWrapper, nameRus, "название по-русски с падежами" ) 
{
    return craftProfessionManager->get( name )->getRusName( );
}

NMI_GET( CraftProfessionWrapper, nameMlt, "название во множественном числе" ) 
{
    return craftProfessionManager->get( name )->getMltName( );
}

NMI_INVOKE( CraftProfessionWrapper, setLevel, "(ch, level) установить персонажу уровень мастерства в этой профессии" )
{
    if (args.size( ) != 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    PCharacter *ch = arg2player(args.front());
    int level = args.back().toNumber();
    XMLAttributeCraft::Pointer attr = ch->getAttributes().getAttr<XMLAttributeCraft>("craft");
    attr->setProficiencyLevel(name, level);     
    return Scripting::Register();
}

NMI_INVOKE( CraftProfessionWrapper, getLevel, "(ch) получить уровень мастерства персонажа в этой профессии" )
{
    PCharacter *ch = args2player(args);
    XMLAttributeCraft::Pointer attr = ch->getAttributes().getAttr<XMLAttributeCraft>("craft");
    return Scripting::Register(attr->proficiencyLevel(name));
}
