/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OLCSAVE_AREA_H__
#define __OLCSAVE_AREA_H__

#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlflags.h"
#include "xmlpointer.h"
#include "xmlmap.h"
#include "xmllist.h"
#include "xmlvariablecontainer.h"

#include "xmlmobilefactory.h"
#include "xmlobjectfactory.h"
#include "xmlroom.h"
#include "xmlareahelp.h"

struct AreaIndexData;

class XMLAreaHeader : public XMLVariableContainer {
XML_OBJECT
public:
    XMLAreaHeader( );

    void init(AreaIndexData *);
    AreaIndexData *compat( );
        
    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void fromXML( const XMLNode::Pointer& );
    
    XML_VARIABLE XMLStringNoEmpty name, credits, authors, altname, translator, speedwalk;
    XML_VARIABLE XMLIntegerNoEmpty security;
    XML_VARIABLE XMLInteger vnumHigh, vnumLow;
    XML_VARIABLE XMLInteger levelHigh, levelLow;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLOptionalString resetMessage;
    XML_VARIABLE XMLStringNode behavior;
    bool loaded;
};

struct AreaIndexData;

typedef XMLListBase<XMLPersistent<HelpArticle> > XMLHelpArticles;

class XMLArea : public XMLVariableContainer {
XML_OBJECT
public:
    XMLArea( );

    void init(area_file *);
    void save(area_file *af);
    void load(const DLString &fname);
    void load_helps(AreaIndexData *);
    void load_objects(AreaIndexData *);
    void load_mobiles(AreaIndexData *);
    void load_rooms(AreaIndexData *);

    XML_VARIABLE XMLAreaHeader areadata;
    XML_VARIABLE XMLListBase<XMLAreaHelp> helps;
    XML_VARIABLE XMLMapBase<XMLMobileFactory> mobiles;
    XML_VARIABLE XMLMapBase<XMLObjectFactory> objects;
    XML_VARIABLE XMLMapBase<XMLRoom> rooms;
};

#endif
