/* $Id: xmlreftest.cpp,v 1.1.4.1.6.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "xmlstreamable.h"
#include "xmlreftest.h"
#include "class.h"

int main()
{
    Class::regMoc<XMLRefTestTarget>();
    Class::regMoc<XMLRefTestMaster>();
    Class::regMoc<XMLRefTestSlave>();

    XMLPersistent<XMLRefTestMaster> m;
    XMLPersistent<XMLRefTestSlave> s;

    m.construct( );
    s.construct( );

    m->ref = new XMLRefTestTarget();
    s->ref = m->ref;

    cout << sizeof(m) << endl;

    s.backup( );
    m.backup( );

    m.recover( );
    s.recover( );
}
