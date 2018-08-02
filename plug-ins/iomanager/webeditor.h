/* $Id$
 *
 * ruffina, 2018
 */

#ifndef WEBEDITOR_H
#define WEBEDITOR_H

#include "xmlattribute.h"
#include "xmlattributes.h"

#include <dlstring.h>

class WebEditorSaveArguments {
public:
    WebEditorSaveArguments(const DLString &s) : text(s) {
    }

    DLString text;
};

extern template class EventHandler<WebEditorSaveArguments>;

#endif
