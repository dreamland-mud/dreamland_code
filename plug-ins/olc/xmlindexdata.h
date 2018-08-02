/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __XMLINDEXDATA_H__
#define __XMLINDEXDATA_H__

#include "xmlvariable.h"
#include "merc.h"

int isreadfn(void *, char *, int);
int oswritefn(void *, const char *, int);

class XMLMobIndexData : public XMLVariable, public mob_index_data {
public:
    XMLMobIndexData();
    XMLMobIndexData(const mob_index_data &);
    virtual ~XMLMobIndexData();

    void clear();

    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
    virtual bool toXML( XMLNode::Pointer& node ) const;
};

class XMLObjIndexData : public XMLVariable, public obj_index_data {
public:
    XMLObjIndexData();
    XMLObjIndexData(const obj_index_data &);
    virtual ~XMLObjIndexData();

    void clear();

    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
    virtual bool toXML( XMLNode::Pointer& node ) const;
};

#endif

