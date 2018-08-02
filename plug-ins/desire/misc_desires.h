/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MISC_DESIRES_H
#define MISC_DESIRES_H

#include "defaultdesire.h"

class BloodlustDesire : public DefaultDesire {
XML_OBJECT
public:
    typedef ::Pointer<BloodlustDesire> Pointer;

    virtual bool applicable( PCharacter * );
protected:
    virtual int getUpdateAmount( PCharacter * );
    virtual void damage( PCharacter * );
};


class ThirstDesire : public DefaultDesire {
XML_OBJECT
public:
    typedef ::Pointer<ThirstDesire> Pointer;

    virtual bool applicable( PCharacter * );
protected:
    virtual int getUpdateAmount( PCharacter * );
    virtual void damage( PCharacter * );
};

class HungerDesire : public DefaultDesire {
XML_OBJECT
public:
    typedef ::Pointer<HungerDesire> Pointer;

    virtual bool applicable( PCharacter * );
protected:
    virtual int getUpdateAmount( PCharacter * );
    virtual void damage( PCharacter * );
};

class FullDesire : public DefaultDesire {
XML_OBJECT
public:
    typedef ::Pointer<FullDesire> Pointer;

    virtual bool applicable( PCharacter * );
    virtual bool canEat( PCharacter * );
    virtual bool canDrink( PCharacter * );
protected:
    virtual bool isOverflow( PCharacter * );
    virtual int getUpdateAmount( PCharacter * );

    XML_VARIABLE XMLInteger overflowLimit;     
};

class DrunkDesire : public DefaultDesire {
XML_OBJECT
public:
    typedef ::Pointer<DrunkDesire> Pointer;

    virtual bool isActive( PCharacter * );
    virtual bool applicable( PCharacter * );
    virtual bool canDrink( PCharacter * );
protected:
    virtual int getUpdateAmount( PCharacter * );
};

#endif
