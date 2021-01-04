/* $Id$
 *
 * ruffina, 2019
 */
#ifndef SEARCHERVAL_H
#define SEARCHERVAL_H

#include "dlstring.h"

struct obj_index_data;
struct mob_index_data;

struct SValue {
    DLString str;
    int num;
    bool result;
};

#define YYSTYPE SValue

#define YY_DECL            int searcher_yylex( struct SValue *value )
YY_DECL;

struct SearcherParam {
    void init();

    // Whether the query was a success.
    bool result;

    /*
     * Common fields for mobs and items.
     */
    // Short description in all grammar cases plus item/mob names.    
    DLString name;
    int vnum, level;
    int hr, dr, hp, mana, move, saves, armor, size;
    int str, inta, wis, dex, con, cha;
    int heal_gain, mana_gain;
    DLString aff, vuln, res, imm, det;
    DLString material;
    // Names of all attached triggers.
    DLString fenia;
    // Weapon or mobile damage noun (DAMW_BITE, weapon_flags enum).
    DLString damage;

    /*
     * Fields specific to mobiles.
     */

    struct mob_index_data *pMob;
    DLString form, parts;
    DLString act, off;
    int group;
    DLString spec;
    int alignment;
    DLString race, sex, number;
    
    /*
     * Fields specific to items.    
     */

    struct obj_index_data *pObj;
    int limit;
    DLString extra;
    DLString wear;
    DLString itemtype;
    int age;
    int slevel;
    // Which skills and groups it improves the knowledge of.
    DLString learned;
    // Whether an item has extra descriptions.
    bool ed;

    /*
     * Fields specific to weapons.
     */

    // Weapon class (dagger) and flags (vampiric).    
    DLString wclass, wflags;
    // Weapon dices and average damage.
    int d1, d2, ave;
};
extern SearcherParam p;

int searcher_yyparse( );

bool searcher_parse(struct obj_index_data *pObj, const char *args);
bool searcher_parse(struct mob_index_data *pMob, const char *args);

#endif
