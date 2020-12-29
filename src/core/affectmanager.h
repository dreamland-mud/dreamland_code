#ifndef __AFFECTMANAGER_H__
#define __AFFECTMANAGER_H__

#include "oneallocate.h"
#include "dllist.h"

class Affect;

class AffectManager : public OneAllocate {
public:        
    typedef ::Pointer<AffectManager> Pointer;
    typedef DLList<Affect>  ExtractList;

    AffectManager( );
    virtual ~AffectManager( );
        
    static void extract(Affect *paf);
    static Affect* getAffect();
    
    static inline AffectManager* getThis()
    {
        return thisClass;
    }
    
private:
    static AffectManager* thisClass;
    static ExtractList extractList;
};

#endif // __AFFECTMANAGER_H__