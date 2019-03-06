/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __WIZNET_H__
#define __WIZNET_H__


/* WIZnet flags */
#define WIZ_ON                  (A)
#define WIZ_TICKS               (B)
#define WIZ_LOGINS              (C)
#define WIZ_SITES               (D)
#define WIZ_LINKS               (E)
#define WIZ_DEATHS              (F)
#define WIZ_RESETS              (G)
#define WIZ_MOBDEATHS           (H)
#define WIZ_FLAGS               (I)
#define WIZ_PENALTIES           (J)
#define WIZ_SACCING             (K)
#define WIZ_LEVELS              (L)
#define WIZ_SECURE              (M)
#define WIZ_SWITCHES            (N)
#define WIZ_SNOOPS              (O)
#define WIZ_RESTORE             (P)
#define WIZ_LOAD                (Q)
#define WIZ_NEWBIE              (R)
#define WIZ_PREFIX              (S)
#define WIZ_SPAM                (T)
#define WIZ_DAMAGES             (U)
#define WIZ_RNAMES              (V)
#define WIZ_CONFIRM             (W)
#define WIZ_QUEST               (X)
#define WIZ_LANGUAGE            (Y)
#define WIZ_SKILLS              (Z)

struct wiznet_type {
    const char * name;
    long         flag;
    int                level;
};

extern const struct wiznet_type wiznet_table [];

long wiznet_lookup( const char *name );

void wiznet( long flag, long flag_skip, int min_level, const char *fmt, ... );

#endif
