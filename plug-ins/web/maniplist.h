#ifndef MANIPLIST_H
#define MANIPLIST_H

#include <list>
#include "command.h"

struct Manip {
    Manip( const DLString &cmdName, const DLString &args );
    DLString toString( ) const;

    DLString cmdName;
    Command::Pointer cmd;
    DLString args;
};


struct ManipList {
    virtual ~ManipList( );
    DLString toString( ) const;
    virtual DLString getID( ) const = 0;

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
