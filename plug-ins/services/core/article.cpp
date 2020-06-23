/* $Id$
 *
 * ruffina, 2004
 */
#include "article.h"
#include "npcharacter.h"

Article::~Article( )
{
}

bool Article::sell( Character *, NPCharacter * )
{
    return false;
}

bool Article::sellable( Character * )
{
    return false;
}

