/* $Id$
 *
 * ruffina, Dream Land, 2018
 */

#include "cryptopp/sha.h"
#include "cryptopp/hex.h"
#include "cryptopp/pwdbased.h"
#include "cryptopp/osrng.h"

#include "crypto.h"

using namespace CryptoPP;

DLString digestWithBinarySalt( const DLString &password, ::byte *salt, size_t salt_len )
{
   unsigned int iterations = 1000;
   
   // Derived key (password hash) storage.
   SecByteBlock derivedkey(SHA512::DIGESTSIZE);

   // Encryption algo implementation.
   PKCS5_PBKDF2_HMAC<SHA512> pbkdf;
   
   // Generate the password hash using provided salt.
   pbkdf.DeriveKey(
      derivedkey, derivedkey.size(),
      0x00,
      (::byte *) password.data(), password.size(),
      salt, salt_len,
      iterations
   );
   // Convert salt into HEX and store in hexSalt.
   DLString hexSalt;
   StringSource ss1(salt, salt_len, true, new HexEncoder(new StringSink(hexSalt)));
   // Convert derived key into HEX and store in hash.
   DLString hash;
   StringSource ss2(derivedkey, derivedkey.size(), true, new HexEncoder(new StringSink(hash)));

   // Return salt prepended to the hash.
   return hexSalt + "$" + hash;
}

DLString digestWithHexSalt( const DLString &password, const DLString &hexSalt )
{
  // Decode HEX salt into byte string.
  DLString salt;
  StringSource ss( hexSalt, true /*pumpAll*/, new HexDecoder( new StringSink(salt) ) );
  // Call digest to return salt and hash combination.
  return digestWithBinarySalt( password, (::byte *)salt.data( ), salt.size( ) );
}

DLString digestWithRandomSalt( const DLString &password )
{
   // Generate new random salt.
   SecByteBlock salt(SHA512::DIGESTSIZE);
   AutoSeededRandomPool().GenerateBlock(salt, salt.size());
   // Call digest to return salt and hash combination.
   return digestWithBinarySalt( password, salt, salt.size( ) );
}

