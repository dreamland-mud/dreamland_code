/* $Id: who.cpp,v 1.1.2.26.6.10 2009/08/16 20:28:04 rufina Exp $
 *
 * ruffina, 2003
 */

#include <sstream>

#include "json/json.h"
#include "class.h"
#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "pcrace.h"

#include "commandtemplate.h"
#include "plugininitializer.h"
#include "jsonservlet.h"
#include "commonattributes.h"
#include "servlet_utils.h"
#include "webmanip.h"
#include "webprompt.h"
#include "clanreference.h"
#include "fight.h"
#include "act.h"
#include "skillreference.h"
#include "mercdb.h"
#include "handler.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"

static const DLString WHO = "who";

GSN(manacles);
GSN(jail);
CLAN(none);

/** Check if profession in 'who/whois' is visible when both belong to the same clan. */
bool can_see_profession( PCharacter *ch, PCMemoryInterface *victim ) 
{
    if (ch->is_immortal( ))
        return true;
    
    if (ch == victim->getPlayer())
        return true;
    
    if (ch->getClan( ) != clan_none 
        && ch->getClan( ) == victim->getClan( )
        && !ch->getClan( )->isDispersed( ))
        return true;

    return false;
}

/** Check if level in 'who/whois' is visible because of low charisma or same clan. */
bool can_see_level( PCharacter *ch, PCMemoryInterface *victim ) 
{
    if (victim->isOnline() && victim->getPlayer()->getCurrStat( STAT_CHA ) < 18)
        return true;

    return can_see_profession( ch, victim );
}

/** Get a list of online players excluding gods in wizinvis. */
list<PCharacter *> who_find_online(PCharacter *looker)
{
    list<PCharacter *> result;

    for (Descriptor *d = descriptor_list; d; d = d->next) {
        PCharacter *victim;
        
        if (d->connected != CON_PLAYING || !d->character)
            continue;

        victim = d->character->getPC( );

        if (!can_see_god(looker, victim)) 
            continue;

        XMLAttributes *attrs = &victim->getAttributes( );
        if (attrs->isAvailable("nowho"))
            continue;

        result.push_back(victim);
    }

    return result;
}

/** Get a list of offline players that are online/idle in Discord. */
list<PCMemoryInterface *> who_find_offline(PCharacter *looker)
{
    list<PCMemoryInterface *> result;

    for (const auto &kv: PCharacterManager::getPCM()) {
        PCMemoryInterface *pcm = kv.second;
        Json::Value discord;
        
        // Consider link dead people as offline.
        if (pcm->getPlayer() && pcm->getPlayer()->desc)
            continue;

        if (get_json_attribute(pcm, "discord", discord) && !discord["id"].asString().empty()) {
            DLString status = discord["status"].asString();
            if (status != "offline") {
                result.push_back(pcm);
            }
        }
    }

    return result;
}

