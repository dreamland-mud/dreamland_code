#ifndef GLOBALPROFILEDATA_H
#define GLOBALPROFILEDATA_H

#include <vector>
#include "xmlvariablecontainer.h"
#include "xmlmap.h"
#include "globalregistry.h"

template <typename ProfileData>
class GlobalProfileArray: public XMLVariableContainer, public vector<ProfileData> {
public:
    typedef ::Pointer<GlobalProfileArray> Pointer;
    typedef XMLMapBase<ProfileData> XMLProfileDataMap;
    typedef vector<ProfileData> BaseVector;
    using BaseVector::size;
    using BaseVector::resize;

    GlobalProfileArray() 
                 : registry(NULL)
    {
    }

    GlobalProfileArray(GlobalRegistryBase *reg)
                         : registry(reg)
    {
        clear();
    }

    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
        XMLProfileDataMap map;
        
        for (unsigned int i = 0; i < size(); i++) {
            const ProfileData &data = (*this)[i]; 
            if (data.isValid()) {
                DLString name(registry->getName(i));
                map[name] = data;
            }
        }
        
        return map.toXML( parent );
    }

    virtual void fromXML( const XMLNode::Pointer& parent ) 
    {
        XMLProfileDataMap map;
        typename XMLProfileDataMap::iterator i;

        map.fromXML( parent );
        
        for (i = map.begin( ); i != map.end( ); i++) {
            int index = registry->lookup(i->first);
            ProfileData &data = get(index);
            data = i->second;
        }
    }

    ProfileData & get(unsigned int index)
    {
        if (!registry || !registry->goodIndex(index))
            return ProfileData::empty; 

        if (index >= size())
            resize(index + 1);

        return BaseVector::operator [](index);
    }

    void clear( )
    {
        BaseVector::clear();

        if (registry)
            resize(registry->size());
    }

    ProfileData & operator [] (unsigned int index)
    {
        return get(index);
    }

protected:
        
    GlobalRegistryBase *registry;

private:
    const ProfileData & operator [] (unsigned int index) const
    {
        return BaseVector::operator [](index);
    }

};

#endif
