#ifndef WORDEFFECTWRAPPER_H
#define WORDEFFECTWRAPPER_H

#include "pluginwrapperimpl.h"

class WordEffect;

class WordEffectWrapper : public PluginWrapperImpl<WordEffectWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<WordEffectWrapper> Pointer;

    WordEffectWrapper();

    virtual void setSelf(Scripting::Object *);
    void setTarget(WordEffect *);
    void checkTarget() const ;
    virtual void extract(bool);
    WordEffect *getTarget() const;

    // wrap(language, effect): locate a word-effect by its language and name and
    // return its Fenia wrapper, e.g. .WordEffect("arcadian", "water2wine").
    static Scripting::Register wrap(const DLString &language, const DLString &name);

private:
    WordEffect *target;
};

#endif
