/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PERSONALNAMEREPAIR_H__
#define __PERSONALNAMEREPAIR_H__

#include "eventbus.h"
#include "plugin.h"

class ItemReadEvent;

/**
 * Backfills the English and Ukrainian names of personalised objects created
 * before the engine learned to write every language slot -- the hero quest
 * items, which carry their owner in the 'owner' field and a "%s" template
 * in every language of their prototype.
 *
 * Rides ItemReadEvent, which reaches everything saved: player inventories and
 * room contents alike are re-read from disk, so no separate admin pass is
 * needed. Idempotent -- an object with no empty slot behind a template costs
 * one string check.
 */
class PersonalNameRepair : public Plugin, public EventHandler {
public:
    typedef ::Pointer<PersonalNameRepair> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual void handleEvent( const type_index &eventType, const Event &event ) const;

protected:
    void eventItemRead( const ItemReadEvent &event ) const;
};

#endif
