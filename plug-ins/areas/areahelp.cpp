/* $Id$
 *
 * ruffina, 2004
 */
#include "areahelp.h"

AreaHelpArticle::AreaHelpArticle( const HelpArticle &art )
{
    level = art.getLevel( );
    fullKeyword = art.getKeyword( );
    assign( art.c_str( ) );
    areafile = art.areafile;
}

AreaHelpArticle::AreaHelpArticle( )
{
}

AreaHelpArticle::~AreaHelpArticle( )
{
}

const DLString XMLAreaHelpArticle::TYPE = "XMLAreaHelpArticle";

XMLAreaHelpArticle::XMLAreaHelpArticle( )
{
}

XMLAreaHelpArticle::XMLAreaHelpArticle( const HelpArticle &art )
                      : AreaHelpArticle( art )
{
}

XMLAreaHelpArticle::~XMLAreaHelpArticle( )
{
}


bool XMLAreaHelpArticle::toXML( XMLNode::Pointer &parent ) const
{
    XMLStringNoEmpty xmlString( *this );

    if (!xmlString.toXML( parent ))
        return false;
    
    if (!getKeyword( ).empty( ))
	parent->insertAttribute( ATTRIBUTE_KEYWORD, getKeyword( ) );

    if (level >= -1)
	parent->insertAttribute( ATTRIBUTE_LEVEL, DLString( level ) );
    
    if (!ref.empty( ))
	parent->insertAttribute( ATTRIBUTE_REF, ref.toString( ) );

    if (!refby.empty( ))
	parent->insertAttribute( ATTRIBUTE_REFBY, refby.toString( ) );

    return true;
}

