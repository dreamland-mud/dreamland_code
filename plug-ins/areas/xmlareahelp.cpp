/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "xmlareahelp.h"
#include "areahelp.h"
#include "so.h"
#include "plugininitializer.h"
#include "mocregistrator.h"
#include "merc.h"

#include "def.h"



/*-------------------------------------------------------------------
 * XMLAreaHelp
 *------------------------------------------------------------------*/
XMLAreaHelp::XMLAreaHelp()
                : level(-1), id(-1)
{
}

bool XMLAreaHelp::toXML( XMLNode::Pointer& parent ) const
{
    XMLVariableContainer::toXML(parent);

    if (level >= -1)
        parent->insertAttribute( HelpArticle::ATTRIBUTE_LEVEL, DLString( level ) );

    if (!labels.empty())
        parent->insertAttribute(HelpArticle::ATTRIBUTE_LABELS, labels);

    if (id > 0)
        parent->insertAttribute(HelpArticle::ATTRIBUTE_ID, DLString(id));

    return true;    
}

void XMLAreaHelp::fromXML( const XMLNode::Pointer&parent )
{
    XMLVariableContainer::fromXML(parent);    

    labels = parent->getAttribute(HelpArticle::ATTRIBUTE_LABELS);
    parent->getAttribute( HelpArticle::ATTRIBUTE_LEVEL, level );
    parent->getAttribute(HelpArticle::ATTRIBUTE_ID, id);
}


class AreaHelpLifetimePlugin : public Plugin {
public:
    typedef ::Pointer<AreaHelpLifetimePlugin> Pointer;

    virtual void initialization( )
    {
        for(auto &area: areaIndexes) {
            HelpArticles::iterator a;
            HelpArticles &articles = area->helps;
            DLString aname(area->getName());
            aname.colourstrip();
            
            for (a = articles.begin( ); a != articles.end( ); a++) {
                a->recover();
                AreaHelp *help = a->getDynamicPointer<AreaHelp>();
                help->areafile = area->area_file;
                if (help->keyword.get(RU).empty()) {
                    help->persistent = false;
                    help->selfHelp = true;
                    help->addAutoKeyword(aname.quote());
                    help->addAutoKeyword(DLString(area->credits).colourStrip().quote());
                }
                else {
                    help->persistent = true;
                    help->selfHelp = is_name(aname.c_str(), (*a)->getAllKeywordsString().c_str());
                }
                if (help->selfHelp) 
                    help->labels.addTransient("area");
                
                helpManager->registrate( *a );
            }
        }
    }

    virtual void destruction( )
    {
        for(auto &area: areaIndexes) {
            HelpArticles::iterator a;
            HelpArticles &articles = area->helps;

            for (a = articles.begin( ); a != articles.end( ); a++) {
                helpManager->unregistrate(*a);
                a->backup();
            }
        }

    }

    // Used for debugging
    static void toStream(XMLPersistent<HelpArticle> &a) 
    {
        XMLNode::Pointer node( NEW );
        XMLDocument::Pointer root( NEW );

        if (a.toXML( node )) {
            node->setName( "helps" );
            root->appendChild( node );
            root->save( LogStream::sendNotice() );
        }
    }
};

extern "C" {

    SO::PluginList initialize_areas( ) 
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<MocRegistrator<QuestStep>>(ppl);
        Plugin::registerPlugin<MocRegistrator<AreaQuest>>(ppl);
        Plugin::registerPlugin<AreaHelpLifetimePlugin>( ppl );
        return ppl;
    }
}


