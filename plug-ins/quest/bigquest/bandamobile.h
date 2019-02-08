#ifndef BANDAMOBILE_H
#define BANDAMOBILE_H

#include "mobquestbehavior.h"
#include "objquestbehavior.h"

class BandaMobile : public ConfiguredMobile {
XML_OBJECT
public:
    BandaMobile();
    virtual bool death( Character * );
    virtual void show( Character *victim, std::basic_ostringstream<char> &buf );

protected:
    virtual void config( PCharacter * );

    XML_VARIABLE XMLBooleanNoFalse configured;
};

class BandaItem : public PersonalItem {
XML_OBJECT
public:
    typedef ::Pointer<BandaItem> Pointer;
    virtual ~BandaItem();
protected:
    virtual void getByHero( PCharacter * );
    virtual void getByOther( Character * );
};

#endif
