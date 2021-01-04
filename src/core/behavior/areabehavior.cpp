/* $Id: areabehavior.cpp,v 1.1.2.1.24.1 2007/06/26 07:24:24 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "areabehavior.h"
#include "merc.h"
#include "def.h"

template class XMLStub<AreaBehavior>;

const DLString AreaBehavior::NODE_NAME = "behavior";

AreaBehavior::AreaBehavior( ) {
    area = NULL;
}

AreaBehavior::~AreaBehavior( ) {
}

void AreaBehavior::setArea( AreaIndexData *area ) {
    this->area = area;
}

void AreaBehavior::unsetArea( ) {
    area = NULL;
}
    
AreaIndexData * AreaBehavior::getArea( ) {
    return area;
}

