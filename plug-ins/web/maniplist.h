#ifndef MANIPLIST_H
#define MANIPLIST_H

#include <list>
#include "command.h"
#include "lang.h"

struct Manip {
    Manip( const DLString &cmdName, const DLString &args );
    DLString toString( lang_t lang ) const;

    DLString cmdName;
    Command::Pointer cmd;
    DLString args;
};


struct ManipList {
    ManipList( ) : lang( LANG_DEFAULT ) { }
    virtual ~ManipList( );
    DLString toString( ) const;
    virtual DLString getID( ) const = 0;

    // The menu entry is a command the player will send back, so it has to be
    // spelled in their own language. Set from the viewer before serialising;
    // LANG_DEFAULT keeps the old Russian-only behaviour for any caller that does not.
    lang_t lang;

    static const DLString TAG;
    static const DLString ATTR_CMD;
    static const DLString ATTR_LOCAL;
    static const DLString THIS;

    // Main commands.
    list<Manip> manips;
    // Commands only available in this room (local).
    list<Manip> locals;
    DLString descr;

};

#endif
