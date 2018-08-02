/* $Id: test.cpp,v 1.1.4.7.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: test.cpp,v 1.1.4.7.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */


#include <fstream>

using namespace std;

#include "test.h"
#include "register-impl.h"

using namespace Scripting;

extern DbEnvContext dbEnv;

NMI_INIT(Test, "test")

Test::Test()
{
    current = this;
    self = 0;
}

Test::~Test()
{
    current = 0;
}

void
Test::load()
{
    manager.open( );
    manager.load( );
    manager.recover( );
}

void
Test::save()
{
    manager.sync( 0 );
    manager.close( );
}

DbEnvContext TestManager::dbEnv;

DbEnvContext *
TestManager::getDbEnv( ) const
{
    return &dbEnv;
}

