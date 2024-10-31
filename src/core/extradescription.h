#ifndef EXTRADESCRIPTION_H
#define EXTRADESCRIPTION_H

#include "xmlmultistring.h"

/*
 * Extra description data for a room or object.
 */
struct ExtraDescription {
    // Keyword in look/examine, contains keywords in all languages.
    DLString keyword; 

    // What to see
    XMLMultiString description; 
};

struct ExtraDescrList: public list<ExtraDescription *> {
    /** Return extra descr that matches the given keyword. */
    ExtraDescription *find(const DLString &keyword) const;

    ExtraDescription *findUnstrict(const DLString &keyword) const;

    /** 
     * Remove matching descr from list and free its memory. 
     * Returns true if found.
     */
    bool findAndDestroy(const DLString &keyword);

    /** Destroy all elements and clear the list. */
    void deallocate();
};



#endif