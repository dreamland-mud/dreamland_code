/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __DLXMLLOADER_H__
#define __DLXMLLOADER_H__

#include "xmlloader.h"
#include "xmltableloader.h"

class DLXMLLoader : public virtual XMLLoader {
public:
    virtual DLString getTablePath( ) const;
};

class DLXMLTableLoader : public virtual XMLTableLoader {
public:
    virtual DLString getTablePath( ) const;
};

class DLXMLRuntimeLoader : public virtual XMLLoader {
public:
    virtual DLString getTablePath( ) const;
};

class DLXMLRuntimeTableLoader : public virtual XMLTableLoader {
public:
    virtual DLString getTablePath( ) const;
};

#endif
