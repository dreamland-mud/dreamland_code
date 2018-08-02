/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WEARLOC_CODES_H
#define WEARLOC_CODES_H

#define F_WEAR_VERBOSE (A)
#define F_WEAR_REPLACE (B)

enum {
    RC_WEAR_OK = 0,
    RC_WEAR_NOMATCH,
    RC_WEAR_CONFLICT,
    RC_WEAR_NOREPLACE,
    RC_WEAR_NORIB,
    RC_WEAR_YOUNG,
    RC_WEAR_HEAVY,
    RC_WEAR_LARGE,
};

#endif
