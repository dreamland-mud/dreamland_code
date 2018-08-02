/* $Id$
 *
 * ruffina, 2004
 */
#include "article.h"
#include "npcharacter.h"

Article::~Article( )
{
}

void Article::sell( Character *, NPCharacter * )
{
}

bool Article::sellable( Character * )
{
    return false;
}

