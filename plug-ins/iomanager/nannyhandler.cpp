/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "descriptor.h"
#include "colour.h"
#include "mudtags.h"
#include "codepage.h"
#include "defaultbufferhandler.h"
#include "interprethandler.h"
#include "ban.h"
#include "descriptorstatemanager.h"
#include "comm.h"
#include "badnames.h"
#include "xmlattributecoder.h"
#include "serversocketcontainer.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "object.h"

#include "dreamland.h"
#include "alignment.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"

#include "nannyhandler.h"
#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "subr.h"
#include "native.h"

#include "def.h"

HOMETOWN(frigate);

using namespace Scripting;
NMI_INIT(NannyHandler, "няня");
bool password_check( PCMemoryInterface *pci, const DLString &input );

using namespace std;

void NannyHandler::close( Descriptor *d )
{
    Character *ch = d->character;

    if (ch) {
        ::Object *obj, *obj_next;
        
        if (ch->in_room)
            char_from_room( ch );

        char_from_list( ch, &newbie_list );
        
        for (obj = ch->carrying; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;
            extract_obj_1( obj, true );
        }

        for (Character *wch = char_list; wch != 0; wch = wch->next) {
            if (wch->reply == ch)
                wch->reply = 0;
        }
        
        PCharacterManager::quit( ch->getPC( ) );
        LogStream::sendWarning( ) << "Closing nanny link to " << ch->getName( ) << '.' << endl;
        mprog_extract( ch, true );
        ddeallocate( ch );
        d->character = NULL;
    }
}

void NannyHandler::init( Descriptor *d )
{
    NannyHandler *nanny = new NannyHandler;
    
    d->handle_input.clear( );
    d->handle_input.push_front( nanny );
    nanny->doGreeting( d );
}

void NannyHandler::initRemort( Descriptor *d )
{
    NannyHandler *nanny = new NannyHandler;

    d->handle_input.clear( );
    d->handle_input.push_back( nanny );
    nanny->doPlace( d );
}

void NannyHandler::doGreeting( Descriptor *d )
{
    time_t clock = dreamland->getCurrentTime( );
    struct tm * now = localtime( &clock );

    if(d->websock.state == WS_ESTABLISHED)
        return;

    d->send( ANSI_CLEARSCR ANSI_HOME ANSI_COLOR_RESET "\n\r" );
    
    if ((now->tm_mon == 11 && now->tm_mday >= 24)
        || (now->tm_mon == 0 && now->tm_mday <= 8))
    {
        do_help( d, "newyear", true );
    }
    else
        do_help( d, "greeting", false );
}

void NannyHandler::doCodepage( Descriptor *d, char *arg )
{
    int num;

    if (!arg[0] 
        || (num = arg[0] - '1') < 0
        || num >= NCODEPAGES)
    {
        ostringstream buf;

        for (int i = 0; i < NCODEPAGES; i++)
            buf << "  " << i + 1 << ") " << russian_codepages[i].name << endl;
        
        buf << endl << "Select your codepage: ";
        d->send( buf.str( ).c_str( ) );
        return;
    }

    d->buffer_handler = new DefaultBufferHandler( num );
    doPlace( d );
}

void NannyHandler::doPlace( Descriptor *d )
{
    static Scripting::IdRef ID_CONNECT( "onConnect" );
    
    d->send( "\r\n" ANSI_HOME ANSI_CLEARSCR );

    if (d->character == NULL)
        d->associate( PCharacterManager::getPCharacter( ) );
    
    d->connected = CON_NANNY;
    char_to_list( d->character, &newbie_list );
    
    if (!invoke( ID_CONNECT, d, Scripting::RegisterList( ) )) {
        d->send("Front door is closed, please use backdoor.\n");
        d->close( );
    }
}

void NannyHandler::doInterpret( Descriptor *d, char *arg )
{
    static Scripting::IdRef ID_INPUT( "onInput" );
    
    Scripting::RegisterList regList;

    regList.push_back( arg );
    invoke( ID_INPUT, d, regList ); 
}


int NannyHandler::handle(Descriptor *d, char *arg) 
{
    switch (d->connected) {
    case CON_CODEPAGE:
        doCodepage( d, arg );
        break;
    
    case CON_NANNY:
        doInterpret( d, arg );
        break;

    default:
        LogStream::sendError( ) << "nanny: unknown descriptor state " << d->connected << endl;
        break;
    }

    return 0;
}

void NannyHandler::prompt( Descriptor *d )
{
    if (d->character 
        && !d->character->getName( ).empty( )
        && d->echo)
    {
        d->character->send_to( "{R>{x ");
    }
}


void NannyHandler::setSelf( Scripting::Object * )
{
}

