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
#include "messengers.h"
#include "commonattributes.h"
#include "subprofession.h"
#include "language.h"
#include "languagemanager.h"
#include "websocketrpc.h"
#include "dreamland.h"
#include "weather.h"
#include "move_utils.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "damageflags.h"
#include "../anatolia/handler.h"

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

static int check_range(const Register &arg, int min, int max)
{
    int n = arg.toNumber();
    if (n < min || n > max)
        throw Scripting::IllegalArgumentException();
    return n;
}

/*
 * METHODS
 */

NMI_INVOKE( Root, Map , "(): конструктор для структуры") 
{
    return Register::handler<IdContainer>();
}

NMI_INVOKE( Root, Array, "(): конструктор для массива") 
{
    return Register::handler<RegContainer>();
}

NMI_INVOKE( Root, List , "(): конструктор для списка") 
{
    return Register::handler<RegList>();
}

NMI_INVOKE( Root, Affect, "([skill[,level,duration,location,mod,where,bits]]): конструктор для аффекта умения skill или пустого" )
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

DLString regfmt(Character *to, const RegisterList &argv);

NMI_INVOKE( Root, fmt, "(args): отформатировать строку, см. статью вики про функции вывода") 
{
    return regfmt( NULL, args );
}

NMI_INVOKE( Root, print , "(msg): вывести строку msg в системные логи") 
{
    LogStream::sendNotice() << ">> " << args.front().toString() << endl;
    return Register();
}

NMI_GET( Root, current_time, "текущее время в секундах") 
{
    return Register((int)dreamland->getCurrentTime( ));
}

NMI_INVOKE( Root, getCurrentTime , "(): текущее время в секундах") 
{
    return Register((int)dreamland->getCurrentTime( ));
}

NMI_GET( Root, world_time, "внутримировое время в секундах") 
{
    return Register((int)dreamland->getWorldTime( ));
}

NMI_SET( Root, world_time, "внутримировое время в секундах") 
{
    dreamland->setWorldTime(arg.toNumber());
}

NMI_INVOKE( Root, saveConfig, "(): сохранить конфигурацию DreamLand на диск")
{
    try {
        dreamland->save(false);
    } catch (const ::Exception &e) {
        return Register(e.what());
    }
    return Register();
}

NMI_INVOKE( Root, loadConfig, "(): считать конфигурацию DreamLand с диска")
{
    try {
        dreamland->load(false);
    } catch (const ::Exception &e) {
        return Register(e.what());
    }
    return Register();
}

NMI_INVOKE( Root, player_exists, "(name): существует ли в мире игрок с данным именем")
{
    return Register( PCharacterManager::find( args2string( args ) ) != NULL );
}

NMI_INVOKE( Root, player_name, "(name): английское имя игрока по его русскому/английскому имени")
{
    PCMemoryInterface *pci = PCharacterManager::find( args2string( args ) );
    
    if (pci)
        return pci->getName( );
    else
        return DLString::emptyString;
}

NMI_INVOKE( Root, player_russian_name, "(name): русское имя игрока с падежами по его русскому/английскому имени")
{
    PCMemoryInterface *pci = PCharacterManager::find( args2string( args ) );
    
    if (pci)
        return pci->getRussianName( ).getFullForm( );
    else
        return DLString::emptyString;
}


NMI_INVOKE( Root, player_clan, "(name): название клана игрока по его имени")
{
    PCMemoryInterface *pci = PCharacterManager::find( args2string( args ) );
    
    if (pci)
        return pci->getClan( )->getName( );
    else
        return DLString::emptyString;
}

NMI_INVOKE( Root, player_attribute, "(playerName, attrName): значение данного аттрибута игрока")
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

NMI_INVOKE( Root, get_obj_world , "(name): ищет в мире предмет с указанным именем")
{
    ::Object *obj;
    const char *name = args.front( ).toString( ).c_str( );
    
    for (obj = object_list; obj; obj = obj->next)
        if (is_name( name, obj->getName( )))
            return WrapperManager::getThis( )->getWrapper(obj); 

    return Register( );
}

