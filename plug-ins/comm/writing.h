/* $Id: writing.h,v 1.1.2.4.10.2 2007/06/26 07:11:42 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef WRITING_H
#define WRITING_H

#include "commandplugin.h"
#include "defaultcommand.h"

class Object;
struct extra_descr_data;

class CWrite : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
    typedef ::Pointer<CWrite> Pointer;

    CWrite( );

    virtual void run( Character *, const DLString & );
        
private:
    void writeOnWall( Character *, Object *, DLString & );
    void writeOnPaper( Character *, Object *, DLString & );
    void usage( Character * );

    extra_descr_data * descFind( Object *, const DLString & );
    extra_descr_data * descAdd( Object *, const DLString & );
    void descFree( Object *, const DLString & );
    
    void lineAdd( extra_descr_data *, const DLString & );
    bool lineDel( extra_descr_data * );

    Object * findNail( Character * );
    
    static const DLString COMMAND_NAME;
};

#endif

