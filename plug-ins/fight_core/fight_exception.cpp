/* $Id$
 *
 * ruffina, 2004
 */
#include "fight_exception.h"

VictimDeathException::VictimDeathException()
    : Exception("victim is dead")
{

}

VictimDeathException::~VictimDeathException( ) 
{
}

