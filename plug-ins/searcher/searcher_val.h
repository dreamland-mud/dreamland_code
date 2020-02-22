/* $Id$
 *
 * ruffina, 2019
 */
#ifndef SEARCHERVAL_H
#define SEARCHERVAL_H

#include "dlstring.h"

struct obj_index_data;

struct SValue {
    DLString str;
    int num;
    bool result;
};

#define YYSTYPE SValue

#define YY_DECL            int searcher_yylex( struct SValue *value )
YY_DECL;

struct SearcherParam {
    struct obj_index_data *pObj;
    DLString extra;
    DLString wear;
    DLString itemtype;
    DLString name;
    int hr, dr, hp, mana, move, saves, armor, size, age;
    int str, inta, wis, dex, con, cha;
    int heal_gain, mana_gain;
    int slevel;
    DLString aff, vuln, res, imm, det;
    DLString fenia;
    DLString learned;
    bool ed;

    // Weapon class (dagger) and flags (vampiric).    
    DLString wclass, wflags;
    // Weapon dices and average damage.
    int d1, d2, ave;
    // Weapon damage noun (DAMW_BITE, weapon_flags enum).
    DLString damage;

    bool result;
};
extern SearcherParam p;

int searcher_yyparse( );

bool searcher_parse(struct obj_index_data *pObj, const char *args);
#endif