Scripting::Register NannyHandler::resolve(Descriptor *d)
{
    static Scripting::IdRef ID_TMP( "tmp" ), ID_NANNY( "nanny" ), ID_INTRO("intro");

    try {
        if (ServerSocketContainer::isNewNanny(d->control)) {
            return *(*Scripting::Context::root[ID_TMP])[ID_INTRO];
        }
        else {
            return *(*Scripting::Context::root[ID_TMP])[ID_NANNY];
        }
    }
    catch (const Scripting::Exception &e) {
        LogStream::sendWarning( ) << "nanny: " << e.what( ) << endl;
    }

    return Scripting::Register();
}

bool NannyHandler::invoke( Scripting::IdRef &id, Descriptor *d, Scripting::RegisterList regList )
{
    if (!FeniaManager::wrapperManager)
        return false;
        
    Scripting::Register tmpNanny = resolve(d);
    if (tmpNanny.type == Register::NONE)
        return false;

    try {
        regList.push_front( FeniaManager::wrapperManager->getWrapper( d->character ) );
        tmpNanny[id]( regList );
    }
    catch (const Scripting::Exception &e) {
        LogStream::sendWarning( ) << "nanny: " << e.what( ) << endl;
        return false;
    }
    
    return true;
}

/*--------------------------------------------------------------------------
 * nanny: login 
 *-------------------------------------------------------------------------*/
Character * NannyHandler::getCharacter( const RegisterList &args ) 
{
    Character *ch;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    FeniaManager::wrapperManager->getTarget( args.front( ), ch );
    return ch;
}

PCharacter * NannyHandler::getPlayer( const RegisterList &args ) 
{
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    return getPlayer( args.front( ) );
}

PCharacter * NannyHandler::getPlayer( const Register &arg ) 
{
    Character *ch;
    
    FeniaManager::wrapperManager->getTarget( arg, ch );

    if (ch->is_npc( ))
        throw Scripting::Exception("nanny invoked on npc");

    return ch->getPC( );
}

NMI_INVOKE( NannyHandler, checkPassword, "" )
{
    PCharacter *ch;
    DLString passwd;

    ch = getPlayer( args );
    passwd = args.back( ).toString( );
    return Register( password_check( ch, passwd ) );
}

NMI_INVOKE( NannyHandler, link, "" )
{
    PCharacter *ch;
    
    ch = getPlayer( args );
    PCharacterManager::update( ch );

    char_from_list( ch, &newbie_list );
    char_to_list( ch, &char_list );

    InterpretHandler::init( ch->desc );
    return Register( );
}

NMI_INVOKE( NannyHandler, load, "" )
{
    PCharacter *ch_old, *ch_new;
    Descriptor *d;
    Room *room;
    
    ch_old = getPlayer( args );
    d = ch_old->desc;
    room = ch_old->in_room;

    ch_new = PCharacterManager::create( ch_old->getName( ) );
    d->handle_input.front( )->close( d );
    d->associate( ch_new );
    PCharacterManager::update( ch_new );

    char_to_room( ch_new, room );
    char_to_list( ch_new, &newbie_list );
    
    return FeniaManager::wrapperManager->getWrapper( ch_new ); 
}

NMI_INVOKE( NannyHandler, notifyCreated, "" )
{
    DescriptorStateManager::getThis( )->handle( 
             CON_CREATE_DONE, 
             CON_READ_MOTD,
             getCharacter( args )->desc );
    return Register( );
}

NMI_INVOKE( NannyHandler, notifyPlaying, "" )
{
    DescriptorStateManager::getThis( )->handle( 
             CON_READ_MOTD,
             CON_PLAYING,
             getCharacter( args )->desc );
    return Register( );
}

NMI_INVOKE( NannyHandler, notifyReconnect, "" )
{
    DescriptorStateManager::getThis( )->handle( 
             CON_BREAK_CONNECT,
             CON_PLAYING,
             getCharacter( args )->desc );
    return Register( );
}

NMI_INVOKE( NannyHandler, reconnect, "" )
{
    Character *ch, *twin;
    Descriptor *d;

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    FeniaManager::wrapperManager->getTarget( args.front( ), ch );
    FeniaManager::wrapperManager->getTarget( args.back( ), twin );
    
    d = ch->desc;
    d->handle_input.front( )->close( d );
    d->associate( twin );
    InterpretHandler::init( d );
    return Register( );
}

NMI_INVOKE( NannyHandler, reanimate, "" )
{
    Character *ch, *twin;
    Descriptor *d;

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    try {
        FeniaManager::wrapperManager->getTarget( args.front( ), ch );
        FeniaManager::wrapperManager->getTarget( args.back( ), twin );
        
        while (( d = descriptor_find_named( ch->desc, ch->getName( ) ) ))
            d->close( );

        if (!PCharacterManager::findPlayer( ch->getName( ) ))
            return false;
        
        d = ch->desc;
        d->handle_input.front( )->close( d );
        d->associate( twin );
        InterpretHandler::init( d );
        return true;
    }
    catch (const Scripting::Exception &) {
        return false;
    }
}

