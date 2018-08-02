/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __ELVISH_EFFECTS_H__
#define __ELVISH_EFFECTS_H__

#include "wordeffect.h"
#include "plugin.h"

class ResistIronWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<ResistIronWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

class RestoringWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<RestoringWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

class BlessEquipWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<BlessEquipWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

class ElvishEffectsPlugin : public Plugin {
public:
    typedef ::Pointer<ElvishEffectsPlugin> Pointer;

    virtual void initialization( );
    virtual void destruction( );
};

#endif

