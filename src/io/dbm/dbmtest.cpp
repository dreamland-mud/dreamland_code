/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */


#include <iostream>

#include <logstream.h>

#include "dbmio.h"

using namespace std;

int
main(int argc, char **argv)
{
    DBMIO::Key key;
    DLString val;
    DBMIO db;

    if(argc < 2) {
        cout << "Usage: dbmtest <file.db>" << endl;
        return 1;
    }

    db.open(argv[1], O_RDWR);

    try {
        db.seq(key, val, R_FIRST);
        while(1) {
            cout << "[" << key << "] " << endl
                 << val << endl;
    
            db.seq(key, val, R_NEXT);
        }
    } catch(const DBMIO::EOFException &ex) {
        /*nothing*/
    } catch(const DBMIO::Exception &ex) {
        cout << ex.what() << endl;
    }

    db.close();
}

