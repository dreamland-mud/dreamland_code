/* $Id: writing.h,v 1.1.2.4.10.2 2007/06/26 07:11:42 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef WRITING_H
#define WRITING_H

#include "commandplugin.h"

class Object;
struct ExtraDescription;

class CWrite : public CommandPlugin {
XML_OBJECT
public:
    typedef ::Pointer<CWrite> Pointer;

    CWrite( );

    virtual void run( Character *, const DLString & );
        
private:
    void writeOnWall( Character *, Object *, DLString & );
    void writeOnPaper( Character *, Object *, DLString & );
    void usage( Character * );

    Object * findNail( Character * );
    
    static const DLString COMMAND_NAME;
};

#endif

