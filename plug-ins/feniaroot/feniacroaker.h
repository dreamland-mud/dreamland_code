#ifndef FENIA_CROAKER_H
#define FENIA_CROAKER_H

#include "feniamanager.h"
#include "plugin.h"

/** 
 * A loggger class that plugs itself into the FeniaManager and provides 
 * more detailed 'crystal orb' exception reporting to all online feners.
 */
class FeniaCroaker : public FeniaCroakerBase, public Plugin {
public:
    typedef ::Pointer<FeniaCroaker> Pointer;

    virtual void initialization();
    virtual void destruction();
    virtual void croak(const FeniaProcess *process, const ::Exception &e) const;
    virtual void croak(const WrapperBase *wrapper, const Scripting::Register &key, const ::Exception &e) const;

private:
    static void wiznet(const DLString &message);
    static bool isFiltered(const ::Exception &e);
};

#endif
