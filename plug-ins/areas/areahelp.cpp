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
    XMLString::toXML(parent);

    if (!keywordAttribute.empty( ))
        parent->insertAttribute( HelpArticle::ATTRIBUTE_KEYWORD, keywordAttribute );

    if (!titleAttribute.empty())
        parent->insertAttribute(HelpArticle::ATTRIBUTE_TITLE, titleAttribute);

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
    XMLString::fromXML(parent);
    keywordAttribute = parent->getAttribute( HelpArticle::ATTRIBUTE_KEYWORD );
    titleAttribute = parent->getAttribute(HelpArticle::ATTRIBUTE_TITLE);
    labels = parent->getAttribute(HelpArticle::ATTRIBUTE_LABELS);
    parent->getAttribute( HelpArticle::ATTRIBUTE_LEVEL, level );
    parent->getAttribute(HelpArticle::ATTRIBUTE_ID, id);
}

/*-------------------------------------------------------------------
 * AreaHelp 
 *------------------------------------------------------------------*/
const DLString AreaHelp::TYPE = "AreaHelp";

void AreaHelp::save() const
{
    if (areafile)
        SET_BIT(areafile->area->area_flag, AREA_CHANGED);
}

DLString AreaHelp::getTitle(const DLString &label) const
{
    ostringstream buf;
    AREA_DATA *area = areafile->area;

    if (!label.empty() || !titleAttribute.empty() || !selfHelp)
        return MarkupHelpArticle::getTitle(label);

    buf << "Зона {c" << area->name << "{x";

    if (strlen(area->credits) > 0 
            && str_str(area->credits, area->name) == 0
            && str_str(area->name, area->credits) == 0)
        buf << " ({c" << area->credits << "{x)";

    return buf.str();
}

void AreaHelp::getRawText( Character *ch, ostringstream &in ) const
{
    AREA_DATA *area = areafile->area;
    
    if (!selfHelp) {
        MarkupHelpArticle::getRawText(ch, in);
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
            DLString aname(area->name);
            aname.colourstrip();
            
            for (a = articles.begin( ); a != articles.end( ); a++) {
                a->recover();
                AreaHelp *help = a->getDynamicPointer<AreaHelp>();
                help->areafile = area->area_file;
                if (help->getKeywordAttribute().empty()) {
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


