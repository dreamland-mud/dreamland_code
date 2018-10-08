/* $Id: main.cpp,v 1.2.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: main.cpp,v 1.2.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <iostream>
#include <fstream>
#include <sys/time.h>

using namespace std;

#include <xmldocument.h>

#include "scope.h"
#include "lex.h"
#include "reference-impl.h"
#include "register-impl.h"
#include "impl.h"

#include "test.h"

using namespace Scripting;

int
main(int argc, char **argv)
{
#if 1
    try {
        TestManager::dbEnv.open( "db" );
        fenia_init();

        Test test;
        test.load();
        
        CodeSource &cs = CodeSource::manager->allocate( );
        Register r(DLString("zz"));
        cout << "sizeof(r): " << sizeof(r) << endl; 
        cs.name = "test";
        cs.author = "root";
        cs.content = "function() { var x, y; x = 42; y = function() { return x; }; return y; }";
        //cs.content = "\"???\"";
        Register rc = cs.eval( );
        
        RegisterList ol;
//        struct timeval tv1, tv2;
//        gettimeofday(&tv1, NULL);
        rc = rc.toFunction()->invoke(Register(), ol);
//        gettimeofday(&tv2, NULL);
        std::cout << rc.toString() << endl;
/*        
        long long l;
        l = tv2.tv_sec - tv1.tv_sec;
        l *= 1000000;
        l += tv2.tv_usec - tv1.tv_usec;;
        cout << "invocation time: " << l << endl;
*/
        test.save();
        TestManager::dbEnv.close( );
    } catch (const exception &e) {
        cout << "exception: " << e.what() << endl;
    }
#endif
    return 0;
}
