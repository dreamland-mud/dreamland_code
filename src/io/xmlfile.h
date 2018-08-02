/* $Id: xmlfile.h,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#ifndef __XMLFILE_H__
#define __XMLFILE_H__

#include "dlstring.h"
#include "dlfile.h"

class XMLVariable;

class XMLFile {
public:
    XMLFile( );
    XMLFile( const DLString &, const DLString &, XMLVariable * );
    XMLFile( const DLFile &, const DLString &, XMLVariable * );
    virtual ~XMLFile( );

    bool load( );
    bool save( ) const;
    
    inline void setFileName( const DLString & );
    inline void setVariable( XMLVariable * );
    inline void setNodeName( const DLString & );
    inline void setFile( const DLFile& );
    
private:
    DLString filename;
    DLString nodename;
    XMLVariable *variable;
};

inline void XMLFile::setFileName( const DLString &f )
{
    filename = f;
}

inline void XMLFile::setVariable( XMLVariable *var ) 
{
    variable = var;
}

inline void XMLFile::setNodeName( const DLString &n )
{
    nodename = n;
}

inline void XMLFile::setFile( const DLFile& file )
{
    filename = file.getPath( );
}

#endif