NMI_INVOKE( Root, get_obj_world_unique , "(vnum, ch): ищет в мире предмет с этим внумом, принадлежащий ch")
{
    int vnum = argnum2number(args, 1);
    Character *ch = argnum2character(args, 2);
    ::Object *obj = get_obj_world_unique(vnum, ch);
    return WrapperManager::getThis()->getWrapper(obj);
}

NMI_INVOKE( Root, get_char_world , "(name): ищет в мире чара с указанным именем")
{
    Character *wch;
    const char *name = args.front( ).toString( ).c_str( );
    
    for (wch = char_list; wch; wch = wch->next) 
        if (is_name( name, wch->getNameP( ) ))
            return WrapperManager::getThis( )->getWrapper(wch); 

    return Register( );
}

NMI_INVOKE( Root, get_mob_index , "(vnum): возвращает прототип моба с заданным vnum")
{
    int vnum;
    MOB_INDEX_DATA *pIndex;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
        
    vnum = args.front( ).toNumber( );
    pIndex = ::get_mob_index( vnum );
    
    return WrapperManager::getThis( )->getWrapper( pIndex );
}

NMI_INVOKE( Root, get_obj_index , "(vnum): возвращает прототип предмета с заданным vnum")
{
    int vnum;
    OBJ_INDEX_DATA *pIndex;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
        
    vnum = args.front( ).toNumber( );
    pIndex = ::get_obj_index( vnum );
    
    return WrapperManager::getThis( )->getWrapper( pIndex );
}

NMI_INVOKE( Root, get_room_index , "(vnum): возвращает комнату с заданным vnum")
{
    int vnum;
    Room *room;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
        
    vnum = args.front( ).toNumber( );
    room = ::get_room_index( vnum );
    
    return WrapperManager::getThis( )->getWrapper( room ); 
}

NMI_INVOKE( Root, min, "(a, b): минимальное из двух чисел a и b") 
{
    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::min(args.front( ).toNumber( ), args.back( ).toNumber( )) );
}

NMI_INVOKE( Root, max, "(a, b): максимальное из двух чисел a и b") 
{
    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::max(args.front( ).toNumber( ), args.back( ).toNumber( )) );
}

NMI_INVOKE( Root, abs, "(n): модуль числа n") 
{
    int x;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    x = args.front( ).toNumber( );
    return ::abs( x );
}

NMI_INVOKE( Root, dice, "(x, y): x раз кинуть кубик с y гранями") 
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

NMI_INVOKE( Root, number_range , "(x, y): произвольное число в промежутке от x до y") 
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

NMI_INVOKE( Root, number_percent , "(): произвольное число от 1 до 100") 
{
    return Register( ::number_percent( ) );
}

NMI_INVOKE( Root, chance , "(x): true если x < .number_percent()") 
{
    int a;

    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    a = args.front( ).toNumber( );
    return Register( ::chance( a ) );
}

NMI_INVOKE( Root, chanceOneOf, "(x): true если .number_range(1, x) == 1") 
{
    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::number_range( 1, args.front( ).toNumber( ) ) == 1);
}

NMI_INVOKE( Root, set_bit, "(mask, b): вернет логическое 'или' между mask и b") 
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

NMI_INVOKE( Root, set_bit_number, "(mask, n): вернет mask с установленными битом под номером n (т.е. mask | 1<<n)") 
{
    int mask = argnum2number(args, 1);
    int n = argnum2number(args, 2);
    check_range(n, 0, 31);
    return mask | (1<<n);
}

NMI_INVOKE( Root, unset_bit, "(mask, b): вернет mask со сброшенными битами из b") 
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

NMI_INVOKE( Root, isset_bit, "(mask, b): вернет логическое 'и' между mask и b") 
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

NMI_INVOKE( Root, eval , "(expr): выполнить феневое выражение expr") 
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

