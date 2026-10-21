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

/* Seconds a telnet login greeting waits for the client's TTYPE answer. */
#define GREETING_WAIT 2

class NannyHandler : public InputHandler, 
                     public NativeHandler,
                     public PluginNativeImpl<NannyHandler> 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<NannyHandler> Pointer;

    virtual void setSelf( Scripting::Object * );
    virtual Scripting::Object *getSelf() const { return 0; }
    
    virtual int handle(Descriptor *d, char *arg);
    virtual void prompt(Descriptor *d);
    virtual void close( Descriptor * );
    
    static void init( Descriptor * );
    static void initRemort( Descriptor * );
    static void greet( Descriptor * );

    static void doGreeting( Descriptor * );
    void doPlace( Descriptor * );
    void doCodepage( Descriptor *, char * );
    void doInterpret( Descriptor *, char * );

    static void extractNewbie(Character *ch);

private:
    static PCharacter * getPlayer( const Scripting::RegisterList & );
    static PCharacter * getPlayer( const Scripting::Register & );
    static Character * getCharacter( const Scripting::RegisterList & );
    static Scripting::Register resolve(Descriptor *d);
    static bool invoke( Scripting::IdRef &, Descriptor *, Scripting::RegisterList );
};

#endif
