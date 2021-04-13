/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DREAMLAND_IMPL_H
#define DREAMLAND_IMPL_H

#include "dreamland.h"

inline int DreamLand::getRebootCounter( ) const
{
    return rebootCounter;
}

inline void DreamLand::setRebootCounter( int value )
{
    rebootCounter = value;
}

inline int DreamLand::getPulsePerSecond( ) const
{
    return pulsePerSecond.getValue( );
}

inline int DreamLand::getPulseViolence( ) const
{
    return pulseViolence.getValue( );
}

inline int DreamLand::getPulseMobile( ) const
{
    return pulseMobile.getValue( );
}

inline int DreamLand::getPulseTick( ) const
{
    return pulseTick.getValue( );
}

inline const DLString& DreamLand::getBasePath( ) const
{
    return basePath;
}

inline const DLString& DreamLand::getTablePath( ) const
{
    return tablePath;
}

inline const DLString& DreamLand::getMiscPath( ) const
{
    return miscPath;
}

inline const DLString& DreamLand::getLibexecPath( ) const
{
    return libexecPath;
}

inline DLDirectory DreamLand::getLibexecDir( ) const
{
    return DLDirectory( basePath, libexecPath );
}

inline const DLString& DreamLand::getDbPath( ) const
{
    return dbPath;
}

inline DLDirectory DreamLand::getTableDir( ) const
{
    return DLDirectory( basePath, tablePath );
}

inline DLDirectory DreamLand::getDbDir( ) const
{
    return DLDirectory( basePath, dbPath );
}

inline DLDirectory DreamLand::getMiscDir( ) const
{
    return DLDirectory( basePath, miscPath );
}

inline DLDirectory DreamLand::getAreaDir( ) const
{
    return DLDirectory( basePath, areaDir );
}

inline DLDirectory DreamLand::getSavedDir( ) const
{
    return DLDirectory( basePath, savedDir );
}

inline DLDirectory DreamLand::getPlayerDir( ) const
{
    return DLDirectory( basePath, playerDir );
}

inline DLDirectory DreamLand::getPlayerDeleteDir( ) const
{
    return DLDirectory( basePath, playerDeleteDir );
}

inline DLDirectory DreamLand::getPlayerRemortDir( ) const
{
    return DLDirectory( basePath, playerRemortDir );
}

inline DLDirectory DreamLand::getPlayerBackupDir( ) const
{
    return DLDirectory( basePath, playerBackupDir );
}

inline DLDirectory DreamLand::getFeniaScriptDir( ) const
{
    return DLDirectory( basePath, feniaScriptDir );
}

inline const DLString & DreamLand::getTempFile( ) const
{
    return tempFile;
}

inline const DLString& DreamLand::getImmLogFile( ) const
{
    return immLogFile;
}

inline const DLString& DreamLand::getShutdownFile( ) const
{
    return shutdownFile;
}

inline const DLString& DreamLand::getAreaListFile( ) const
{
    return areaListFile;
}

inline long DreamLand::getBootVersion( ) const
{
    return bootVersion.getValue( );
}

inline long DreamLand::getWorldTime( ) const
{
    return worldTime.getValue( );
}

inline void DreamLand::setWorldTime( long newWorldTime )
{
    worldTime.setValue( newWorldTime );
}

inline long DreamLand::getBalanceMerchantBank() const
{
    return merchantBank.getValue();
}

inline long long DreamLand::genID( )
{
    long long ver = ((long long)getBootVersion( )) << 32;
    lastID++;
    return ver + lastID;
}

inline time_t DreamLand::getCurrentTime( ) const
{
    return currentTime;
}

inline time_t DreamLand::getBootTime( ) const
{
    return bootTime;
}

inline const DLString& DreamLand::getVersion( ) const
{
    return version;
}

inline DbEnvContext * DreamLand::getFeniaDbEnv( ) const
{
    return feniaDbEnv;
}

inline bool DreamLand::hasOption( int opt ) const
{
    return workingOptions.isSet( opt );
}

inline void DreamLand::setOption( int opt )
{
    workingOptions.setBit( opt );
}

inline void DreamLand::removeOption( int opt )
{
    workingOptions.removeBit( opt );
}

inline void DreamLand::resetOption( int opt )
{
    if (options.isSet( opt ))
        workingOptions.setBit( opt );
    else
        workingOptions.removeBit( opt );
}

inline bool DreamLand::getFromMerchantBank( long gold )
{
    if (gold > merchantBank)
        return false;
    else {
        merchantBank -= gold;
        return true;
    }
}

inline void DreamLand::putToMerchantBank( long gold )
{
    merchantBank += gold;
}

#endif
