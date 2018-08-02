/* $Id: root.cpp,v 1.1.4.35.6.25 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "core/object.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "room.h"

#include "interprethandler.h"
#include "descriptor.h"
#include "wiznet.h"
#include "infonet.h"
#include "commonattributes.h"

#include "dreamland.h"
#include "weather.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"

#include "root.h"
#include "nannyhandler.h"
#include "nativeext.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "reglist.h"
#include "object.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "mobindexwrapper.h"
#include "structwrappers.h"
#include "objindexwrapper.h"
#include "wrappermanager.h"
#include "affectwrapper.h"
#include "tableswrapper.h"
#include "schedulerwrapper.h"
#include "commandwrapper.h"
#include "codesource.h"
#include "subr.h"
#include "fenia/handler.h"
#include "wrap_utils.h"

#include "def.h"

Profession * find_prof_unstrict( const DLString &className);

using namespace std;
using namespace Scripting;

NMI_INIT(Root, "корневой объект");

/*
 * METHODS
 */

NMI_INVOKE( Root, Map , "конструктор для структуры") 
{
    return Register::handler<IdContainer>();
}

NMI_INVOKE( Root, Array, "конструктор для массива") 
{
    return Register::handler<RegContainer>();
}

NMI_INVOKE( Root, List , "конструктор для списка") 
{
    return Register::handler<RegList>();
}

NMI_INVOKE( Root, Affect, "конструктор для аффекта" )
{
    if (args.empty( ))
	return Register::handler<AffectWrapper>( );
    else {
	AffectWrapper::Pointer aff( NEW, args );
	Scripting::Object *obj = &Scripting::Object::manager->allocate( );

	obj->setHandler( aff );
	return Register( obj );
    }
}

NMI_INVOKE( Root, Skill, "регистрирует скил-пустышку с заданным именем, скил просуществует до ребута" )
{
    DLString skillName;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    skillName = args.front( ).toString( );

    if (skillManager->findExisting( skillName ))
	throw Scripting::CustomException( "Skill already exists" );

    skillManager->lookup( skillName );
    return Register( );
}

DLString regfmt(Character *to, const RegisterList &argv);

NMI_INVOKE( Root, fmt, "отформатировать строку") 
{
    return regfmt( NULL, args );
}

NMI_INVOKE( Root, print , "вывести строку в системные логи") 
{
    LogStream::sendNotice() << ">> " << args.front().toString() << endl;
    return Register();
}

NMI_GET( Root, current_time, "текущее время в секундах") 
{
    return Register((int)dreamland->getCurrentTime( ));
}

NMI_INVOKE( Root, getCurrentTime , "текущее время в секундах") 
{
    return Register((int)dreamland->getCurrentTime( ));
}

NMI_INVOKE( Root, player_exists, "существует ли в мире игрок с данным именем")
{
    return Register( PCharacterManager::find( args2string( args ) ) != NULL );
}

NMI_INVOKE( Root, player_name, "английское имя игрока по его русскому/английскому имени")
{
    PCMemoryInterface *pci = PCharacterManager::find( args2string( args ) );
    
    if (pci)
	return pci->getName( );
    else
	return DLString::emptyString;
}

NMI_INVOKE( Root, player_russian_name, "русское имя игрока с падежами по его русскому/английскому имени")
{
    PCMemoryInterface *pci = PCharacterManager::find( args2string( args ) );
    
    if (pci)
	return pci->getRussianName( ).getFullForm( );
    else
	return DLString::emptyString;
}


NMI_INVOKE( Root, player_clan, "")
{
    PCMemoryInterface *pci = PCharacterManager::find( args2string( args ) );
    
    if (pci)
	return pci->getClan( )->getName( );
    else
	return DLString::emptyString;
}