NMI_INVOKE(Root, create_money, "(gold, silver): создает объект-деньги указанной стоимости")
{
    int gold = argnum2number(args, 1);
    int silver = argnum2number(args, 2);	
    return wrap(
		create_money(gold, silver));
}

inline bool 
delim(char c)
{
    return c == ' ' || c == '-';
}

NMI_INVOKE( Root, makeShort , "(s1,s2,...,s6): конструирует строку с палками из шести строк с падежами")
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


NMI_INVOKE(Root, get_random_room, "(): произвольная комната из числа общедоступных" )
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

NMI_INVOKE(Root, get_random_room_vanish, "(ch): произвольная комната, куда разрешен vanish персонажу ch" )
{
    Character *ch = args2character(args);
    Room *r =  get_random_room_vanish(ch);
    if (!r)
        throw Scripting::Exception("No suitable room found for vanish");
        
    return WrapperManager::getThis( )->getWrapper(r); 
}

NMI_INVOKE(Root, date, "(): строка с датой, как ее видно по команде time" )
{
    ostringstream buf;

    make_date( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE(Root, api, "(): печатает этот API" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<Root>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE(Root, gecho, "(msg): выдать сообщение msg всем играющим" )
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

NMI_INVOKE(Root, discord, "(msg): послать сообщение в чат Discord от имени бота Хрустальный Шар")
{
    DLString msg = args2string(args);
    send_discord_orb(msg);
    return Register( );
}

NMI_INVOKE(Root, infonet, "(msg): выдать сообщение msg через хрустальный шар" )
{
    if (args.size() != 2)
        throw Scripting::NotEnoughArgumentsException( );

    ::infonet( args.front( ).toString( ).c_str( ),
               wrapper_cast<CharacterWrapper>(args.back( ))->getTarget( ),
               0 );
    return Register( );
}

NMI_INVOKE(Root, wiznet, "(msg): выдать сообщение msg по wiznet" )
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

NMI_INVOKE(Root, sync, "(): test for objects sync (системное)")
{
    while(!Scripting::Object::manager->sync(0))
        ;
    return Register( );
}


NMI_INVOKE(Root, object, "(id): поиск феневого объекта по ID (cистемное)" )
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

NMI_GET( Root, obj_index_list, "список (List) всех прототипов предметов") 
{
    RegList::Pointer list(NEW);
    
    for (int iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (OBJ_INDEX_DATA *pObj = obj_index_hash[iHash]; pObj; pObj = pObj->next)
            list->push_back(wrap(pObj)); 
    
    Scripting::Object *listObj = &Scripting::Object::manager->allocate();
    listObj->setHandler(list);
    return Register(listObj);
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

NMI_GET( Root, mob_index_list, "список (List) всех прототипов мобов") 
{
    RegList::Pointer list(NEW);
    
    for (int iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[iHash]; pMob; pMob = pMob->next)
            list->push_back(wrap(pMob)); 
    
    Scripting::Object *listObj = &Scripting::Object::manager->allocate();
    listObj->setHandler(list);
    return Register(listObj);
}

NMI_SET( Root, hour , "текущий час суток, 0..23") 
{
    time_info.hour = check_range(arg, 0, 23);
}

NMI_SET( Root, day, "текущий день месяца, 0..34") 
{
    time_info.day = check_range(arg, 0, 34);
}

NMI_SET( Root, year, "текущий год") 
{
    time_info.year = check_range(arg, 0, 700);
}

NMI_SET( Root, month, "текущий месяц, 0..16" ) 
{
    time_info.month = check_range(arg, 0, 16);
}

NMI_GET( Root, hour , "текущий час суток, 0..23") 
{
    return Register( time_info.hour ); 
}

NMI_GET( Root, day, "текущий день месяца, 0..34") 
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

NMI_SET( Root, sunlight , "время суток: 0=ночь, 1=рассвет, 2=день, 3=закат") 
{
    weather_info.sunlight = check_range(arg, 0, 3);
}

NMI_GET( Root, sunlight , "время суток: 0=ночь, 1=рассвет, 2=день, 3=закат") 
{
    return Register( weather_info.sunlight ); 
}

NMI_GET( Root, sky , "текущая погода: 0=безоблачно, 1=облачно, 2=дождь, 3=молнии") 
{
    return Register( weather_info.sky ); 
}

NMI_SET( Root, sky , "текущая погода: 0=безоблачно, 1=облачно, 2=дождь, 3=молнии") 
{
    weather_info.sky = check_range(arg, 0, 3);
}

NMI_SET( Root, tmp, "структура где можно хранить глобальные переменные") {
    this->tmp = arg;
    self->changed();
}
NMI_GET( Root, tmp, "структура где можно хранить глобальные переменные") {
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

NMI_GET( Root, nanny, "доступ к методам 'няни', для процесса создания персонажа" )
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

NMI_INVOKE( Root, Hometown, "(name): конструктор для хометауна по имени" )
{
    DLString name;

    if (args.empty( ))
        name = "none";
    else
        name = args.front( ).toString( );
        
    return HometownWrapper::wrap( name );
}

NMI_INVOKE( Root, Area, "(filename): конструктор для зоны по имени файла" )
{
    DLString name;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    name = args.front( ).toString( );
        
    return AreaWrapper::wrap( name );
}

NMI_INVOKE( Root, find_profession, "(name): нестрогий поиск профессии по русскому или англ названию" )
{
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    DLString profName = args.front( ).toString( );
    Profession *prof = find_prof_unstrict( profName );
    if (!prof)
        throw Scripting::IllegalArgumentException( );

    return Register::handler<ProfessionWrapper>(prof->getName());
}

NMI_GET( Root, professions, "список всех профессий, доступных игрокам") 
{
    RegList::Pointer list(NEW);
    Profession *prof;
    
    for (int i = 0; i < professionManager->size( ); i++) {
        prof = professionManager->find( i );

        if (prof->isValid( ) && prof->isPlayed( )) 
            list->push_back( Register::handler<ProfessionWrapper>(prof->getName()) );
    }
    
    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

NMI_INVOKE( Root, Profession, "(name): конструктор для профессии (класса) по имени" )
{
    DLString name = args2string(args);
    Profession *prof = professionManager->findExisting(name);
    if (!prof)
        throw Scripting::Exception("Profession not found");
    return Register::handler<ProfessionWrapper>(prof->getName());
}

NMI_INVOKE( Root, CraftProfession, "(name): конструктор для дополнительной профессии по имени" )
{
    DLString name = args2string(args);
    CraftProfession::Pointer prof = craftProfessionManager->get(name);
    if (!prof)
        throw Scripting::Exception("Craft profession not found");
    return Register::handler<CraftProfessionWrapper>(prof->getName());
}

NMI_INVOKE( Root, Bonus, "(name): конструктор для бонусов по имени" )
{
    DLString name = args2string(args);
    Bonus *bonus = bonusManager->findExisting(name);
    if (!bonus)
        throw Scripting::Exception("Bonus not found");
    return Register::handler<BonusWrapper>(bonus->getName());
}

NMI_INVOKE( Root, Religion, "(name): конструктор для религии по имени" )
{
    DLString name = args2string(args);
    Religion *religion = religionManager->findExisting(name);
    if (!religion)
        throw Scripting::Exception("Religion not found");
    return Register::handler<ReligionWrapper>(religion->getName());
}

NMI_INVOKE( Root, Language, "(name): конструктор для древнего языка по имени" )
{
    DLString name = args2string(args);
    Language::Pointer lang = languageManager->findLanguage(name);
    if (!lang)
        throw Scripting::Exception("Language not found");
    return Register::handler<LanguageWrapper>(lang->getName());
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

NMI_INVOKE( Root, Race, "(name): конструктор для расы по имени" )
{
    DLString name;

    if (args.empty( ))
        name = "none";
    else
        name = args.front( ).toString( );
        
    return RaceWrapper::wrap( name );
}

NMI_INVOKE( Root, findPlayer, "(name): поиск игрока по точному имени" )
{
    DLString name;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    name = args.front( ).toString( );

    return FeniaManager::wrapperManager->getWrapper( 
                PCharacterManager::findPlayer( name ) ); 
}

NMI_INVOKE( Root, Liquid, "(name): конструктор для жидкости по имени" )
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

static bool normalize_skill_name(DLString &arg)
{
    arg.toLower().stripWhiteSpace();
    if (arg.empty())
        return false;

    for (DLString::size_type s = 0; s < arg.length(); s++)
        if (!isalpha(arg.at(s)) && arg.at(s) != ' ')
            return false;

    return true;
}

NMI_INVOKE( Root, Skill, "(name): конструктор для умения по имени" )
{
    Skill *skill = argnum2skill(args, 1);
    return Register::handler<SkillWrapper>(skill->getName());    
}

NMI_INVOKE( Root, FeniaSkill, "(name): конструктор для нового умения" )
{
    DLString name = args2string(args);
    Skill *skill = skillManager->findExisting(name);

    if (skill && skill->isValid())
        throw Scripting::CustomException(name + ": skill already exists.");

    if (!normalize_skill_name(name))
        throw Scripting::Exception("Skill name can only consist of letters and spaces");

    return FeniaSkill::wrap(name);
}

NMI_INVOKE( Root, Clan, "(name): конструктор для клана по имени" )
{
    DLString name;

    if (args.empty( ))
        name = "none";
    else
        name = args.front( ).toString( );
        
    return ClanWrapper::wrap( name );
}

NMI_INVOKE( Root, Command, "(): конструктор для команды" )
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

NMI_INVOKE( Root, repr, "(obj): попытка привести феневый объект obj к строке" )
{
    return Register(args.front().repr());
}

NMI_INVOKE( Root, obj_by_id, "(id): найти феневый объект по уникальному идентификатору" )
{
    return Register(&Scripting::Object::manager->at(args.front().toNumber()));
}

NMI_INVOKE(Root, codesource, "(func): номер сценария, в котором объявлена данная функция")
{
    Register reg = argnum2function(args, 1);
    Scripting::CodeSourceRef csRef = reg.toFunction()->getFunction()->source;
    return Register((int)csRef.source->getId());
}

NMI_INVOKE(Root, apply, "(func, this, args): вызвать func с указанным this и списком аргументов args")
{    
    Register func = argnum2function(args, 1);

    Register thiz;
    if (args.size() > 1)
        thiz = argnum(args, 2);

    RegisterList registerList;

    if (args.size() > 2) {
        Register params = argnum(args, 3);
        RegList::Pointer regList = params.toHandler().getDynamicPointer<RegList>();

        for (RegList::const_iterator r = regList->begin(); r != regList->end(); r++)
            registerList.push_back(*r);
    }

    return func.toFunction()->invoke(thiz, registerList);
}

NMI_INVOKE(Root, webcmd, "(ch,cmd,label): создать линку для веб-клиента, выглядящую как label и выполняющую по клику команду cmd")
{
    Character *ch = argnum2character(args, 1);
    DLString cmd = argnum2string(args, 2);
    DLString seeFmt = argnum2string(args, 3);

    return Register(web_cmd(ch, cmd, seeFmt));
}

NMI_INVOKE(Root, spells, "(targets): вернуть все заклинания, действующие на цели (.tables.target_table)")
{
    int targets = argnum2flag(args, 1, target_table);
    RegList::Pointer spells(NEW);

    for (int sn = 0; sn < skillManager->size( ); sn++) {
        Skill *skill = skillManager->find(sn);
        Spell::Pointer spell = skill->getSpell();

        if (!spell || !spell->isCasted())
            continue;

        if (targets > 0 && !IS_SET(spell->getTarget(), targets))
            continue;

        spells->push_back(Register(skill->getName()));
    }

    Scripting::Object *listObj = &Scripting::Object::manager->allocate();
    listObj->setHandler(spells);
    return Register(listObj);
}