NMI_INVOKE( NannyHandler, checkBan, "" )
{
    PCMemoryInterface *pci;
    Character *ch;
    Descriptor *d;
    
    ch = getPlayer( args );
    d = ch->desc;
    
    if (banManager->check( d, BAN_ALL ))
        return "banall";

    if (( pci = PCharacterManager::find( ch->getName( ) ) )) {
        if (pci->getAttributes( ).isAvailable( "deny" ))
            return "deny";

        if (pci->get_trust( ) < LEVEL_IMMORTAL && banManager->check( d, BAN_PLAYER ))
            return "banplayer";
    }
    else {
        if (dreamland->hasOption( DL_NEWLOCK ))
            return "newlock";

        if (banManager->check( d, BAN_NEWBIES ))
            return "bannewbie";
    }

    if (dreamland->hasOption( DL_WIZLOCK )) 
        if (!pci || (!pci->isOnline( ) && pci->get_trust( ) < LEVEL_HERO))
            return "wizlock";
    
    return Register( );
}

/*--------------------------------------------------------------------------
 * nanny: character creation 
 *-------------------------------------------------------------------------*/
NMI_INVOKE( NannyHandler, checkName, "" )
{
    DLString name;
    
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );
    
    name = args.front( ).toString( );
    if (!badNames->checkName( name )) 
        return false;

    if (descriptor_find_named( NULL, name ))
        return false;

    return true;
}

NMI_INVOKE( NannyHandler, setupStats, "initial, non-random stat setup" )
{
    static const int STAT_DELTAS [] = {
        -4, // str
        -3, // int
        -3, // wis
        -3, // dex
        -3, // con
        -8, // cha
    };
    PCharacter *target;

    target = getPlayer( args );
    
    if (target->getHometown( ) != home_frigate && !target->isCoder( ))
        throw Scripting::Exception( "setupStats requested on non-newbie" );

    for (int i = 0; i < stat_table.size; i++) {
        int max_train = target->getMaxTrain( i );
        target->perm_stat[i] = max_train + STAT_DELTAS[i];
    }

    return Register( );
}

static int get_max_train( PCharacter *target, int stat )
{
    int max_train = target->getMaxTrain( stat );

    if (stat == STAT_CHA)
        max_train -= 5;
    
    return max_train;
}

NMI_INVOKE( NannyHandler, randomizeStats, "randomize player stats during rolling" )
{
    int i, sum;
    vector<int> marks;
    static const int MIN_THROW = 10;
    static const int MIN_SUM = 20;
    PCharacter *target;

    target = getPlayer( args );
    
    if (target->getHometown( ) != home_frigate && !target->isCoder( ))
        throw Scripting::Exception( "randomizeStats requested on non-newbie" );

    for (i = 0, sum = 0; i < stat_table.size; i++) {
        int max_train = get_max_train( target, i );
        target->perm_stat[i] = max( MIN_THROW, 
                                max_train - number_range( 0, 5 ) );
        sum += max_train - target->perm_stat[i];
    }
    
    marks.resize( stat_table.size, 0 );

    while (sum != MIN_SUM) {
        int j, m, d;
        
        for (j = 0, m = 0; j < stat_table.size; j++)
            m += marks[j];

        m /= stat_table.size;
        
        for ( ; ; ) {
            j = number_range( 0, stat_table.size - 1 );

            if (marks[j] == m) {
                marks[j] = m + 1;
                break;
            }
        }
        
        if (sum > MIN_SUM)
            d = min( get_max_train( target, j ) - target->perm_stat[j], 
                     number_range( 0, sum - MIN_SUM ) );
        else
            d = max( MIN_THROW - target->perm_stat[j], 
                     number_range( sum - MIN_SUM, 0 ) );
        
        target->perm_stat[j] += d;
        sum -= d;
    }

    return Register( );
}

NMI_INVOKE( NannyHandler, alignAllowed, "" )
{
    ostringstream buf;
    
    align_print_allowed( getPlayer( args ), buf );
    return buf.str( );
}

NMI_INVOKE( NannyHandler, alignChoose, "" )
{
    DLString a;
    int n;
    PCharacter *target;

    if (args.size( ) != 2)
       throw Scripting::NotEnoughArgumentsException( );
    
    target = getPlayer( args.front( ) );
    a = args.back( ).toString( );
    
    if (a.isNumber( )) 
        try {
            n = align_choose_allowed( target, a.toInt( ) );  
        } catch (const ExceptionBadType &) {
            return false;
        }
    else
        n = align_choose_allowed( target, a.c_str( ) );

    
    if (n == ALIGN_ERROR)
        return false;

    target->alignment = n;
    return true;
}

NMI_INVOKE( NannyHandler, help, "" )
{
    DLString a;
    Character *target;

    if (args.size( ) != 2)
       throw Scripting::NotEnoughArgumentsException( );
    
    target = getCharacter( args );
    a = args.back( ).toString( );

    do_help( target, a.c_str( ) );
    return Register( );
}