/** Format left column of the 'who' output, with race/class/clan. */
static DLString who_cmd_left_column(PCharacter *ch, PCharacter *vict) 
{
    std::basic_ostringstream<char> buf, tmp;
    bool coder = vict->getAttributes().isAvailable("coder");

    if (coder && ch->isCoder( ) && vict->getLevel( ) >= LEVEL_HERO) {
        /* visible only to each other, like vampires >8) */
        buf << GET_SEX( vict, "{le{C    CODER    {lr{C    КОДЕР    {x",
                              "{le{C    CODER    {lr{C    КОДЕР    {x",
                              "{le{C    CODER    {lr{C   КОДЕРША   {x" );         
        return buf.str( );
    }
    
    switch (vict->getLevel( )) {

    case MAX_LEVEL - 0: buf << GET_SEX( vict, "{le{W IMPLEMENTOR {lr{W   ДЕМИУРГ   {x",
                                              "{le{W IMPLEMENTOR {lr{W   ДЕМИУРГ   {x",
                                              "{le{WIMPLEMENTRESS{lr{W  ДЕМИУРГИНЯ {x" );            
                        break;

    case MAX_LEVEL - 1: buf << GET_SEX( vict, "{le{С   СREATOR   {lr{С  ТВОРЯЩИЙ   {x",
                                              "{le{С   СREATOR   {lr{С  ТВОРЯЩИЙ   {x",
                                              "{le{С   СREATOR   {lr{С  ТВОРЯЩАЯ   {x" );            
                        break;            

    case MAX_LEVEL - 2: buf << GET_SEX( vict, "{le{C  SUPREMACY  {lr{С  ТВОРЯЩИЙ   {x",
                                              "{le{C  SUPREMACY  {lr{С  ТВОРЯЩИЙ   {x",
                                              "{le{C  SUPREMACY  {lr{С  ТВОРЯЩАЯ   {x" );            
                        break;
                                       
    case MAX_LEVEL - 3: buf << GET_SEX( vict, "{le{C    DEITY    {lr{С  ТВОРЯЩИЙ   {x",
                                              "{le{C    DEITY    {lr{С  ТВОРЯЩИЙ   {x",
                                              "{le{C    DEITY    {lr{С  ТВОРЯЩАЯ   {x" );            
                        break;
            
    case MAX_LEVEL - 4: buf << GET_SEX( vict, "{le{C     GOD     {lr{С     БОГ     {x",
                                              "{le{C     GOD     {lr{С     БОГ     {x",
                                              "{le{C   GODDESS   {lr{С   БОГИНЯ    {x" );            
                        break;
            
    case MAX_LEVEL - 5: buf << GET_SEX( vict, "{le{G   IMMORTAL  {lr{G БЕССМЕРТНЫЙ {x",
                                              "{le{G   IMMORTAL  {lr{G БЕССМЕРТНЫЙ {x",
                                              "{le{G   IMMORTAL  {lr{G БЕССМЕРТНАЯ {x" );            
                        break;
            
    case MAX_LEVEL - 6: buf << GET_SEX( vict, "{le{G   DEMIGOD   {lr{G   ПОЛУБОГ   {x",
                                              "{le{G   DEMIGOD   {lr{G   ПОЛУБОГ   {x",
                                              "{le{G DEMIGODDESS {lr{G ПОЛУБОГИНЯ  {x" );            
                        break;

    case MAX_LEVEL - 7: buf << GET_SEX( vict, "{le{G    ANGEL    {lr{G    АНГЕЛ    {x",
                                              "{le{G    ANGEL    {lr{G    АНГЕЛ    {x",
                                              "{le{G    ANGEL    {lr{G    АНГЕЛ    {x" );            
                        break;

    case MAX_LEVEL - 8: buf << GET_SEX( vict, "{le{G   AVATAR    {lr{G   АВАТАР    {x",
                                              "{le{G   AVATAR    {lr{G   АВАТАР    {x",
                                              "{le{G   AVATAR    {lr{G   АВАТАР    {x" );            
                        break;
                        
    case MAX_LEVEL - 9: buf << GET_SEX( vict, "{le{y  LEGENDARY  {lr{G ЛЕГЕНДАРНЫЙ {x",
                                              "{le{y  LEGENDARY  {lr{G ЛЕГЕНДАРНЫЙ {x",
                                              "{le{y  LEGENDARY  {lr{y ЛЕГЕНДАРНАЯ {x" );            
                        break;
            
    }

    if (!buf.str( ).empty( ))
        return buf.str( );

    /* Level */
    if (can_see_level(ch, vict)) {
        if (!can_see_profession(ch, vict))
            tmp << "{c";

        tmp << vict->getLevel() << "{x";
    }

    buf << dlprintf( "%3s", tmp.str( ).c_str( ) );
    tmp.str( "" );
    
    /* Race */
    tmp << vict->getRace( )->getPC( )->getWhoNameFor( ch );

    if (tmp.str( ).size( ) < 4)
        buf << "  " << dlprintf( "%-4s", tmp.str( ).c_str( ) );
    else 
        buf << " " << dlprintf( "%-5s", tmp.str( ).c_str( ) );

    tmp.str( "" );

    /* Class */
    if (can_see_profession(ch, vict))
        tmp << "{Y" << vict->getProfession( )->getWhoNameFor( ch );

    buf << " " << dlprintf( "%3s", tmp.str( ).c_str( ) );
    return buf.str( );
}

