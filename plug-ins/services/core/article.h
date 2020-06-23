/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ARTICLE_H__
#define __ARTICLE_H__

#include <sstream>

#include "dlobject.h"

class Character;
class NPCharacter;
class DLString;

class Article : public virtual DLObject {
public:
    typedef ::Pointer<Article> Pointer;
    
    virtual ~Article( );

    virtual bool purchase( Character *, NPCharacter *, const DLString &, int = 1 ) = 0;
    virtual bool available( Character *, NPCharacter * ) const = 0;
    virtual int getQuantity( ) const = 0;

    virtual bool sell( Character *, NPCharacter * ); 
    virtual bool sellable( Character * );
};

#endif
