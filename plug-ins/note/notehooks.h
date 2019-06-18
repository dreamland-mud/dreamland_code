/* $Id$
 *
 * ruffina, 2018
 */
#ifndef NOTEHOOKS_H
#define NOTEHOOKS_H

#include "logstream.h"
#include "commandplugin.h"
#include "defaultcommand.h"
#include "notethread.h"

class NoteHooks : public CommandPlugin, public DefaultCommand {
public:        
    typedef ::Pointer<NoteHooks> Pointer;
    
    NoteHooks( );

    virtual void run( Character *, const DLString & );

    static void processNoteMessage( const NoteThread &thread, const Note &note );

private:
    static const DLString COMMAND_NAME;

    static void notifyOrb(const NoteThread &thread, const Note &note);
    static void hookDiscord(const NoteThread &thread, const Note &note);    
    static void webDumpNews();
    static void webDumpModernStories();
    static void webDumpOldStories();
};

#endif
