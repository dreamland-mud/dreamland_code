/* $Id: globalarray.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#ifndef GLOBALARRAY_H
#define GLOBALARRAY_H

#include <vector>
#include "globalregistry.h"

class GlobalBitvector;

class GlobalArray : public vector<int> {
public:
    static const GlobalArray emptyArray;
    
    GlobalArray( );
    GlobalArray( GlobalRegistryBase * );
    virtual ~GlobalArray( );
    
    void clear( );
    int & operator [] (size_type);
    void applyBitvector(const GlobalBitvector &bitvector, int modifier);

protected:
    GlobalRegistryBase *registry;

private:
    const int & operator [] (size_type) const;
};


#endif
