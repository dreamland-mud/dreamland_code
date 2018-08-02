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

    virtual void croak(const Register &key, const ::Exception &e) const {
	Register prog;

	if (triggerFunction(key, prog)) {
	    const DLString &author = prog.toFunction( )->getFunction()->source.source->author;
	    PCMemoryInterface *pcm = PCharacterManager::find( author );

	    if (pcm && pcm->isOnline( ))
		pcm->getPlayer( )->printf(
			"{CТихий голос из хрустального шара фенера: {WИсключение при вызове %s:{x\n%s\n",
			key.toString( ).c_str( ),
			e.what( ) );
                        
	}

        WrapperBase::croak(key, e);
    }
};
// MOC_SKIP_END

#define ROOM_VNUM_FENIA_STORAGE 9

#endif
