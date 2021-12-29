/* $Id$
 *
 * ruffina, 2004
 */
#include "structwrappers.h"

#include "grammar_entities_impl.h"
#include "hometown.h"
#include "skill.h"
#include "skillcommand.h"
#include "skillgroup.h"
#include "profession.h"
#include "defaultreligion.h"
#include "language.h"
#include "languagemanager.h"
#include "wordeffect.h"
#include "subprofession.h"
#include "room.h"
#include "pcharacter.h"
#include "pcrace.h"
#include "desire.h"
#include "clan.h"
#include "clantypes.h"
#include "spelltarget.h"
#include "material-table.h"
#include "material.h"

#include "nativeext.h"
#include "regcontainer.h"
#include "reglist.h"
#include "wrappermanager.h"
#include "subr.h"
#include "schedulerwrapper.h"
#include "characterwrapper.h"
#include "wrap_utils.h"

#include "calendar_utils.h"
#include "skill_utils.h"
#include "xmlattributerestring.h"
#include "handler.h"
#include "gsn_plugin.h"
#include "profflags.h"
#include "liquidflags.h"
#include "damageflags.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

using namespace std;

DESIRE(thirst);
DESIRE(hunger);
DESIRE(full);
CLAN(battlerager);

/*----------------------------------------------------------------------
 * Area
 *----------------------------------------------------------------------*/
NMI_INIT(AreaWrapper, "area, зона");

AreaWrapper::AreaWrapper( const DLString &n )
                  : filename( n )
{
}

AreaIndexData * AreaWrapper::getTarget() const
{
    AreaIndexData *pArea = get_area_index(filename);
    if (!pArea)
        throw Scripting::Exception( "Area " + filename + " not found." );

    return pArea;
}

Scripting::Register AreaWrapper::wrap( const DLString &filename )
{
    AreaWrapper::Pointer aw( NEW, filename );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( aw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( AreaWrapper, api, "(): печатает этот api" )
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
    return Scripting::Register( getTarget()->name );
}

NMI_GET( AreaWrapper, area_flag, "флаги зоны (таблица .tables.area_flags)" ) 
{
    return Scripting::Register((int)(getTarget()->area_flag));
}

NMI_GET( AreaWrapper, min_vnum, "нижняя граница диапазона vnum-ов зоны" ) 
{
    return Scripting::Register((int)(getTarget()->min_vnum));
}

NMI_GET( AreaWrapper, max_vnum, "верхняя граница диапазона vnum-ов зоны" ) 
{
    return Scripting::Register((int)(getTarget()->max_vnum));
}

NMI_GET( AreaWrapper, players, "список игроков из default экземпляра зоны" ) 
{
    RegList::Pointer rc(NEW);

    for(auto &r: getTarget()->area->rooms)
        for (auto &rch: r.second->getPeople())
            if (!rch->is_npc())
                rc->push_back(::wrap(rch));
    
    return ::wrap(rc);
}

/*----------------------------------------------------------------------
 * Hometown
 *----------------------------------------------------------------------*/
NMI_INIT(HometownWrapper, "hometown, город");

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

NMI_INVOKE( HometownWrapper, isAllowed, "(ch): доступен ли город персонажу ch" )
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

NMI_INVOKE( HometownWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<HometownWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( HometownWrapper, name, "английское название" ) 
{
    return Scripting::Register( name );
}

NMI_GET( HometownWrapper, recall, "vnum комнаты возврата (recall)" ) 
{
    return Scripting::Register( hometownManager->find( name )->getRecall( ) );
}

NMI_GET( HometownWrapper, areaname, "полное название арии" ) 
{
    Room *room = get_room_instance( hometownManager->find( name )->getAltar( ) );

    if (room)
        return Scripting::Register( room->areaName() );
    else
        return Scripting::Register( DLString::emptyString );
}

NMI_GET( HometownWrapper, altname, "альтернативное название арии" ) 
{
    Room *room = get_room_instance( hometownManager->find( name )->getAltar( ) );

    if (room)
        return Scripting::Register( room->areaIndex()->altname );
    else
        return Scripting::Register( DLString::emptyString );
}

NMI_GET( HometownWrapper, credits, "оригинальное англ название арии" ) 
{
    Room *room = get_room_instance( hometownManager->find( name )->getAltar( ) );

    if (room)
        return Scripting::Register( room->areaIndex()->credits );
    else
        return Scripting::Register( DLString::emptyString );
}

/*----------------------------------------------------------------------
 * Profession
 *----------------------------------------------------------------------*/
NMI_INIT(ProfessionWrapper, "profession, класс персонажа");

ProfessionWrapper::ProfessionWrapper( const DLString &n )
                  : name( n )
{
}

NMI_INVOKE( ProfessionWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<ProfessionWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( ProfessionWrapper, points, "дополнительные очки опыта" ) 
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
            return 40100;
	case WEAPON_SPEAR:
	    return 40117;
    }
    return -1;
}

NMI_INVOKE( ProfessionWrapper, bestWeapon, "(ch): vnum лучшего новичкового оружия для расы и класса персонажа ch" )
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
    if (ch->getTarget( )->getSkill( gsn_mace ) == 100)
        return weapon_vnum( WEAPON_MACE );
    if (ch->getTarget( )->getSkill( gsn_spear ) == 100)
        return weapon_vnum( WEAPON_SPEAR );

    return professionManager->find( name )->getWeapon( );
}

