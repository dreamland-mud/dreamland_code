/* $Id: listfilter_val.h,v 1.1.2.2 2005/04/27 18:46:15 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef LISTFILTERVAL_H
#define LISTFILTERVAL_H

#include "notethread.h"

struct SValue {
    DLString str;
    int num;
    bool result;
};

#define YYSTYPE SValue

#define YY_DECL            int listfilter_yylex( struct SValue *value )
YY_DECL;

struct ListFilterParam {
    PCharacter *ch;
    int cnt, last;
    const Note *note;
    bool hidden, result;
};
extern ListFilterParam lfParam;

int listfilter_yyparse( );

bool listfilter_parse( PCharacter *ch, int cnt, int last, const Note *note,
                       bool hidden, const char *args );
#endif
