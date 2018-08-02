/* $Id: initializer.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2008
 */

#ifndef __INITIALIZER_H__
#define __INITIALIZER_H__

#define INITPRIO_NORMAL 100

class SharedObject;

class Initializer {
public:
    Initializer(int prio);
    virtual ~Initializer( );
    
    virtual void init(SharedObject *) = 0;

    int getPriority( ) const {
        return priority;
    }
private:
    int priority;
};

#endif
