#ifndef EQUIPSET_H
#define EQUIPSET_H

#include "objectbehaviormanager.h"

class EquipSet : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<EquipSet> Pointer;
        
        EquipSet(int sn, int totalSetSize, bool, bool);    
        virtual void equip( Character *victim );                           
        virtual void remove( Character *victim );

protected:
        bool isComplete(Character *, bool verbose) const;
        virtual bool hasAffect(Character *) const = 0;
        virtual void addAffect(Character *) const = 0;
        virtual void removeAffect(Character *) const = 0;

        int sn;
        int totalSetSize;
        bool noDoubleNeck;
        bool noDoubleWrist;        
};

class SidheArmorSet : public EquipSet {
XML_OBJECT
public:
        typedef ::Pointer<SidheArmorSet> Pointer;
        SidheArmorSet();

protected:
        virtual bool hasAffect(Character *) const;
        virtual void addAffect(Character *) const;
        virtual void removeAffect(Character *) const;
};

class TravellersJoySet: public EquipSet {
XML_OBJECT
public:
        typedef ::Pointer<TravellersJoySet> Pointer;
        TravellersJoySet();

        virtual void fight( Character *ch );
protected:
        virtual bool hasAffect(Character *) const;
        virtual void addAffect(Character *) const;
        virtual void removeAffect(Character *) const;
};

class MorrisDancerSet: public EquipSet {
XML_OBJECT
public:
        typedef ::Pointer<MorrisDancerSet> Pointer;
        MorrisDancerSet();

        virtual void fight( Character *ch );
        virtual bool area( );
protected:
        virtual bool hasAffect(Character *) const;
        virtual void addAffect(Character *) const;
        virtual void removeAffect(Character *) const;
};

class NorivaMyrvaleSet: public EquipSet {
XML_OBJECT
public:
        typedef ::Pointer<NorivaMyrvaleSet> Pointer;
        NorivaMyrvaleSet();

protected:
        virtual bool hasAffect(Character *) const;
        virtual void addAffect(Character *) const;
        virtual void removeAffect(Character *) const;
};

class ReykarisShevaleSet: public EquipSet {
XML_OBJECT
public:
        typedef ::Pointer<ReykarisShevaleSet> Pointer;
        ReykarisShevaleSet();

protected:
        virtual bool hasAffect(Character *) const;
        virtual void addAffect(Character *) const;
        virtual void removeAffect(Character *) const;
};

#endif
