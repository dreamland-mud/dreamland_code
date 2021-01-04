/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __XMLPCSTRINGEDITOR_H__
#define __XMLPCSTRINGEDITOR_H__

#include "xmlstring.h"
#include "xmled.h"
#include "xmlattribute.h"
#include "xmlvariablecontainer.h"
#include "webeditor.h"

class Descriptor;

class XMLPCStringEditor: public XMLEditor
{
public:
    virtual Descriptor *getOwner( ) = 0;

    virtual void print(const std::string&);
    virtual void error(const std::string&);
    virtual string shell(const string &, const string &);
    virtual reg_t &registerAt(char);

    void prompt( Descriptor *d );
    
    reg_t defaultReg;
};

class XMLAttributeEditorState : public AttributeEventHandler<WebEditorSaveArguments>,
                                public XMLVariableContainer
{
XML_OBJECT
public:
    XML_VARIABLE XMLEditorRegisters regs;

    bool handle( const WebEditorSaveArguments &args );
};

#endif