NMI_INVOKE( Root, player_attribute, "значение данного аттрибута игрока")
{
    if (args.size( ) != 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    PCMemoryInterface *pci = PCharacterManager::find( args2string( args ) );
    DLString attrName = args.back( ).toString( );

    if (!pci)
	throw Scripting::CustomException( "Player not found" );

    XMLStringAttribute::Pointer sAttr = pci->getAttributes( ).findAttr<XMLStringAttribute>( attrName );

    if (sAttr)
	return sAttr->getValue( );
    else
	return DLString::emptyString;
}

NMI_INVOKE( Root, get_obj_world , "ищет в мире предмет с указанным именем")
{
    ::Object *obj;
    const char *name = args.front( ).toString( ).c_str( );
    
    for (obj = object_list; obj; obj = obj->next)
	if (is_name( name, obj->getName( )))
	    return WrapperManager::getThis( )->getWrapper(obj); 

    return Register( );
}

NMI_INVOKE( Root, get_char_world , "ищет в мире чара с указанным именем")
{
    Character *wch;
    const char *name = args.front( ).toString( ).c_str( );
    
    for (wch = char_list; wch; wch = wch->next) 
	if (is_name( name, wch->getNameP( ) ))
	    return WrapperManager::getThis( )->getWrapper(wch); 

    return Register( );
}

NMI_INVOKE( Root, get_mob_index , "возвращает прототип моба с заданным vnum")
{
    int vnum;
    MOB_INDEX_DATA *pIndex;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
	
    vnum = args.front( ).toNumber( );
    pIndex = ::get_mob_index( vnum );
    
    return WrapperManager::getThis( )->getWrapper( pIndex );
}

NMI_INVOKE( Root, get_obj_index , "возвращает прототип предмета с заданным vnum")
{
    int vnum;
    OBJ_INDEX_DATA *pIndex;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
	
    vnum = args.front( ).toNumber( );
    pIndex = ::get_obj_index( vnum );
    
    return WrapperManager::getThis( )->getWrapper( pIndex );
}

NMI_INVOKE( Root, get_room_index , "возвращает комнату с заданным vnum")
{
    int vnum;
    Room *room;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
	
    vnum = args.front( ).toNumber( );
    room = ::get_room_index( vnum );
    
    return WrapperManager::getThis( )->getWrapper( room ); 
}

NMI_INVOKE( Root, min, "минимальное из двух чисел") 
{
    if (args.size( ) != 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::min(args.front( ).toNumber( ), args.back( ).toNumber( )) );
}

NMI_INVOKE( Root, max, "максимальное из двух чисел") 
{
    if (args.size( ) != 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::max(args.front( ).toNumber( ), args.back( ).toNumber( )) );
}

NMI_INVOKE( Root, abs, "модуль числа") 
{
    int x;

    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    x = args.front( ).toNumber( );
    return ::abs( x );
}

NMI_INVOKE( Root, dice, "(x, y) x раз кинуть кубик с y гранями") 
{
    RegisterList::const_iterator i;
    int a, b;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    b = i->toNumber( );

    return Register( ::dice( a, b ) );
}

NMI_INVOKE( Root, number_range , "(x, y) произвольное число в промежутке от x до y") 
{
    RegisterList::const_iterator i;
    int a, b;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    b = i->toNumber( );

    return Register( ::number_range( a, b ) );
}

NMI_INVOKE( Root, number_percent , "произвольное число от 1 до 100") 
{
    return Register( ::number_percent( ) );
}

NMI_INVOKE( Root, chance , "(x) true если x < .number_percent()") 
{
    int a;

    if (args.size( ) < 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    a = args.front( ).toNumber( );
    return Register( ::chance( a ) );
}

NMI_INVOKE( Root, chanceOneOf, "(x) true если .number_range(1, x) == 1") 
{
    if (args.size( ) < 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::number_range( 1, args.front( ).toNumber( ) ) == 1);
}

NMI_INVOKE( Root, set_bit, "(mask, b) вернет mask с установленными битом b (логическое 'или')") 
{
    RegisterList::const_iterator i;
    int a, bit;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    bit = i->toNumber( );

    return a | bit;
}

NMI_INVOKE( Root, unset_bit, "(mask, b) вернет mask со сброшенным битом b") 
{
    RegisterList::const_iterator i;
    int a, bit;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    bit = i->toNumber( );

    return a & ~bit;
}

NMI_INVOKE( Root, isset_bit, "(mask, b) true если бит b установлен в mask (логическое 'и')") 
{
    RegisterList::const_iterator i;
    int a, bit;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    bit = i->toNumber( );

    return a & bit;
}

NMI_INVOKE( Root, eval , "выполнить выражение указанное в строке") 
{
    if (args.empty())
	throw Scripting::NotEnoughArgumentsException( );
    
    const DLString &src = args.front().toString();
    Scripting::CodeSource &cs = Scripting::CodeSource::manager->allocate();
    
    cs.author = "<unknown>";
    cs.name = "<recursive eval>";

    cs.content = src;

    return cs.eval(Register( ));
}

inline bool 
delim(char c)
{
    return c == ' ' || c == '-';
}

NMI_INVOKE( Root, makeShort , "конструирует строку с палками из шести строк с падежами")
{
    if(args.size() != 6)
	throw Scripting::NotEnoughArgumentsException( );
   
    char strings[6][MAX_INPUT_LENGTH];
    char *cssp[6];
    
    int i;
    RegisterList::const_iterator ipos;
    
    for(i=0, ipos = args.begin();ipos != args.end();i++, ipos++) {
        strcpy(strings[i], ipos->toString().c_str());
	cssp[i] = strings[i];
    }

    DLString rc;
    
    for(;;) {
	/*skip spacess*/
	while(*cssp[0] && delim(*cssp[0]))
	    rc += *cssp[0]++;
	
	for(i=1;i<6;i++)
	    while(*cssp[i] && delim(*cssp[i]))
		cssp[i]++;

	/*check done*/
	for(i=0;i<6;i++)
	    if(*cssp[i])
		break;
	
	if(i == 6)
	    break;

	/*copy matches*/
	for(;;) {
	    for(i=0;i<5;i++)
		if(!*cssp[i] || !*cssp[i+1] || *cssp[i] != *cssp[i+1])
		    break;
		    
	    if(i < 5)
		break;

	    rc += *cssp[0];
	    
	    for(i=0;i<6;i++)
		cssp[i]++;
	}
	
	/*copy difference*/
	for(i=0;i<6;i++) {
	    rc += '|';
	    for(;*cssp[i] && !delim(*cssp[i]);cssp[i]++)
		rc += *cssp[i];
	}
    }

    return rc;
}


NMI_INVOKE(Root, get_random_room, "произвольная комната из числа общедоступных" )
{
    std::vector<Room *> rooms;
    Room *r;
    
    for (r = room_list; r; r = r->rnext)
	if (r->isCommon() && !r->isPrivate())
	    rooms.push_back(r);

    if (rooms.empty())
	return Register( );
    else {
	r = rooms[::number_range(0, rooms.size() - 1)];
	return WrapperManager::getThis( )->getWrapper(r); 
    }
}

NMI_INVOKE(Root, date, "строка с датой, как ее видно по команде time" )
{
    ostringstream buf;

    make_date( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE(Root, api, "печатает этот API" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<Root>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE(Root, gecho, "сообщение всем" )
{
    Descriptor *d;

    if (args.empty())
	throw Scripting::NotEnoughArgumentsException( );
    
    DLString txt = args.front().toString() + "\r\n";
    
    for (d = descriptor_list; d != 0; d = d->next)
	if (d->connected == CON_PLAYING && d->character)
	    d->character->send_to( txt.c_str( ) );
    
    return Register( );
}

NMI_INVOKE(Root, infonet, "сообщение по infonet" )
{
    if (args.size() != 2)
	throw Scripting::NotEnoughArgumentsException( );

    ::infonet( args.front( ).toString( ).c_str( ),
               wrapper_cast<CharacterWrapper>(args.back( ))->getTarget( ),
	       0 );
    return Register( );
}

NMI_INVOKE(Root, wiznet, "сообщение по wiznet" )
{
    DLString msg;
    int trust = 0, wiztype = WIZ_QUEST, wiznum;
    RegisterList::const_iterator i;
    
    if (args.empty())
	throw Scripting::NotEnoughArgumentsException( );
	
    i = args.begin( );
    msg = i->toString( );
    
    if (++i != args.end( )) {
	trust = i->toNumber( );
	
	if (++i != args.end( )) {
	    if (( wiznum = wiznet_lookup( i->toString( ).c_str( ) ) ) == -1)
		throw Scripting::Exception( "Unknown wiznet type" );
	    else
		wiztype = wiznet_table[wiznum].flag;
	}
    }
    
    ::wiznet( wiztype, 0, trust, args.front( ).toString( ).c_str( ) );
    return Register( );
}

NMI_INVOKE(Root, sync, "(системное) test for objects sync")
{
    while(!Scripting::Object::manager->sync(0))
	;
    return Register( );
}


NMI_INVOKE(Root, object, "(cистемное) поиск феневого объекта" )
{
    Scripting::Object::id_t id;

    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    id = args.front( ).toNumber( );
    Scripting::Object::Map::iterator i = Scripting::Object::manager->find( id );

    if (i == Scripting::Object::manager->end( ))
       return Register( );

    return Register(&*i);
}

#if 0
NMI_INVOKE( Root, clearGuts, "(системное) удаление феневого объекта" )
{
    GutsContainer *gc;
    
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );
    
    gc = wrapper_cast<GutsContainer>( args.front( ) );
    gc->extract( true );
    return Register( );
}
#endif

/*
 * FIELDS
 */
NMI_GET( Root, buildplot, "true для мира-стройплощадки")
{
    return dreamland->hasOption( DL_BUILDPLOT );
}

NMI_GET( Root, object_list , "список всех предметов, поле предмета next указывает на следующий") 
{
    return WrapperManager::getThis( )->getWrapper(object_list); 
}

extern Room *room_list;

NMI_GET( Root, room_list , "список всех комнат, поле комнаты rnext указывает на следующую") 
{
    return WrapperManager::getThis( )->getWrapper(room_list); 
}

NMI_GET( Root, char_list , "список всех чаров, поле чара next указывает на следующего") 
{
    return WrapperManager::getThis( )->getWrapper(char_list); 
}

NMI_GET( Root, hour , "текущий час суток, 0..23") 
{
    return Register( time_info.hour ); 
}

NMI_GET( Root, day, "текущий день недели, 0..6") 
{
    return Register( time_info.day); 
}

NMI_GET( Root, year, "текущий год") 
{
    return Register( time_info.year); 
}

NMI_GET( Root, month, "текущий месяц, 0..16" ) 
{
    return Register( time_info.month ); 
}

NMI_GET( Root, sunlight , "время суток: 0=ночь, 1=рассвет, 2=день, 3=закат") 
{
    return Register( weather_info.sunlight ); 
}

NMI_GET( Root, sky , "текущая погода: 0=безоблачно, 1=облачно, 2=дождь, 3=молнии") 
{
    return Register( weather_info.sky ); 
}

NMI_SET( Root, tmp, "") {
    this->tmp = arg;
    self->changed();
}
NMI_GET( Root, tmp, "") {
    return tmp;
}
    
NMI_GET( Root, scheduler , "объект-планировщик")
{
    if(scheduler.type == Register::NONE) {
	scheduler = Register::handler<SchedulerWrapper>();
	self->changed();
    }

    return scheduler;
}

NMI_GET( Root, tables, "доступ ко всем таблицам" )
{
    if(tables.type == Register::NONE) {
	tables = Register::handler<TablesWrapper>();
	self->changed();
    }

    return tables;
}

NMI_GET( Root, nanny, "" )
{
    if(nanny.type == Register::NONE) {
	nanny = Register::handler<NannyHandler>();
	self->changed();
    }

    return nanny;
}

NMI_GET( Root, hometowns, "список всех хометаунов") 
{
    RegList::Pointer list(NEW);
    Hometown *ht;
    
    for (int i = 0; i < hometownManager->size( ); i++) {
	ht = hometownManager->find( i );

	if (ht->isValid( )) 
	    list->push_back( HometownWrapper::wrap( ht->getName( ) ) );
    }
    
    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

NMI_INVOKE( Root, Hometown, "конструктор для хометауна по имени" )
{
    DLString name;

    if (args.empty( ))
	name = "none";
    else
	name = args.front( ).toString( );
	
    return HometownWrapper::wrap( name );
}

NMI_INVOKE( Root, Area, "конструктор для зоны по имени файла" )
{
    DLString name;

    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    name = args.front( ).toString( );
	
    return AreaWrapper::wrap( name );
}

NMI_INVOKE( Root, find_profession, "нестрогий поиск профессии по русскому или англ названию" )
{
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    DLString profName = args.front( ).toString( );
    Profession *prof = find_prof_unstrict( profName );
    if (!prof)
	throw Scripting::IllegalArgumentException( );

    return ProfessionWrapper::wrap( prof->getName( ) );
}

NMI_GET( Root, professions, "список всех профессий, доступных игрокам") 
{
    RegList::Pointer list(NEW);
    Profession *prof;
    
    for (int i = 0; i < professionManager->size( ); i++) {
	prof = professionManager->find( i );

	if (prof->isValid( ) && prof->isPlayed( )) 
	    list->push_back( ProfessionWrapper::wrap( prof->getName( ) ) );
    }
    
    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

NMI_INVOKE( Root, Profession, "конструктор для профессии по имени" )
{
    DLString name;

    if (args.empty( ))
	name = "none";
    else
	name = args.front( ).toString( );
	
    return ProfessionWrapper::wrap( name );
}

NMI_GET( Root, races, "список всех рас") 
{
    RegList::Pointer list(NEW);
    Race *race;
    
    for (int i = 0; i < raceManager->size( ); i++) {
	race = raceManager->find( i );

	if (race->isValid( )) 
	    list->push_back( RaceWrapper::wrap( race->getName( ) ) );
    }
    
    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

NMI_GET( Root, pcraces, "список рас, доступных игрокам") 
{
    RegList::Pointer list(NEW);
    Race *race;
    
    for (int i = 0; i < raceManager->size( ); i++) {
	race = raceManager->find( i );

	if (race->isValid( ) && race->isPC( )) 
	    list->push_back( RaceWrapper::wrap( race->getName( ) ) );
    }
    
    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

NMI_INVOKE( Root, Race, "конструктор для расы по имени" )
{
    DLString name;

    if (args.empty( ))
	name = "none";
    else
	name = args.front( ).toString( );
	
    return RaceWrapper::wrap( name );
}

NMI_INVOKE( Root, findPlayer, "поиск игрока по точному имени" )
{
    DLString name;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    name = args.front( ).toString( );

    return FeniaManager::wrapperManager->getWrapper( 
		PCharacterManager::findPlayer( name ) ); 
}

NMI_INVOKE( Root, Liquid, "конструктор для жидкости по имени" )
{
    DLString name;

    if (!args.empty( )) {
	Register arg = args.front( );
	
	if (arg.type == Register::NUMBER)
	    name = liquidManager->getName( arg.toNumber( ) );
	else 
	    name = arg.toString( );
    }
    
    return LiquidWrapper::wrap( name.empty( ) ? "none" : name );
}

NMI_INVOKE( Root, Clan, "конструктор для клана по имени" )
{
    DLString name;

    if (args.empty( ))
	name = "none";
    else
	name = args.front( ).toString( );
	
    return ClanWrapper::wrap( name );
}

NMI_INVOKE( Root, Command, "конструктор для команды" )
{
    return Register::handler<CommandWrapper>();
}

NMI_GET( Root, players, "список (List) всех игроков") 
{
    Descriptor *d;
    RegList::Pointer list(NEW);

    for (d = descriptor_list; d != 0; d = d->next)
	if (d->connected == CON_PLAYING && d->character)
	    list->push_back( wrap( d->character->getPC( ) ) );

    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

NMI_GET( Root, feniadbStats, "статистика базы данных скриптовых объектов")
{
    return Register(Scripting::Object::manager->stats());
}

NMI_INVOKE( Root, repr, "" )
{
    return Register(args.front().repr());
}

NMI_INVOKE( Root, obj_by_id, "" )
{
    return Register(&Scripting::Object::manager->at(args.front().toNumber()));
}

