/* $Id$
 *
 * ruffina, 2004
 */
#ifndef PLUGINWRAPPERIMPL_H
#define PLUGINWRAPPERIMPL_H

#include <sstream>

#include "codesource.h"
#include "pluginnativeimpl.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "dreamland.h"

#ifdef _never_defined_parsed_only_by_moc_
using namespace Scripting;
template <typename T>
class PluginWrapperImpl : public PluginNativeImpl<T>, public WrapperBase { }
#endif

void fenia_wiznet(const WrapperBase *wrapper, const Register &key, const ::Exception &e);

// MOC_SKIP_BEGIN
template <typename T>
class PluginWrapperImpl : public PluginNativeImpl<T>, 
                          public WrapperBase 
{                        
public:
    typedef PluginWrapperImpl<T> GutsContainer;

    void traitsAPI( ostringstream& buf ) const {
        Guts::const_iterator i;
        
        buf << endl << endl << "{WRuntime fields:{x" << endl;
        for (i = guts.begin(); i != guts.end(); i++)
            buf << "{x" << Lex::getThis()->getName(i->first) << "{x" << endl;
    }

    virtual void croak(const Register &key, const ::Exception &e) const 
    {
        fenia_wiznet(this, key, e);
        WrapperBase::croak(key, e);
    }
};
// MOC_SKIP_END

#define ROOM_VNUM_FENIA_STORAGE 9

#endif
