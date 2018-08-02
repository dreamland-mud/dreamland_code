/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "xmlpolymorphvariable.h"
#include "class.h"
#include "plugin.h"
#include "article.h"

class Service : public Article, public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<Service> Pointer;
    
    virtual ~Service( );

    virtual void toStream( Character *, ostringstream & ) const = 0;
    virtual bool visible( Character * ) const = 0;
    virtual bool matches( const DLString & ) const = 0;
};

#endif
