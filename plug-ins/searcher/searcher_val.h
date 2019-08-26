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
    DLString aff, vuln, res, imm, det;
    DLString wclass, wflags;
    int ave;
    DLString fenia;
    bool result;
};
extern SearcherParam p;

int searcher_yyparse( );

bool searcher_parse(struct obj_index_data *pObj, const char *args);
#endif
