/* $Id$
 *
 * ruffina, 2004
 */
#ifndef NOTEHOOKS_H
#define NOTEHOOKS_H

#include "oneallocate.h"
#include "logstream.h"
#include "dlstring.h"
#include "plugin.h"
#include "notethread.h"

class NoteHooks : public Plugin, public OneAllocate {
public:        
    typedef ::Pointer<NoteHooks> Pointer;
    
    NoteHooks( );
    virtual ~NoteHooks( );

    virtual void initialization( );
    virtual void destruction( );

    void processNoteMessage( const NoteThread &thread, const Note &note ) const;
};

extern NoteHooks *noteHooks;

#endif
