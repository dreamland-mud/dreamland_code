#ifndef COMMANDELEMENT_H
#define COMMANDELEMENT_H

#include "xmltableelement.h"
#include "wrappedcommand.h"

/** A command that gets instantiated when all XML profiles from a folder
 * are loaded by a XMLTableLoaderPlugin.  
 * Can be created via 'cmdedit create' and doesn't need a plugin.
 * An instance of table loader plugin loading all from 'commands/fenia' folder
 * is defined in 'feniaroot' plugin.
 */
class CommandElement : public WrappedCommand, public XMLTableElement {
XML_OBJECT
public:
    typedef ::Pointer<CommandElement> Pointer;

    virtual void run(Character *, const DLString &);
    virtual bool saveCommand() const;

    virtual void loaded();
    virtual void unloaded();
    virtual const DLString & getName() const;
    virtual void setName(const DLString &);
};

#endif