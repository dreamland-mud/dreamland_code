/* $Id$
 *
 * ruffina, 2004
 */

#include <sstream>
#include <fstream>

#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "logstream.h"
#include "plugininitializer.h"
#include "commandtemplate.h"
#include "xmlstreamable.h"
#include "dlfileop.h"

#include "profiler.h"
#include "object.h"
#include "npcharacter.h"
#include "loadsave.h"
#include "comm.h"

#include "xmlarea.h"
#include "xmlroom.h"
#include "xmlobjectfactory.h"
#include "xmlmobilefactory.h"
#include "room.h"
#include "dreamland.h"
#include "mercdb.h"

using namespace std;

void
load_xml_areas( )
{
    ProfilerBlock prof("load xml areas");
    DLFileRead areaListFile( dreamland->getAreaDir( ), dreamland->getAreaListFile( ), ".xml" );
    ifstream is(areaListFile.getPath( ).c_str( ));
    
    if(!is)
        return;
    
    XMLDocument::Pointer doc(NEW);
    doc->load(is);
    
    XMLNode::Pointer node = doc->getFirstNode( );
    if(!node)
        return;
    
    XMLListBase<XMLString> lst;
    lst.fromXML(node);
    
    XMLListBase<XMLString>::iterator it;
    
    for(it = lst.begin( ); it != lst.end( ); it++) {
        try {
            XMLArea a;
            a.load(it->getValue( ));
        } catch(const Exception &e) {
            LogStream::sendError() << "load_xml_area: " << e.what( ) << endl;
            throw e;
        }
    }
}

/**
 * De-register area help articles on shutdown. Otherwise objects of AreaHelp type 
 * are still going to be present in HelpManager and will cause segmentation fault
 * in its destructor.
 */
void unload_xml_helps()
{
    const HelpArticles &articles = helpManager->getArticles( );
    list<HelpArticle::Pointer> articlesToRemove;

    for (HelpArticles::const_iterator a = articles.begin( ); a != articles.end( ); a++) 
        if ((*a)->areafile)
            articlesToRemove.push_back(*a);

    for (list<HelpArticle::Pointer>::iterator a = articlesToRemove.begin(); a != articlesToRemove.end(); a++)
        helpManager->unregistrate(*a);
}

class XMLAreaLoadTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<XMLAreaLoadTask> Pointer;

    virtual void run( ) {
        if (DLScheduler::getThis()->getCurrentTick( ) == 0) 
            load_xml_areas( );
    }

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 5;
    }

    virtual void destruction() {
        if (dreamland->isShutdown()) 
            unload_xml_helps();
    }
};

PluginInitializer<XMLAreaLoadTask> initLoadXMLAreas;
