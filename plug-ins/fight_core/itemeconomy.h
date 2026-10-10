#ifndef ITEMECONOMY_H
#define ITEMECONOMY_H

/*
 * Item economy: one weight model and one price model for every object
 * (docs/plans/weight-cost-sweep.md sections 2b, 3.4).
 *
 * Weight: reference kg for the armor slot, weapon class or item type, x the
 * material's density inside its own family, x heft, x two hands; solid lumps
 * (gems) keep a block. Config fight/item_weight.json. Units: 1/10 lb.
 *
 * Price: min(cost cap, base + magic). base = by type (level, spells, charges,
 * hours, capacity...) + material price per kg x weight; magic = cost_k x M x
 * level, M = the sage's base score of the item / one M. Config
 * fight/item_value.json "cost" and "measure", material.json "price". Silver.
 *
 * Prototypes: the area file's <weight> > 0 overrides the model, 0 = the model.
 * <cost> is the price and the model only lowers it: min(model, <cost>); no
 * <cost> stays 0, so no price rises. Computed at boot (SCDP_BOOT + 7), on
 * oedit commit, never written back to the area file.
 */
#include "dlstring.h"

class Object;
struct obj_index_data;

/** The model weight of an instance (its own material, class, hands, heft and
 *  coverage props). heft light|medium|heavy overrides the item's own. */
int item_weight( Object *obj, const DLString &heft = DLString::emptyString );

/** The model weight of a prototype. */
int item_weight( obj_index_data *pObj, const DLString &heft = DLString::emptyString );

/** What a prototype weighs: the area-file override, else the model. */
int item_proto_weight( obj_index_data *pObj );

/** What an instance should weigh: its prototype's weight while it is shaped
 *  like the prototype (same type, material, weapon class and hands, no own heft
 *  or coverage), else the model; coins by the coin formula. */
int item_instance_weight( Object *obj );

/** Price cap for any item, item_value.json measure.cost_cap. */
int item_cost_cap( );

/** The base (non-magic) part of a prototype's auto price. */
int item_base_cost( obj_index_data *pObj );

/** A prototype's measure in centi-M (best of melee and caster), 0 without stats. */
int item_measure_cm( obj_index_data *pObj );

/** A prototype's auto price: min(cap, base + magic); 0 for a non-take item. */
int item_auto_cost( obj_index_data *pObj );

/** A prototype with Fenia triggers, a C++ behavior or any behavior: the price
 *  model can not see what it does. */
bool item_scripted( obj_index_data *pObj );

/** What a prototype costs: min(auto, area-file cost), 0 without one; a scripted
 *  prototype with no stats (M 0) keeps its area-file cost. Always within the cap. */
int item_proto_cost( obj_index_data *pObj );

/** Recompute pObj->weight and pObj->cost from the area-file values and the models. */
void item_economy_proto( obj_index_data *pObj );

/** The sage's base score of a prototype lives in feniaroot, which registers it
 *  here on load and clears it on unload. Unset -> magic 0. */
typedef double (*ItemProtoPointsFn)( obj_index_data *pObj, bool caster );
void item_set_proto_points_fn( ItemProtoPointsFn fn );

#endif
