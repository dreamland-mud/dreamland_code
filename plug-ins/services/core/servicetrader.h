/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SERVICETRADER_H__
#define __SERVICETRADER_H__

#include "xmllist.h"

#include "service.h"
#include "trader.h"

typedef XMLPointer<Service> XMLService;

class ServiceTrader : public virtual Trader {
XML_OBJECT
public:
    typedef XMLListBase<XMLService> ServiceList;
    typedef ::Pointer<ServiceTrader> Pointer;
    
protected:
    virtual Article::Pointer findArticle( Character *, DLString & );
    virtual void toStream( Character *, ostringstream & );

    XML_VARIABLE ServiceList services;    
};

#endif