/** Format flags near the player name in 'who' command. */
static DLString who_cmd_flags(PCharacter *victim) 
{
    std::basic_ostringstream<char> buf, result;
    XMLAttributes *attrs = &victim->getAttributes( );

    if (IS_SET( victim->comm, COMM_AFK ))  buf << "{CA";
    if (victim->incog_level >= LEVEL_HERO) buf << "{DI";
    if (victim->invis_level >= LEVEL_HERO) buf << "{DW";
    if (IS_KILLER( victim ))               buf << "{RK";
    if (IS_THIEF( victim ))                buf << "{RT";
    if (IS_SLAIN( victim ))                buf << "{DS";
    if (IS_GHOST( victim ))                buf << "{DG";
    if (IS_DEATH_TIME( victim ))           buf << "{DP";
    if (IS_VIOLENT( victim ))              buf << "{BV";
    if (victim->curse != 100)              buf << "{DC";
    if (victim->bless)                     buf << "{CB";
    if (IS_SET( victim->act, PLR_WANTED))  buf << "{RW";
    if (victim->isAffected(gsn_manacles)) buf << "{mM";        
    if (victim->isAffected(gsn_manacles)) buf << "{mM";
    if (victim->isAffected(gsn_manacles)) buf << "{mM";        
    if (victim->isAffected(gsn_jail ))   buf << "{mJ";
    if (!IS_SET( victim->act, PLR_CONFIRMED )) buf << "{gU";
    if (attrs->isAvailable("nochannel"))   buf << "{mN";
    if (attrs->isAvailable( "nopost" ))    buf << "{mP";
    if (attrs->isAvailable( "teacher" ))   buf << "{gT";

    if (!buf.str( ).empty( )) 
        result << "{x(" << buf.str( ) << "{x)";

    return result.str( );
}

/** Format offline players for the 'who' command. */
static DLString who_cmd_format_offline( PCharacter *ch, PCMemoryInterface *victim ) 
{
    ostringstream buf;
    Json::Value discord;

    get_json_attribute(victim, "discord", discord);

    DLString status = discord["status"].asString(); 
    DLString color = status == "online" ? "G" : (status == "idle" ? "Y" : "R");
    buf << " {" << color << "*{x ";

    DLString name = ch->getConfig().runames && !victim->getRussianName().getFullForm().empty() 
        ? victim->getRussianName().decline('1') : victim->getName();
    buf << "{W" << name << "  {D(Discord){x" << endl;        
    
    return buf.str();
}

/** Format one line of 'who' command. */
static DLString who_cmd_format_char( PCharacter *ch, PCharacter *victim ) 
{
    DLString result;
    std::basic_ostringstream<char> buf, tmp;

    /* Level, Race, Class */
    buf << "{x|" << who_cmd_left_column( ch, victim ) << "{x|";
        
    /* PK */
    if (victim->getModifyLevel( ) >= PK_MIN_LEVEL && !is_safe_nomessage( ch, victim ))
        tmp << "{x({rPK{x)";
    
    buf << dlprintf( "%4s", tmp.str( ).c_str( ) );
    tmp.str( "" );

    /* Clan, (R) (L) */
    if (!victim->getClan( )->isHidden( ) && victim->get_trust() < LEVEL_IMMORTAL) {
        const Clan &clan = *victim->getClan( );

        tmp << "{x[{" << clan.getColor( ) << clan.getShortName( ).at( 0 ) << "{x]";
        
        if (clan.isLeader( victim ))
            tmp << "{R({CL{R){x";
        else if (clan.isRecruiter( victim ))
            tmp << "{R({CR{R){x";
        else 
            tmp << "   ";
    } 
    
    buf << dlprintf( "%6s", tmp.str( ).c_str( ) );
    tmp.str( "" );
   
    /* Remorts */
    int remorts = victim->getRemorts( ).size( );
    if (remorts) {
        if (remorts < 10)
                tmp << " {W({M" + DLString(remorts) + "{W)";
        else
                tmp << "{W({M" + DLString(remorts) + "{W)";
    }
    
    buf << dlprintf( "%4s", tmp.str( ).c_str( ) );
    tmp.str( "" );

    /* Flags */
    buf << dlprintf( "%9s", who_cmd_flags(victim).c_str( ) );

    /* Pretitle, Name, Title */
    bool coder = victim->getAttributes().isAvailable("coder");
    if (victim->getLevel( ) > LEVEL_HERO 
            || (coder && ch->isCoder( ) && victim->getLevel( ) >= LEVEL_HERO))
        buf << "{C";
    else
        buf << "{W";
   
    DLString descr = ch->seeName( victim );
    webManipManager->decorateCharacter( buf, descr, victim, ch );
    buf << "{x " << victim->getParsedTitle( ) << "{x" << endl;

    return buf.str( );
}


/*-------------------------------------------------------------------------
 * 'who' command.
 *------------------------------------------------------------------------*/
