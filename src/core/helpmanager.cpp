/* $Id$
 *
 * ruffina, 2004
 */
#include "helpmanager.h"
#include "character.h"

const DLString XMLHelpArticle::ATTRIBUTE_KEYWORD = "keyword";
const DLString XMLHelpArticle::ATTRIBUTE_LEVEL = "level";
const DLString XMLHelpArticle::ATTRIBUTE_REF = "ref";
const DLString XMLHelpArticle::ATTRIBUTE_REFBY = "refby";

HelpArticle::HelpArticle( ) 
               : areafile( NULL ),
                 level( -1 )
{
}

DLString HelpArticle::getText( Character * ) const
{
    return *this;
}

const DLString & HelpArticle::getKeyword( ) const
{
    return fullKeyword;
}

int HelpArticle::getLevel( ) const
{
    return level;
}

void HelpArticle::setLevel( int level )
{
    this->level = level;
}

void HelpArticle::setText( const DLString &text )
{
    assign( text );
}

void HelpArticle::addKeyword( const DLString &add )
{
    StringSet kwd;
    
    kwd.fromString( fullKeyword );
    kwd.fromString( add );

    fullKeyword = kwd.toString( ).toUpper( );
}

bool HelpArticle::visible( Character *ch ) const
{
    return ch->get_trust( ) >= level;
}

bool XMLHelpArticle::toXML( XMLNode::Pointer &parent ) const
{
    XMLStringNoEmpty xmlString( *this );

    if (!xmlString.toXML( parent ))
        if (keyword.empty( ) && ref.empty( ) && refby.empty( ))
            return false;
    
    if (!keyword.empty( ))
        parent->insertAttribute( ATTRIBUTE_KEYWORD, keyword );

    if (level >= -1)
        parent->insertAttribute( ATTRIBUTE_LEVEL, DLString( level ) );
    
    if (!ref.empty( ))
        parent->insertAttribute( ATTRIBUTE_REF, ref.toString( ) );

    if (!refby.empty( ))
        parent->insertAttribute( ATTRIBUTE_REFBY, refby.toString( ) );

    return true;
}

void XMLHelpArticle::fromXML( const XMLNode::Pointer &parent ) throw( ExceptionBadType )
{
    XMLStringNoEmpty xmlString;
    
    xmlString.fromXML( parent );
    assign( xmlString );

    keyword = parent->getAttribute( ATTRIBUTE_KEYWORD );
    addKeyword( keyword );

    if (parent->hasAttribute( ATTRIBUTE_LEVEL ))
        level = parent->getAttribute( ATTRIBUTE_LEVEL ).toInt( );

    ref.fromString( parent->getAttribute( ATTRIBUTE_REF ) );
    refby.fromString( parent->getAttribute( ATTRIBUTE_REFBY ) );
}

HelpManager * helpManager = NULL;

HelpManager::HelpManager( )
{
    checkDuplicate( helpManager );
    helpManager = this;
}

HelpManager::~HelpManager( )
{
    helpManager = NULL;
}


void HelpManager::registrate( HelpArticle::Pointer art )
{
    articles.push_back( art );
}

void HelpManager::unregistrate( HelpArticle::Pointer art )
{
    articles.remove( art );
}


