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

class WrapperBase;

class XMLIndexData {
public:
    virtual int getVnum() const = 0;
    virtual area_data * getArea() const = 0;
    virtual const char * getIndexType() const = 0;
};

class XMLRoomIndexData : public XMLIndexData {
public:
    XMLRoomIndexData(Room *room);

    virtual int getVnum() const;
    virtual area_data * getArea() const;
    virtual const char * getIndexType() const;

protected:
    Room *room;    
};

class XMLMobIndexData : public XMLVariable, public mob_index_data, public XMLIndexData {
public:
    XMLMobIndexData();
    XMLMobIndexData(const mob_index_data &);
    virtual ~XMLMobIndexData();

    void clear();

    virtual void fromXML( const XMLNode::Pointer& node ) ;
    virtual bool toXML( XMLNode::Pointer& node ) const;
    virtual int getVnum() const;
    virtual area_data * getArea() const;
    virtual const char * getIndexType() const;
};

class XMLObjIndexData : public XMLVariable, public obj_index_data, public XMLIndexData {
public:
    XMLObjIndexData();
    XMLObjIndexData(const obj_index_data &);
    virtual ~XMLObjIndexData();

    void clear();

    virtual void fromXML( const XMLNode::Pointer& node ) ;
    virtual bool toXML( XMLNode::Pointer& node ) const;
    virtual int getVnum() const;
    virtual area_data * getArea() const;
    virtual const char * getIndexType() const;
};

#endif