CMDRUN(who)
{
    ostringstream buf;
    DLString arg, arguments = constArguments;
    
    if (ch->getPC( ) == 0)
        return;

    PCharacter *pch = ch->getPC();
    list<PCharacter *> online = who_find_online(pch);
    int online_count = online.size();
    list<PCMemoryInterface *> offline = who_find_offline(pch);
    int offline_count = offline.size();
    int total = online_count + offline_count;

    // Refresh offline player count, just in case.
    Descriptor::updateMaxOffline(offline_count);
    int max_total = Descriptor::getMaxOnline() + Descriptor::getMaxOffline();

    if (online_count > 0)
        buf << fmt(0, "Сейчас в мире {W%1$d{w игрок%1$I|а|ов:", online_count) << endl;
    for (const auto &victim: online)
        buf << who_cmd_format_char(pch, victim);

    if (offline_count > 0)
        buf << endl << fmt(0, "Еще {W%1$d{w игрок%1$I|а|ов слыш%1$Iит|ат|ат общие каналы:", offline_count) << endl;
    for (const auto &victim: offline)
        buf << who_cmd_format_offline(pch, victim);

    buf << endl;

    buf << "Всего {W" << total << "{w, максимум за последнее время был {W" << max_total << "{w." << endl;

    if (!IS_SET( ch->act, PLR_CONFIRMED ) && pch->getRemorts( ).size( ) == 0) 
        buf << "Буква (U) рядом с твоим именем означает, что твое описание еще не одобрено богами." << endl
            << "Подробнее читай {y{hcсправка подтверждение{x." << endl;
    ch->send_to( buf );
}

/*-------------------------------------------------------------------------
 * WhoWebPromptListener
 *------------------------------------------------------------------------*/
class WhoWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<WhoWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );

        static Json::Value jsonPlayer( PCharacter *ch, PCharacter *wch );
        static Json::Value jsonWho( PCharacter *ch );
};

void WhoWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    attr->updateIfNew( WHO, jsonWho( ch->getPC() ), prompt ); 
}    

Json::Value WhoWebPromptListener::jsonPlayer( PCharacter *ch, PCharacter *wch )
{
    Json::Value player;
    
    // Player name and sex.
    player["n"] = wch->toNoun(ch)->decline('1').cutSize(10);
    player["s"] = wch->getSex( ) == SEX_MALE ? "m" : "f";

    // First 2 letters of player race.
    player["r"] = wch->getRace( )->getName( ).substr(0, 2);

    // Clan name (first letter) and colour.
    player["cn"] = wch->getClan( )->getName( ).substr(0, 1);
    
    DLString clr = wch->getClan( )->getColor( );
    if (!clr.empty( )) {
        player["cc"] = DLString(dl_isupper(clr.at(0)) ? "b" : "d") + dl_tolower(clr.at(0));
    }

    return player;
}

Json::Value WhoWebPromptListener::jsonWho( PCharacter *ch )
{
    Json::Value who;
    list<PCharacter *> players = who_find_online(ch);
    list<PCharacter *>::iterator p;

    // Populate player list.
    int pc = 0;
    for (p = players.begin( ); p != players.end( ); p++) {
        who["p"][pc++] = jsonPlayer( ch, *p );
    }

    // Total player count.
    who["t"] = DLString(players.size());
    return who;
}

PluginInitializer<WhoWebPromptListener> initWhoWebPromptListener;

/*-------------------------------------------------------------------------
 * /who public servlet
 *------------------------------------------------------------------------*/
JSONSERVLET_HANDLE(cmd_who, "/who")
{
    PCharacter dummy;
    dummy.config.setBit(CONFIG_RUCOMMANDS);
    
    list<PCharacter *> online = who_find_online(0);
    list<PCMemoryInterface *> offline = who_find_offline(0);

    for (const auto &victim: online) {
        Json::Value wch;

        wch["name"]["en"] = victim->getName();
        wch["name"]["ru"] = victim->getRussianName().decline('1');
        wch["race"]["en"] = victim->getRace()->getName();
        wch["race"]["ru"] = victim->getRace()->getNameFor(&dummy, victim);

        if (victim->getClan() != clan_none)
            wch["clan"]["en"] = victim->getClan()->getShortName().toLower().upperFirstCharacter();

        if (!victim->getPretitle().empty())
            wch["pretitle"]["en"] = victim->getPretitle().colourStrip();

        if (!victim->getRussianPretitle().empty())
            wch["pretitle"]["ru"] = victim->getRussianPretitle().colourStrip();

        wch["title"] = victim->getParsedTitle().colourStrip();
        wch["remorts"] = DLString(victim->getRemorts().size());

        body["people"].append(wch);
    }

    for (const auto &victim: offline) {
        Json::Value wch;

        wch["name"]["en"] = victim->getName();
        wch["name"]["ru"] = victim->getRussianName().decline('1');
        body["discord"].append(wch);
    }

    body["total"] = (int)(online.size() + offline.size());
}