NMI_GET( ProfessionWrapper, name, "английское название" ) 
{
    return professionManager->find( name )->getName( );
}

NMI_GET( ProfessionWrapper, nameRus, "русское название с падежами" ) 
{
    return professionManager->find( name )->getRusName( );
}

NMI_GET( ProfessionWrapper, nameMlt, "русское название во множ.числе с падежами" ) 
{
    return professionManager->find( name )->getMltName( );
}

NMI_INVOKE( ProfessionWrapper, flags, "(ch): флаги класса для этого персонажа (таблица .tables.prof_flags)" ) 
{
    Character *ch = args2character(args);
    Profession *prof = professionManager->find( name );
    return Register((int)prof->getFlags(ch).getValue());
}


NMI_GET( ProfessionWrapper, ethos, "список подходящих этосов" ) 
{
    return professionManager->find( name )->getEthos( ).names( );
}

NMI_GET( ProfessionWrapper, alignName, "русское имя подходящей натуры или 'любая'" ) 
{
    const Flags &a = professionManager->find( name )->getAlign( );
    
    if (a.equalsToBitNumber( N_ALIGN_EVIL ))
        return "злая";
    if (a.equalsToBitNumber( N_ALIGN_GOOD ))
        return "добрая";
    if (a.equalsToBitNumber( N_ALIGN_NEUTRAL ))
        return "нейтр.";

    return "любая";
}

NMI_INVOKE(ProfessionWrapper, wearModifier, "(type): бонус на уровень владения этим типом предмета (.tables.item_table)")
{
    int itype = argnum2flag(args, 1, item_table);
    if (itype == NO_FLAG)
        throw Scripting::Exception("Item type not found");

    return professionManager->find(name)->getWearModifier(itype);
}

NMI_GET( ProfessionWrapper, statPlus, "какие параметры увеличиваются у представителей этого класса" ) 
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

NMI_INVOKE( ProfessionWrapper, goodSex, "(ch): проверить ограничения по полу на класс для персонажа ch" )
{
    CharacterWrapper *ch;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
   
    Profession *prof =  professionManager->find( name );
    ch = wrapper_cast<CharacterWrapper>(args.front( ));
    return prof->getSex( ).isSetBitNumber( ch->getTarget( )->getSex( ) );
}

NMI_INVOKE( ProfessionWrapper, goodRace, "(ch): проверить ограничения по расе на класс для персонажа ch" )
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

NMI_INVOKE( ProfessionWrapper, goodPersonality, "(ch): проверить ограничение на натуру и этос на класс для персонажа ch" )
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
NMI_INIT(RaceWrapper, "race, раса персонажа и моба");

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

NMI_INVOKE( RaceWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<RaceWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( RaceWrapper, name, "английское название" ) 
{
    return raceManager->find( name )->getName( );
}

NMI_GET( RaceWrapper, nameMlt, "русское название во множ.числе с падежами" ) 
{
    return raceManager->find( name )->getMltName( );
}

NMI_GET( RaceWrapper, nameMale, "русское название в мужском роде с падежами" ) 
{
    return raceManager->find( name )->getMaleName( );
}

NMI_GET( RaceWrapper, nameFemale, "русское название в женском роде с падежами" ) 
{
    return raceManager->find( name )->getFemaleName( );
}

NMI_INVOKE( RaceWrapper, nameRus, "(ch): русское название в зависимости от пола персонажа ch" ) 
{
    CharacterWrapper *ch;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    ch = wrapper_cast<CharacterWrapper>(args.front( ));

    if (ch->getTarget( )->getSex( ) == SEX_FEMALE)
        return raceManager->find( name )->getFemaleName( );
    else
        return raceManager->find( name )->getMaleName( );
}

NMI_GET( RaceWrapper, hpBonus, "бонус на здоровья при создании персонажа этой расы" ) 
{
    return raceManager->find( name )->getPC( )->getHpBonus( );
}

NMI_GET( RaceWrapper, manaBonus, "бонус на ману при создании персонажа этой расы" ) 
{
    return raceManager->find( name )->getPC( )->getManaBonus( );
}

NMI_GET( RaceWrapper, pracBonus, "бонус на кол-во практик при создании персонажа этой расы" ) 
{
    return raceManager->find( name )->getPC( )->getPracBonus( );
}

NMI_GET( RaceWrapper, det, "врожденные детекты (таблица .tables.detect_flags)" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getDet( ).getValue( ) );
}

