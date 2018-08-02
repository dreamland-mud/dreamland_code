/* $Id$
 *
 * ruffina, 2004
 */
#ifndef HELPFORMATTER_H
#define HELPFORMATTER_H

#include <sstream>
#include "dlstring.h"
#include "dlobject.h"

class Character;

class HelpFormatter : public virtual DLObject {
public:
    typedef ::Pointer<HelpFormatter> Pointer;
    
    HelpFormatter( );
    virtual ~HelpFormatter( );

    virtual void run( Character *, ostringstream & );

protected:
    virtual bool handleKeyword( const DLString &, ostringstream & );
    virtual void setup( Character * );
    virtual void reset( );

    bool fParse;
    const char *text;
};

class DefaultHelpFormatter : public HelpFormatter {
public:
    DefaultHelpFormatter( const char * );
    virtual ~DefaultHelpFormatter( );
};

#endif
