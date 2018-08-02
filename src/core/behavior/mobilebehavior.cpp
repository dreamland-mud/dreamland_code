/* $Id: mobilebehavior.cpp,v 1.1.2.2.10.2 2007/09/22 23:13:42 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "mobilebehavior.h"
#include "npcharacter.h"
#include "object.h"
#include "def.h"

template class XMLStub<MobileBehavior>;

const DLString MobileBehavior::NODE_NAME = "behavior";

MobileBehavior::MobileBehavior( ) : ch( NULL )
{
}

MobileBehavior::~MobileBehavior( ) 
{
}

void MobileBehavior::setChar( NPCharacter *mob ) 
{
    ch = mob;
}

void MobileBehavior::unsetChar( ) 
{
}
    
NPCharacter * MobileBehavior::getChar( ) 
{
    return ch;
}

bool MobileBehavior::extract( bool ) 
{
    ch = NULL; 
    return false;
}

bool MobileBehavior::command( Character *, const DLString &, const DLString & ) 
{
    return false;
}