NMI_GET( RaceWrapper, aff, "врожденные аффекты (таблица .tables.affect_flags)" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getAff( ).getValue( ) );
}

NMI_GET( RaceWrapper, vuln, "врожденные уязвимости (таблица .tables.vuln_flags)" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getVuln( ).getValue( ) );
}

NMI_GET( RaceWrapper, res, "врожденная сопротивляемость (таблица .tables.res_flags)" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getRes( ).getValue( ) );
}

NMI_GET( RaceWrapper, imm, "врожденный иммунитет (таблица .tables.imm_flags)" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getImm( ).getValue( ) );
}

NMI_GET( RaceWrapper, form, "формы тела (таблица .tables.form_flags)" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getForm( ).getValue( ) );
}

NMI_GET( RaceWrapper, parts, "части тела (таблица .tables.part_flags)" ) 
{
    return Scripting::Register( (int) raceManager->find( name )->getParts( ).getValue( ) );
}

NMI_GET( RaceWrapper, size, "размер (таблица .tables.size_table)" ) 
{
    return raceManager->find( name )->getSize( ).getValue( );
}

NMI_GET( RaceWrapper, wearloc, "список доступных wear locations" ) 
{
    return raceManager->find( name )->getWearloc( ).toString( );
}

/*----------------------------------------------------------------------
 * Liquid
 *----------------------------------------------------------------------*/
NMI_INIT(LiquidWrapper, "liquid, жидкость");

LiquidWrapper::LiquidWrapper( const DLString &n )
                  : name( n )
{
}

Scripting::Register LiquidWrapper::wrap( const DLString &name )
{
    if (!liquidManager->findExisting(name))
        throw Scripting::Exception("Liquid not found");

    LiquidWrapper::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );

    return Scripting::Register( sobj );
}

Liquid * LiquidWrapper::getTarget() const
{
    Liquid *liq = liquidManager->find(name);
    if (!liq)
        throw Scripting::Exception("Liquid not found");
    return liq;
}

NMI_INVOKE( LiquidWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<LiquidWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}


NMI_GET( LiquidWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}
NMI_GET( LiquidWrapper, short_descr, "русское название с цветами и падежами" ) 
{
    return getTarget()->getShortDescr( );
}
NMI_GET( LiquidWrapper, color, "прилагательное цвета с падежами" ) 
{
    return getTarget()->getColor( );
}
NMI_GET( LiquidWrapper, sip_size, "размер глотка" ) 
{
    return getTarget()->getSipSize( );
}
NMI_GET( LiquidWrapper, flags, "флаги жидкости (таблица .tables.liquid_flags)" ) 
{
    return Scripting::Register( (int)getTarget()->getFlags( ).getValue( ) );;
}
NMI_GET( LiquidWrapper, index, "внутренний порядковый номер" ) 
{
    return getTarget()->getIndex( );
}
NMI_GET( LiquidWrapper, hunger, "как хорошо утоляет голод" ) 
{
    return getTarget()->getDesires( )[desire_hunger];
}
NMI_GET( LiquidWrapper, thirst, "как хорошо утоляет жажду" ) 
{
    return getTarget()->getDesires( )[desire_thirst];
}
NMI_GET( LiquidWrapper, full, "как хорошо насыщает" ) 
{
    return getTarget()->getDesires( )[desire_full];
}
NMI_INVOKE( LiquidWrapper, isBooze, "алкоголь ли это" ) 
{
    return getTarget()->getFlags().isSet(LIQF_BEER|LIQF_LIQUOR|LIQF_WINE);
}

/*----------------------------------------------------------------------
 * Material
 *----------------------------------------------------------------------*/
NMI_INIT(MaterialWrapper, "material, материал(ы)");

MaterialWrapper::MaterialWrapper( const DLString &n )
                        : names(n)
{
}

Scripting::Register MaterialWrapper::wrap( const DLString &names )
{
    MaterialWrapper::Pointer mw( NEW, names );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( mw );

    return Scripting::Register( sobj );
}

material_t * MaterialWrapper::getTarget() const
{
    // TODO
    return 0;
}

