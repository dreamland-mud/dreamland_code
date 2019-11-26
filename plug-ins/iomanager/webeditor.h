/* $Id$
 *
 * ruffina, 2018
 */

#ifndef WEBEDITOR_H
#define WEBEDITOR_H

#include "xmlattribute.h"
#include "xmlattributes.h"

#include <dlstring.h>

class PCharacter;

class WebEditorSaveArguments {
public:
    WebEditorSaveArguments(PCharacter *ch, const DLString &s) 
            : pch(ch), text(s) 
    {
    }

    PCharacter *pch;
    DLString text;
};

extern template class EventHandler<WebEditorSaveArguments>;

#endif
