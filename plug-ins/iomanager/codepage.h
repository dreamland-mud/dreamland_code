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

#ifndef _CHARSET_H_
#define _CHARSET_H_

#define	NCODEPAGES		7

struct codepage_t {
  const char *		name;
  unsigned char *	from;
  unsigned char *	to;
};

extern unsigned char koi8_koi8[256];
extern unsigned char alt_koi8[256];
extern unsigned char koi8_alt[256];
extern unsigned char win_koi8[256];
extern unsigned char koi8_win[256];
extern unsigned char iso_koi8[256];
extern unsigned char koi8_iso[256];
extern unsigned char mac_koi8[256];
extern unsigned char koi8_mac[256];
extern unsigned char tran_koi8[256];
extern unsigned char koi8_tran[256];

extern codepage_t russian_codepages[];

#endif
