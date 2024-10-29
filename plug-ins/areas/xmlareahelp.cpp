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
#include "string_utils.h"
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

void XMLAreaHelp::init(AreaHelp *ahelp)
{
    level = ahelp->getLevel();
    id = ahelp->getID();
    labels = ahelp->labels.persistent.toString();

    title = ahelp->title;
    extra = ahelp->extra;
    keyword = ahelp->keyword;
    text = ahelp->text;
}

AreaHelp::Pointer XMLAreaHelp::compat() const
{
    AreaHelp::Pointer help(NEW);

    help->keyword = keyword;
    help->title = title;
    help->extra = extra;
    help->text = text;
    help->setLevel(level);
    help->setID(id);
    help->labels.addPersistent(labels);

    return help;
}


class AreaHelpLifetimePlugin : public Plugin {
public:
    typedef ::Pointer<AreaHelpLifetimePlugin> Pointer;

    virtual void initialization( )
    {
        for(auto &area: areaIndexes) {
            HelpArticles::iterator a;
            HelpArticles &articles = area->helps;
        
            for (a = articles.begin( ); a != articles.end( ); a++) {
                a->recover();
                AreaHelp *help = a->getDynamicPointer<AreaHelp>();
                help->setAreaIndex(area);                
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


