#ifndef EQUIPSET_H
#define EQUIPSET_H

#include "objectbehavior.h"

class EquipSet : public ObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<EquipSet> Pointer;
        
        EquipSet(int);    
        virtual void equip( Character *victim );                           
        virtual void remove( Character *victim );

protected:
        bool isComplete(Character *) const;
        virtual bool hasAffect(Character *) const = 0;
        virtual void addAffect(Character *) const = 0;
        virtual void removeAffect(Character *) const = 0;

        int totalSetSize;
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
        
        int sn;
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
        
        int sn;
};
#endif
