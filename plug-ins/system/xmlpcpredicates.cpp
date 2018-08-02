/* $Id: xmlpcpredicates.cpp,v 1.1.2.2 2008/02/24 17:26:57 rufina Exp $
 *
 * ruffina, 2004
 */
 
#include "xmlpcpredicates.h"
#include "logstream.h"
#include "race.h"
#include "pcharacter.h"

#include "merc.h"

/*---------------------------------------------------------------------------
 * XMLStringPredicate
 *--------------------------------------------------------------------------*/
void XMLStringPredicate::fromXML( const XMLNode::Pointer & parent ) throw (ExceptionBadType) 
{
    XMLString::fromXML( parent );
    XMLPredicate::fromXML( parent );
}

bool XMLStringPredicate::toXML( XMLNode::Pointer& parent ) const 
{
    XMLString::toXML( parent );
    XMLPredicate::toXML( parent );
    return true;
}

bool XMLStringPredicate::eval( DLObject * arg ) const 
{
    return invert ^ (getValue( ) == getString( arg ));
}

/*---------------------------------------------------------------------------
 * XMLPCStringPredicate
 *--------------------------------------------------------------------------*/
DLString XMLPCStringPredicate::getString( DLObject *arg ) const
{
    DLString *str = dynamic_cast<DLString *>( arg );
    
    if (str)
	return *str;
    else {
	PCharacter *pch = dynamic_cast<PCharacter *>( arg );

	if (!pch) {
	    LogStream::sendError( ) << getType( ) << ": passed argument of invalid type " << typeid( arg ).name( ) << endl;
	    return DLString::emptyString;
	}
	
	return getString( pch );
    }
}

/*---------------------------------------------------------------------------
 * type strings 
 *--------------------------------------------------------------------------*/
const DLString XMLPCClassPredicate::TYPE = "XMLPCClassPredicate";
const DLString XMLPCRacePredicate::TYPE  = "XMLPCRacePredicate";
const DLString XMLPCAlignPredicate::TYPE = "XMLPCAlignPredicate";
const DLString XMLPCEthosPredicate::TYPE = "XMLPCEthosPredicate";
const DLString XMLPCSexPredicate::TYPE   = "XMLPCSexPredicate";

/*---------------------------------------------------------------------------
 *  impl
 *--------------------------------------------------------------------------*/
DLString XMLPCClassPredicate::getString( PCharacter *pch ) const 
{
    return pch->getProfession( )->getName( );
}

DLString XMLPCRacePredicate::getString( PCharacter *pch ) const 
{
    return pch->getRace( )->getName( );
}

DLString XMLPCAlignPredicate::getString( PCharacter *pch ) const 
{
    return align_table.name( ALIGNMENT(pch) );
}

DLString XMLPCEthosPredicate::getString( PCharacter *pch ) const 
{
    return ethos_table.name( pch->ethos );
}

DLString XMLPCSexPredicate::getString( PCharacter *pch ) const 
{
    return sex_table.name( pch->getSex( ) );
}

