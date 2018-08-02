/* $Id: kidnapquestregistrator.cpp,v 1.1.2.5.10.2 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */

#include "kidnapquestregistrator.h"

KidnapQuestRegistrator * KidnapQuestRegistrator::thisClass = NULL;

KidnapQuestRegistrator::KidnapQuestRegistrator( ) 
{
    thisClass = this;
}

KidnapQuestRegistrator::~KidnapQuestRegistrator( ) 
{
    thisClass = NULL;
}

