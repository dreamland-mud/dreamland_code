/* Dreamland trilinguality — Fenia wrapper carrying a MultiMessage.
 * Wave 3 / Trello 2594.
 *
 * Produced by the Fenia `._(msg)` call and consumed by regfmt(): an opaque,
 * transient data carrier (RU source + originating script FILE) that the output
 * path resolves to each recipient's language. Fenia scripts never read fields
 * or call methods on it, and it is never persisted, so the Handler / XML
 * interface methods are inert.
 */
#ifndef MULTIMESSAGEWRAPPER_H
#define MULTIMESSAGEWRAPPER_H

#include "fenia/handler.h"
#include "multimessage.h"

class MultiMessageWrapper : public Scripting::Handler {
public:
    typedef ::Pointer<MultiMessageWrapper> Pointer;

    MultiMessageWrapper() : self(0) { }

    void init(const DLString &ru, const DLString &file) { multi = MultiMessage(ru, file); }
    const MultiMessage & getMulti() const { return multi; }

    /* Handler interface -- opaque carrier, nothing exposed to scripts. The two
     * Register-returning methods are defined out-of-line (Register is only
     * forward-declared here; returning one by value needs the complete type). */
    virtual void setField(const Scripting::Register &, const Scripting::Register &) { }
    virtual Scripting::Register getField(const Scripting::Register &);
    virtual Scripting::Register callMethod(const Scripting::Register &, const Scripting::RegisterList &);
    virtual void setSelf(Scripting::Object *s) { self = s; }
    virtual Scripting::Object *getSelf() const { return self; }

    /* XMLPolymorphVariable / XMLVariable / AllocateClass -- never persisted. */
    virtual const DLString & getType() const { return TYPE; }
    virtual void fromXML(const XMLNode::Pointer &) { }
    virtual bool toXML(XMLNode::Pointer &) const { return false; }
    virtual DLObject::Pointer set(DLObject::Pointer, DLObject::Pointer) { return DLObject::Pointer(); }

    static const DLString TYPE;

private:
    MultiMessage multi;
    Scripting::Object *self;
};

#endif
