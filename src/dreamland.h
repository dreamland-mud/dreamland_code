/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
// dreamland.h: interface for the DreamLand class.
//
//////////////////////////////////////////////////////////////////////

#ifndef DREAMLAND_H
#define DREAMLAND_H

#include "oneallocate.h"

#include "xmlvariablecontainer.h"
#include "xmlboolean.h"
#include "xmllong.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlinteger.h"

#include "dldirectory.h"
#include "xmlconfigurable.h"

#include <sys/time.h>


class DbEnvContext;

class DreamLand;
class DLPluginManager;
class PCharacterManager;
class NPCharacterManager;
class ObjectManager;
class DLScheduler;
class RaceManager;
class RaceLanguageManager;
class SkillManager;
class ClanManager;
class ProfessionManager;
class FeniaManager;
class ProcessManager;
class HometownManager;
class WearlocationManager;
class ReligionManager;
class LiquidManager;
class DesireManager;
class HelpManager;
class SkillGroupManager;
class BonusManager;
class SocketManager;
class ServletManager;
class EventBus;

extern DreamLand * dreamland;

class DreamLand : public OneAllocate, 
                  public XMLVariableContainer,
                  public XMLConfigurableWithPath
{
XML_OBJECT
public:
        DreamLand( );
        virtual ~DreamLand( );

        void shutdown( );
        bool isShutdown( ) const;

        void run( );
        void load( bool = true );
        void save( bool = true );

        inline int getRebootCounter( ) const;
        inline void setRebootCounter( int );
        inline int getPulsePerSecond( ) const;
        inline int getPulseViolence( ) const;
        inline int getPulseMobile( ) const;
        inline int getPulseTick( ) const;

        inline const DLString& getBasePath( ) const;
        inline const DLString& getTablePath( ) const;
        inline const DLString& getDbPath( ) const;
        inline const DLString& getMiscPath( ) const;
        inline const DLString& getLibexecPath( ) const;
        inline DLDirectory getTableDir( ) const;
        inline DLDirectory getDbDir( ) const;
        inline DLDirectory getAreaDir( ) const;
        inline DLDirectory getMiscDir( ) const;
        inline DLDirectory getLibexecDir( ) const;
        inline DLDirectory getSavedDir( ) const;
        inline DLDirectory getPlayerDir( ) const;
        inline DLDirectory getPlayerDeleteDir( ) const;
        inline DLDirectory getPlayerRemortDir( ) const;
        inline DLDirectory getPlayerBackupDir( ) const;
        inline DLDirectory getFeniaScriptDir() const;
        inline const DLString& getTempFile( ) const;
        inline const DLString& getImmLogFile( ) const;
        inline const DLString& getShutdownFile( ) const;
        inline const DLString& getAreaListFile( ) const;

        inline const DLString & getVersion( ) const;
        inline long getBootVersion( ) const;
        inline time_t getBootTime( ) const;
        inline time_t getCurrentTime( ) const;
        inline long getWorldTime( ) const;
        inline void setWorldTime( long );

        inline bool getFromMerchantBank( long gold );
        inline void putToMerchantBank( long gold );
        inline long getBalanceMerchantBank( ) const;
        
        inline void setConfigFileName( const DLString& );
        inline long long genID( );
        inline DbEnvContext *getFeniaDbEnv( ) const;

        inline bool hasOption( int ) const;
        inline void setOption( int );
        inline void removeOption( int );
        inline void resetOption( int );
        void setCurrentTime( );

private:
        void setBootTime( );
        void pulseStart( );
        void pulseEnd( );

        void setSignals( );
        static void signalHandler( int );

private:
        static const DLString version; 
        static const long     DEFAULT_OPTIONS;

        /** time of this pulse */
        time_t currentTime;
        /** world booting time */
        time_t bootTime;

        struct timeval pulseStartTime;

        long long lastID;
        
        int rebootCounter;
        
        Flags workingOptions;
        XML_VARIABLE XMLFlags options;
        XML_VARIABLE XMLInteger pulsePerSecond;
        XML_VARIABLE XMLInteger pulseViolence;
        XML_VARIABLE XMLInteger pulseMobile;
        XML_VARIABLE XMLInteger pulseTick;
        XML_VARIABLE XMLString basePath;
        XML_VARIABLE XMLString tablePath;
        XML_VARIABLE XMLString miscPath;
        XML_VARIABLE XMLString libexecPath;
        XML_VARIABLE XMLString dbPath;
        XML_VARIABLE XMLString playerDir;
        XML_VARIABLE XMLString playerDeleteDir;
        XML_VARIABLE XMLString playerRemortDir;
        XML_VARIABLE XMLString playerBackupDir;
        XML_VARIABLE XMLString tempFile;                                 
        XML_VARIABLE XMLString immLogFile;
        XML_VARIABLE XMLString shutdownFile;
        XML_VARIABLE XMLString areaListFile;
        XML_VARIABLE XMLString areaDir;
        XML_VARIABLE XMLString savedDir;
        XML_VARIABLE XMLString logPattern;

        XML_VARIABLE XMLString feniaDbDir;
        XML_VARIABLE XMLString feniaScriptDir;

        XML_VARIABLE XMLLong   bootVersion;
        XML_VARIABLE XMLLong   worldTime;
        XML_VARIABLE XMLLong   merchantBank;

private:
        ::Pointer<DLScheduler> scheduler;
        ::Pointer<DLPluginManager> pluginManager;
        ::Pointer<ObjectManager> objectManager;
        ::Pointer<PCharacterManager> pcharacterManager;
        ::Pointer<NPCharacterManager> npcharacterManager;
        ::Pointer<RaceManager> raceManager;
        ::Pointer<RaceLanguageManager> raceLanguageManager;
        ::Pointer<SkillManager> skillManager;
        ::Pointer<ClanManager> clanManager;
        ::Pointer<ProfessionManager> professionManager;
        ::Pointer<FeniaManager> feniaManager;
        ::Pointer<ProcessManager> processManager;
        ::Pointer<HometownManager> hometownManager;
        ::Pointer<WearlocationManager> wearlocationManager;
        ::Pointer<LiquidManager> liquidManager;
        ::Pointer<ReligionManager> religionManager;
        ::Pointer<DesireManager> desireManager;
        ::Pointer<HelpManager> helpManager;
        ::Pointer<SkillGroupManager> skillGroupManager;
        ::Pointer<BonusManager> bonusManager;
        ::Pointer<SocketManager> socketManager;
        ::Pointer<ServletManager> servletManager;
        ::Pointer<EventBus> eventBus;

        DbEnvContext *feniaDbEnv;
};

#endif

#include "dreamland-impl.h"