NMI_INVOKE( MaterialWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<MaterialWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( MaterialWrapper, name, "английские названия" ) 
{
    return names;
}

NMI_GET( MaterialWrapper, nameRus, "русские названия с падежами" ) 
{
    return material_rname(names.c_str());
}

NMI_GET( MaterialWrapper, wood, "среди материалов есть дерево" )
{
    return Register( material_is_typed( names.c_str(), MAT_WOOD ) );
}

NMI_GET( MaterialWrapper, metal, "среди материалов есть металл" )
{
    return Register( material_is_typed( names.c_str(), MAT_METAL ) );
}

NMI_GET( MaterialWrapper, cloth, "среди материалов есть ткань" )
{
    return Register( material_is_typed( names.c_str(), MAT_CLOTH ) );
}

NMI_GET( MaterialWrapper, leather, "среди материалов есть кожа" )
{
    return Register( material_is_typed( names.c_str(), MAT_LEATHER ) );
}

NMI_GET( MaterialWrapper, mineral, "среди материалов есть камень или минерал" )
{
    return Register( material_is_typed( names.c_str(), MAT_MINERAL ) );
}

NMI_GET( MaterialWrapper, gem, "среди материалов есть драгоценный камень" )
{
    return Register( material_is_typed( names.c_str(), MAT_GEM ) );
}

NMI_GET( MaterialWrapper, burns, "сколько тиков горит (-1 если тушит огонь)" )
{
    return Register( material_burns( names.c_str() ) );
}

NMI_GET( MaterialWrapper, swims, "как плавает: 2=никогда не утонет; 1=тут же утонет; 0=будет зависеть от предмета" )
{
    return Register( material_swims( names.c_str() ) );
}

NMI_GET( MaterialWrapper, indestructible, "среди материалов есть неуничтожимый (dragonskin, platinum)" )
{
    return Register( material_is_flagged(names.c_str(), MAT_INDESTR) );
}


/*----------------------------------------------------------------------
 * Clan
 *----------------------------------------------------------------------*/
NMI_INIT(ClanWrapper, "clan, клан");

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

NMI_INVOKE( ClanWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<ClanWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}


NMI_GET( ClanWrapper, name, "английское название" ) 
{
    return clanManager->find( name )->getName( );
}
NMI_GET( ClanWrapper, index, "внутренний порядковый номер" ) 
{
    return clanManager->find( name )->getIndex( );
}
NMI_GET( ClanWrapper, color, "буква цвета" ) 
{
    return clanManager->find( name )->getColor( );
}
NMI_GET( ClanWrapper, dispersed, "true для разрозненных кланов (одиночки,изгои,внеклановые)" ) 
{
    return clanManager->find(name)->isDispersed();
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
        throw Scripting::Exception( "No such clan" );
    
    if (!clan->hasDiplomacy( ) || !otherClan->hasDiplomacy( ))
        return diplomacy_count - 1;

    int dnum = clan->getData( )->getDiplomacy( otherClan );
    return URANGE( 0, dnum, diplomacy_count - 1 );
}

NMI_INVOKE( ClanWrapper, diplomacy, "(clan): англ название дипломатии с кланом clan (clan dip list)" ) 
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
NMI_INIT(CraftProfessionWrapper, "craftprofession, дополнительная профессия");

CraftProfessionWrapper::CraftProfessionWrapper( const DLString &n )
                  : name( n )
{
}

CraftProfession * CraftProfessionWrapper::getTarget() const
{
    CraftProfession::Pointer prof = craftProfessionManager->get(name);
    if (!prof)
        throw Scripting::Exception("Profession not found");
    return *prof;
}

NMI_INVOKE( CraftProfessionWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<CraftProfessionWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( CraftProfessionWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}

NMI_GET( CraftProfessionWrapper, nameRus, "русское название с падежами" ) 
{
    return getTarget()->getRusName( );
}

NMI_GET( CraftProfessionWrapper, nameMlt, "название во множественном числе с падежами" ) 
{
    return getTarget()->getMltName( );
}

NMI_INVOKE( CraftProfessionWrapper, setLevel, "(ch, level): установить персонажу уровень мастерства в этой профессии" )
{
    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    PCharacter *ch = arg2player(args.front());
    int level = args.back().toNumber();
    getTarget()->setLevel(ch, level);
    return Scripting::Register();
}

NMI_INVOKE( CraftProfessionWrapper, getLevel, "(ch): получить уровень мастерства персонажа в этой профессии" )
{
    PCharacter *ch = args2player(args);
    return getTarget()->getLevel(ch);
}

NMI_INVOKE( CraftProfessionWrapper, getTotalExp, "(ch): суммарный опыт персонажа в этой профессии" )
{
    PCharacter *ch = args2player(args);
    return getTarget()->getCalculator(ch)->totalExp();
}

NMI_INVOKE( CraftProfessionWrapper, getExpToLevel, "(ch): кол-во опыта до следующего уровня мастерства в этой профессии" )
{
    PCharacter *ch = args2player(args);
    return getTarget()->getCalculator(ch)->expToLevel();
}

NMI_INVOKE( CraftProfessionWrapper, gainExp, "(ch, exp): заработать очков опыта в этой профессии" )
{
    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    PCharacter *ch = arg2player(args.front());
    int exp = args.back().toNumber();
    getTarget()->gainExp(ch, exp);
    return Scripting::Register();
}

/*----------------------------------------------------------------------
 * Bonus
 *----------------------------------------------------------------------*/
NMI_INIT(BonusWrapper, "bonus, календарный или религиозный бонус");

BonusWrapper::BonusWrapper( const DLString &n )
                  : name( n )
{
}

Bonus * BonusWrapper::getTarget() const
{
    Bonus::Pointer bonus = bonusManager->findExisting(name);
    if (!bonus)
        throw Scripting::Exception("Bonus not found");
    return *bonus;
}

NMI_INVOKE( BonusWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<BonusWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( BonusWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}

NMI_GET( BonusWrapper, nameRus, "русское название" ) 
{
    return getTarget()->getRussianName( );
}

NMI_GET( BonusWrapper, color, "буква цвета в календаре" ) 
{
    return getTarget()->getColor();
}

NMI_INVOKE( BonusWrapper, give, "(ch,days): дать бонус на days дней. Вернет true, если присвоено успешно.")
{
    PCharacter *ch = argnum2player(args, 1);
    long today = day_of_epoch(time_info);
    long end = today + argnum2number(args, 2);

    if (end < today)
        throw Scripting::Exception("end day cannot be negative");

    PCBonusData &data = ch->getBonuses().get(getTarget()->getIndex());
    // Do nothing for active bonuses.
    if (data.start <= today && today <= data.end)
        return Register(false);

    data.start = today;
    data.end = end;
    return Register(true);
}


NMI_INVOKE( BonusWrapper, remove, "(ch): очистить бонус у персонажа. Вернет true, если было что очищать.")
{
    PCharacter *ch = argnum2player(args, 1);
    PCBonusData &data = ch->getBonuses().get(getTarget()->getIndex());

    if (data.isValid()) {
        data.start = -1;
        data.end = -1;
        return Register(true);
    }

    return Register(false);
}

/*----------------------------------------------------------------------
 * Religion
 *----------------------------------------------------------------------*/
NMI_INIT(ReligionWrapper, "reglion, религия");

ReligionWrapper::ReligionWrapper( const DLString &n )
                  : name( n )
{
}

DefaultReligion * ReligionWrapper::getTarget() const
{
    Religion *rel = religionManager->findExisting(name);
    if (!rel)
        throw Scripting::Exception("Religion not found");

    DefaultReligion *religion = dynamic_cast<DefaultReligion *>(rel);
    if (!religion)
        throw Scripting::Exception("Religion not found");

    return religion;
}

NMI_INVOKE( ReligionWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<ReligionWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( ReligionWrapper, name, "английское название с маленькой буквы" ) 
{
    return getTarget()->getName( );
}

NMI_GET( ReligionWrapper, nameRus, "русское название с падежами" ) 
{
    return getTarget()->getRussianName( );
}

NMI_GET( ReligionWrapper, shortDescr, "английское название с большой буквы" ) 
{
    return getTarget()->getShortDescr( );
}

NMI_GET( ReligionWrapper, description, "описание (бог чего именно)" ) 
{
    return getTarget()->getDescription( );
}

NMI_GET( ReligionWrapper, sex, "пол божества (таблица .tables.sex_table)" ) 
{
    return Register((int)getTarget()->getSex());
}

NMI_GET( ReligionWrapper, tattooVnum, "vnum объекта-знака религии" ) 
{
    return Register(getTarget()->tattooVnum);
}

NMI_GET( ReligionWrapper, flags, "флаги религий (таблица .tables.religion_flags)" ) 
{
    return Register((int)getTarget()->flags.getValue());
}

NMI_GET( ReligionWrapper, align, "разрешенные натуры или пустая строка (таблица .tables.align_table)" ) 
{
    return Register((int)getTarget()->align.getValue());
}

NMI_GET( ReligionWrapper, ethos, "разрешенные этосы или пустая строка (таблица .tables.ethos_table)" ) 
{
    return Register((int)getTarget()->ethos.getValue());
}

NMI_GET( ReligionWrapper, classes, "разрешенные классы или пустая строка (olchelp class)" ) 
{
    return Register(getTarget()->classes.toString());
}

NMI_GET( ReligionWrapper, clans, "разрешенные кланы или пустая строка (olchelp clan)" ) 
{
    return Register(getTarget()->clans.toString());
}

NMI_GET( ReligionWrapper, races, "разрешенные расы или пустая строка (olchelp race)" ) 
{
    return Register(getTarget()->races.toString());
}

NMI_GET( ReligionWrapper, minstat, "список параметров, по которым ограничено сверху" ) 
{
    DefaultReligion *target = getTarget();
    RegList::Pointer list(NEW);

    for (int i = 0; i < stat_table.size; i++)
        if (target->minstat[i] != 0)
            list->push_back(Register(stat_table.name(i)));

    return wrap(list);
}

NMI_INVOKE( ReligionWrapper, available, "(ch): НОВАЯ ЛОГИКА - доступна ли религия персонажу")
{
    Character *ch = args2character(args);
    return getTarget()->available(ch);
}

NMI_INVOKE( ReligionWrapper, reasonWhy, "(ch): НОВАЯ ЛОГИКА - причина почему недоступна или пустая строка")
{
    Character *ch = args2character(args);
    return getTarget()->reasonWhy(ch);
}

/*----------------------------------------------------------------------
 * Language
 *----------------------------------------------------------------------*/
NMI_INIT(LanguageWrapper, "language, древний язык");

LanguageWrapper::LanguageWrapper( const DLString &n )
                  : name( n )
{
}

Language * LanguageWrapper::getTarget() const
{
    Language::Pointer lang = languageManager->findLanguage(name);
    if (!lang)
        throw Scripting::Exception("Language not found");
    return lang.getPointer();
}

NMI_INVOKE( LanguageWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<LanguageWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( LanguageWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}

NMI_GET( LanguageWrapper, nameRus, "русское название" ) 
{
    return getTarget()->getRussianName( );
}

NMI_INVOKE( LanguageWrapper, word, "(): создать одноразовое слово по правилам языка")
{
    return getTarget()->createDictum();
}

NMI_INVOKE( LanguageWrapper, runEffect, "(effect, ch, target): выполнить эффект с данным именем (good, bad, bless etc) от имени ch и с целью на персонажа,себя или предмет")
{
    DLString effectName = argnum2string(args, 1);
    PCharacter *ch = argnum2player(args, 2);
    SpellTarget::Pointer effectTarget = argnum2target(args, 3);

    WordEffect::Pointer effect = getTarget()->findEffect(effectName);
    if (!effect)
        throw Scripting::Exception(effectName + " effect not found for language " + name);

    switch (effectTarget->type) {
    case SpellTarget::CHAR:
        return Register(effect->run(ch, effectTarget->victim));

    case SpellTarget::OBJECT:
        return Register(effect->run(ch, effectTarget->obj));

    default:
        throw Scripting::Exception(effectName + " invalid target, can be character or item only");
    }
}

NMI_INVOKE( LanguageWrapper, effective, "(ch): узнать процент раскачки языка у персонажа" )
{
    Character *ch = args2character(args);
    return Register( getTarget()->getEffective(ch) );
}


/*----------------------------------------------------------------------
 * Skill
 *----------------------------------------------------------------------*/
NMI_INIT(SkillWrapper, "skill, умение или заклинание");

SkillWrapper::SkillWrapper( const DLString &n )
                  : name( n )
{
}

Skill * SkillWrapper::getTarget() const
{
    Skill *skill = skillManager->find(name);
    if (!skill)
        throw Scripting::Exception(name + ": skill no longer exists");
    return skill;
}

NMI_INVOKE( SkillWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<SkillWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( SkillWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}

NMI_GET( SkillWrapper, nameRus, "русское название" ) 
{
    return getTarget()->getRussianName( );
}

NMI_GET( SkillWrapper, index, "порядковый номер (для value у волшебных предметов)" ) 
{ 
    return getTarget()->getIndex();
}

NMI_GET(SkillWrapper, spellTarget, "флаги целей заклинания (.tables.target_table)")
{
    Spell::Pointer spell = getTarget()->getSpell();
    return spell ? spell->getTarget() : 0;
}

NMI_GET(SkillWrapper, spellType, "вид заклинания (.tables.spell_types)")
{
    Spell::Pointer spell = getTarget()->getSpell();
    return spell ? spell->getSpellType() : 0;
}

NMI_GET(SkillWrapper, groups, "список групп умения (.SkillGroup)")
{
    RegList::Pointer rc(NEW);

    for (auto &group: getTarget()->getGroups().toArray()) {
        DLString groupName = skillGroupManager->find(group)->getName();
        rc->push_back(SkillGroupWrapper::wrap(groupName));
    }

    return wrap(rc);
}

NMI_GET(SkillWrapper, group, "первая, часто и единственная, группа умения (.SkillGroup)")
{
    vector<int> gsns = getTarget()->getGroups().toArray();
    DLString groupName = gsns.empty() ? "none" : skillGroupManager->find(gsns.front())->getName();
    return SkillGroupWrapper::wrap(groupName);
}

NMI_INVOKE(SkillWrapper, beats, "(ch): длина задержки в пульсах для персонажа с учетом бонусов")
{
    Character *ch = args2character(args);
    return getTarget()->getBeats(ch);
}

NMI_INVOKE(SkillWrapper, mana, "(ch): цена этого умения в мане для персонажа ch")
{
    Character *ch = args2character(args);
    return getTarget()->getMana(ch);
}

NMI_INVOKE(SkillWrapper, moves, "(ch): цена этого умения в шагах для персонажа ch [пока что одинакова для всех]")
{
    Character *ch = args2character(args);
    return getTarget()->getMoves(ch);
}

NMI_INVOKE(SkillWrapper, level, "(ch): уровень умения для персонажа с учетом бонусов")
{
    Character *ch = args2character(args);
    return skill_level(*getTarget(), ch);
}

NMI_INVOKE( SkillWrapper, usable, "(ch): доступно ли умение для использования прямо сейчас персонажу ch" )
{
    Character *ch = args2character(args);
    return getTarget()->usable( ch, false );
}

NMI_INVOKE( SkillWrapper, visible, "(ch): видно ли это умение ch, независимо от уровня, включая временные скилы" )
{
    Character *ch = args2character(args);
    return getTarget()->visible( ch );
}

NMI_INVOKE( SkillWrapper, adept, "(ch): вернуть максимальное значение, до которого можно практиковаться" )
{
    PCharacter *ch = args2player(args); 
    return getTarget()->getAdept(ch);
}

NMI_INVOKE( SkillWrapper, learned, "(ch[,percent]): вернуть разученность или установить ее в percent" )
{
    PCharacter *ch = args2player(args); 
    int sn = getTarget()->getIndex();

    if (args.size() > 1) {
        int value = args.back( ).toNumber( );
        
        if (value < 0)
            throw Scripting::IllegalArgumentException( );
        
        ch->getSkillData(sn).learned = value;
        return Register( );
    }

    return Register(ch->getSkillData(sn).learned);
}

NMI_INVOKE( SkillWrapper, effective, "(ch): узнать процент раскачки у персонажа" )
{
    Character *ch = args2character(args);
    return Register( getTarget()->getEffective(ch) );
}

NMI_INVOKE( SkillWrapper, improve, "(ch,success[,victim]): попытаться улучшить знание умения на успехе/неудаче (true/false), применен на жертву" )
{
    Character *ch = argnum2character(args, 1);
    int success = argnum2number(args, 2);
    Character *victim = args.size() > 2 ? argnum2character(args, 3) : NULL;
     
    getTarget()->improve( ch, success, victim );
    return Register( );
}

NMI_INVOKE( SkillWrapper, giveTemporary, "(ch[,learned[,days[,origin]]]): присвоить временное умение персонажу, разученное на learned % (или на 75%), работающее days дней (или вечно), помеченное как origin (или fenia). Вернет true, если присвоено успешно.")
{
    PCharacter *ch = argnum2player(args, 1);
    int learned = args.size() > 1 ? argnum2number(args, 2) : ch->getProfession()->getSkillAdept();
    long today = day_of_epoch(time_info);
    long end;
    int origin;

    if (args.size() <= 2)
        end = PCSkillData::END_NEVER;
    else {
        end = argnum2number(args, 3);
        if (end < 0)
            throw Scripting::Exception("end day cannot be negative");
        end = today + end;
    }

    if (args.size() >= 4)
        origin = argnum2flag(args, 4, skill_origin_table);
    else
        origin = SKILL_FENIA;

    if (learned <= 0)
        throw Scripting::Exception("learned param cannot be negative");

    // Do nothing for already available permanent or temporary skills.
    Skill *skill = getTarget();
    if (skill->available(ch))
        return Register(false);
    
    // Do nothing for spells and battlerager clan.
    if (skill->getSpell() && skill->getSpell()->isCasted() && ch->getClan() == clan_battlerager)
        return Register(false);

    // Create and save temporary skill data.
    PCSkillData &data = ch->getSkillData(skill->getIndex());
    data.origin = origin;
    data.start = today;
    data.end = end;
    data.learned = learned;

    return Register(true);
}

NMI_INVOKE( SkillWrapper, removeTemporary, "(ch[,origin]): очистить временное умение у персонажа, помеченное как origin (.tables.skill_origin_table). Вернет true, если было что очищать.")
{
    PCharacter *ch = argnum2player(args, 1);
    Skill *skill = getTarget();
    PCSkillData &data = ch->getSkillData(skill->getIndex());
    int origin;

    if (args.size() >= 2)
        origin = argnum2flag(args, 2, skill_origin_table);
    else
        origin = SKILL_FENIA;

    if (!data.isTemporary())
        return Register(false);
    if (data.origin != origin)
        return Register(false);

    data.clear();

    return Register(true);
}

NMI_INVOKE(SkillWrapper, apply, "(ch,vict|obj|room|arg,level): выполнить умение без проверок и сообщений")
{
    Skill *skill = getTarget();
    Character *ch = argnum2character(args, 1);

    if (skill->getSpell()) {
        SpellTarget::Pointer target = argnum2target(args, 2);
        int level = argnum2number(args, 3);
        skill->getSpell()->apply(ch, target, level);
    }
    else {
        Character *victim = args.size() > 1 ? argnum2character(args, 2) : 0;
        int level = args.size() > 2 ? argnum2number(args, 3) : 0;
        skill->getCommand()->apply(ch, victim, level);
    }

    return Register();
}

NMI_INVOKE(SkillWrapper, dressItem, "(obj,ch[,key]): рестрингнуть предмет согласно аттрибутам персонажа")
{
    Object *item = argnum2item(args, 1);
    Character *ch = argnum2character(args, 2);
    DLString key;
    if (args.size() > 2)
        key = argnum2string(args, 3);

    dress_created_item(getTarget()->getIndex(), item, ch, key);
    return Register();
}

/*----------------------------------------------------------------------
 * FeniaSkill
 *----------------------------------------------------------------------*/
NMI_INIT(FeniaSkill, "феневое умение");

FeniaSkill::FeniaSkill( const DLString &n )
                    : Skill(n)
{
    myname = n;
}

Scripting::Register FeniaSkill::wrap( const DLString &name )
{
    FeniaSkill::Pointer hw( NEW, name );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( hw );
    return Scripting::Register( sobj );
}

void FeniaSkill::setSelf(Scripting::Object *obj)
{
    if (obj) {
        if (!myname.empty()) {
            this->setName(myname);
            this->loaded();
            notice("FeniaSkill loaded: %s.", name.c_str());
        }
    } else {
        if (!myname.empty()) {
            this->setName(myname);
            if (this == skillManager->findExisting(name)) {
                this->unloaded();
                notice("FeniaSkill unloaded: %s.", name.c_str());
            }
        }
    }

    self = obj;
}

void FeniaSkill::backup()
{
    if (!name.empty() && this == skillManager->findExisting(name)) {
        this->unloaded();
        notice("FeniaSkill unloaded on backup: %s.", name.c_str());
    }
}

NMI_GET( FeniaSkill, name, "название умения" ) 
{ 
    return Register(name); 
} 

NMI_SET(FeniaSkill, nameRus, "русское название умения")
{
    nameRus.setValue(arg.toString());
    self->changed();
}

NMI_GET(FeniaSkill, nameRus, "русское название умения")
{
    return Register(nameRus);
}

NMI_SET(FeniaSkill, dammsg, "сообщение о повреждении с падежами через |")
{
    dammsg.setFullForm(arg.toString());
    self->changed();
}

NMI_GET(FeniaSkill, dammsg, "сообщение о повреждении с падежами через |")
{
    return Register(dammsg.getFullForm());
}

NMI_SET(FeniaSkill, dammsg_gender, "грам.род сообщения о повреждении (m, f, n, p)")
{
    dammsg.setGender(Grammar::MultiGender(arg.toString().c_str()));
    self->changed();
}

NMI_INVOKE(FeniaSkill, api, "(): печатает этот api")
{
    ostringstream buf;
    
    Scripting::traitsAPI<FeniaSkill>( buf );
    return Scripting::Register( buf.str( ) );
}


/*----------------------------------------------------------------------
 * SkillGroup
 *----------------------------------------------------------------------*/
NMI_INIT(SkillGroupWrapper, "skill group, группа умений");

SkillGroupWrapper::SkillGroupWrapper( const DLString &n )
                  : name( n )
{
}

Register SkillGroupWrapper::wrap(const DLString &name)
{
    SkillGroup *group = skillGroupManager->findExisting(name);
    if (!group)
        throw Scripting::Exception(name + ": skill group not found");
        
    return Register::handler<SkillGroupWrapper>(group->getName());    
}

SkillGroup * SkillGroupWrapper::getTarget() const
{
    SkillGroup *group = skillGroupManager->find(name);
    if (!group)
        throw Scripting::Exception(name + ": skill group no longer exists");
    return group;
}

NMI_INVOKE( SkillGroupWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<SkillGroupWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

NMI_GET( SkillGroupWrapper, name, "английское название" ) 
{
    return getTarget()->getName( );
}

NMI_GET( SkillGroupWrapper, nameRus, "русское название" ) 
{
    return getTarget()->getRussianName( );
}
