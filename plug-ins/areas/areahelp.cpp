/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "areahelp.h"
#include "so.h"
#include "plugininitializer.h"
#include "mocregistrator.h"
#include "regexp.h"
#include "merc.h"
#include "mercdb.h"
#include "dl_strings.h"

/*-------------------------------------------------------------------
 * XMLAreaHelp
 *------------------------------------------------------------------*/
XMLAreaHelp::XMLAreaHelp()
                : level(-1)
{
}

bool XMLAreaHelp::toXML( XMLNode::Pointer& parent ) const
{
    XMLString::toXML(parent);

    if (!keyword.empty( ))
        parent->insertAttribute( HelpArticle::ATTRIBUTE_KEYWORD, keyword );

    if (level >= -1)
        parent->insertAttribute( HelpArticle::ATTRIBUTE_LEVEL, DLString( level ) );

    if (!labels.empty())
        parent->insertAttribute(HelpArticle::ATTRIBUTE_LABELS, labels);

    return true;    
}

void XMLAreaHelp::fromXML( const XMLNode::Pointer&parent ) throw( ExceptionBadType )
{
    XMLString::fromXML(parent);
    keyword = parent->getAttribute( HelpArticle::ATTRIBUTE_KEYWORD );
    labels = parent->getAttribute(HelpArticle::ATTRIBUTE_LABELS);

    if (parent->hasAttribute( HelpArticle::ATTRIBUTE_LEVEL ))
        level = parent->getAttribute( HelpArticle::ATTRIBUTE_LEVEL ).toInt( );
}

/*-------------------------------------------------------------------
 * AreaHelp 
 *------------------------------------------------------------------*/
const DLString AreaHelp::TYPE = "AreaHelp";

void AreaHelp::getRawText( Character *ch, ostringstream &in ) const
{
    AREA_DATA *area = areafile->area;
    
    if (!selfHelp) {
        in << *this;
        return;
    }

    in << "Зона {Y" << area->name << "{x, " 
       << "уровни {Y" << area->low_range << "-" << area->high_range << "{x, "
       << "автор {y" << area->authors << "{x";
    if (str_cmp(area->translator, ""))
        in << ", перевод {y" << area->translator << "{x";

    // This bit is going to be replaced with a link to the map by the webclient.
    in << "%PAUSE% {Iw[map=" << areafile->file_name << "]{Ix%RESUME%";

    in << endl
       << endl;

    if (!empty())
       in << *this << endl;
    
    if (str_cmp(area->speedwalk, "")) {
        in << "{yКак добраться{x: " << area->speedwalk << endl;

        // If 'speedwalk' field contains something resembling a run path,
        // and not just text, explain the starting point.
        RegExp speedwalkRE("[0-9]?[nsewud]+");
        if (speedwalkRE.match(area->speedwalk))
           in << "{D(все пути ведут от Рыночной Площади Мидгаарда, если не указано иначе){x" << endl;
    }

}

class AreaHelpLifetimePlugin : public Plugin {
public:
    typedef ::Pointer<AreaHelpLifetimePlugin> Pointer;

    virtual void initialization( )
    {
        struct area_data *area;

        for (area = area_first; area; area = area->next) {
            HelpArticles::iterator a;
            HelpArticles &articles = area->helps;

            for (a = articles.begin( ); a != articles.end( ); a++) {
                a->recover();
                AreaHelp *help = a->getDynamicPointer<AreaHelp>();
                help->areafile = area->area_file;
                if (help->getKeywordAttribute().empty()) {
                    help->persistent = false;
                    help->selfHelp = true;
                    help->addKeyword(DLString(area->name).colourStrip().quote());
                    help->addKeyword(DLString(area->credits).colourStrip().quote());
                }
                else {
                    help->persistent = true;
                    help->selfHelp = is_name(area->name, (*a)->getKeyword().c_str());
                }
                if (help->selfHelp) 
                    help->addLabel("area");
                
                helpManager->registrate( *a );
            }
        }
    }

    virtual void destruction( )
    {
        struct area_data *area;

        for (area = area_first; area; area = area->next) {
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
        
        Plugin::registerPlugin<MocRegistrator<AreaHelp> >( ppl );
        Plugin::registerPlugin<AreaHelpLifetimePlugin>( ppl );
        return ppl;
    }
}


