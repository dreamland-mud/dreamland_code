/* $Id$
 *
 * ruffina, Dream Land, 2018
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include "dlstring.h"

/**
 * Produce a digest of given password string using given salt.
 */
DLString digestWithBinarySalt( const DLString &password, unsigned char *salt, size_t salt_len );

/**
 * Produce a digest of given password string using unique random salt.
 */
DLString digestWithRandomSalt( const DLString &password );


/**
 * Produce a digest of given password string using salt in HEX format.
 */
DLString digestWithHexSalt( const DLString &password, const DLString &hexSalt );

#endif    
