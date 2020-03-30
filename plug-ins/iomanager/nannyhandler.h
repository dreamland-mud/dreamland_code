/* $Id$
 *
 * ruffina, 2004
 */
#ifndef NANNYHANDLER_H
#define NANNYHANDLER_H

#include "inputhandler.h"
// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginnativeimpl.h"

class Character;
class PCharacter;
class Descriptor;
using Scripting::NativeHandler;

class NannyHandler : public InputHandler, 
                     public NativeHandler,
                     public PluginNativeImpl<NannyHandler> 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<NannyHandler> Pointer;

    virtual void setSelf( Scripting::Object * );

    virtual int handle(Descriptor *d, char *arg);
    virtual void prompt(Descriptor *d);
    virtual void close( Descriptor * );
    
    static void init( Descriptor * );
    static void initRemort( Descriptor * );

    void doGreeting( Descriptor * );
    void doPlace( Descriptor * );
    void doCodepage( Descriptor *, char * );
    void doInterpret( Descriptor *, char * );

private:
    static PCharacter * getPlayer( const Scripting::RegisterList & );
    static PCharacter * getPlayer( const Scripting::Register & );
    static Character * getCharacter( const Scripting::RegisterList & );
    static Scripting::Register resolve(Descriptor *d);
    static bool invoke( Scripting::IdRef &, Descriptor *, Scripting::RegisterList );
};

#endif
