/* $Id$
 *
 * ruffina, 2004
 */
#include "loadsave.h"
#include "save.h"
#include "logstream.h"

#include "fenia/register-impl.h"
#include "feniamanager.h"
#include "wrapperbase.h"
#include "fenia_utils.h"

#include "skillreference.h"
#include "mobilebehaviormanager.h"
#include "objectbehaviormanager.h"
#include "objectmanager.h"
#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "npcharactermanager.h"
#include "object.h"
#include "race.h"

#include "itemevents.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

GSN(sanctuary);
GSN(haste);
GSN(protection_evil);
GSN(protection_good);
GSN(stardust);
GSN(dark_shroud);
WEARLOC(none);
WEARLOC(stuck_in);
RELIG(chronos);
PROF(mobile);


typedef void(NPCharacter::*SetterMethod)(const DLString &);
/** Override proto descriptions if there's some formatting present. */
static void override_description(NPCharacter *mob, const char *cProtoField, SetterMethod method)
{
    DLString protoField(cProtoField);

    if (protoField.find("%1") == DLString::npos)
        return;

    DLString result = fmt(0, cProtoField, mob);
    if (result != protoField)
        (mob->*method)(result);
}

/*
 * Create an instance of a mobile.
 */
NPCharacter *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    return create_mobile_org( pMobIndex, 0 );
}
NPCharacter *create_mobile_nocount( MOB_INDEX_DATA *pMobIndex )
{
    return create_mobile_org( pMobIndex, FCREATE_NOCOUNT );
}
NPCharacter *create_mobile_org( MOB_INDEX_DATA *pMobIndex, int flags )
{
        NPCharacter *mob;
        int i;
        Affect af;
        Race *race;

        if ( pMobIndex == 0 )
        {
                bug( "Create_mobile: 0 pMobIndex.", 0 );
                exit( 1 );
        }

        mob = NPCharacterManager::getNPCharacter( );

        mob->pIndexData        = pMobIndex;
        race = raceManager->find( pMobIndex->race );

        DLString name( pMobIndex->player_name );
        mob->setName( name );

        mob->spec_fun        = pMobIndex->spec_fun;

        if ( pMobIndex->wealth == 0 )
        {
                mob->silver = 0;
                mob->gold   = 0;
        }
        else
        {
                long wealth;

                wealth = number_range(pMobIndex->wealth/2, 3 * pMobIndex->wealth/2) / 8;
                mob->gold = number_range(wealth/2000,wealth/1000);
                mob->silver = wealth/10 - (mob->gold * 100);
        }

        if (pMobIndex->new_format)    /* load in new style */
        {
                /* read from prototype */
                mob->group                = pMobIndex->group;
                mob->act                 = pMobIndex->act | ACT_IS_NPC;
                // FIXME: explicitly apply race affect/detect/resist bits to the mob instance, ignoring 'del' attribute values saved in area XML format.
                // To fix it properly, all mob indexes should have a pair of overrides for every bit field, "bits to delete" and "bits to add".
                mob->affected_by        = pMobIndex->affected_by | race->getAff() ;
                mob->detection                = pMobIndex->detection | race->getDet();
                mob->alignment                = pMobIndex->alignment;
                mob->setLevel( pMobIndex->level );
//                mob->hitroll                = (mob->getRealLevel( ) / 2) + pMobIndex->hitroll;
                mob->hitroll                = mob->getRealLevel( ) + pMobIndex->hitroll;
                mob->damroll                = pMobIndex->damage[DICE_BONUS];
                mob->max_hit                = dice(pMobIndex->hit[DICE_NUMBER], pMobIndex->hit[DICE_TYPE]) + pMobIndex->hit[DICE_BONUS];
                mob->hit                = mob->max_hit;
                mob->max_mana                = dice(pMobIndex->mana[DICE_NUMBER], pMobIndex->mana[DICE_TYPE]) + pMobIndex->mana[DICE_BONUS];
                mob->mana                = mob->max_mana;
                mob->max_move           = 200 + 16 * mob->getRealLevel( ); // 2 times more than best-trained pc
                mob->move               = mob->max_move;
                mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
                mob->damage[DICE_TYPE]        = pMobIndex->damage[DICE_TYPE];
                mob->dam_type                = pMobIndex->dam_type;
        
                if (mob->dam_type == 0)
                        switch(number_range(1,3))
                        {
                        case (1): mob->dam_type = 3;        break;  /* slash */
                        case (2): mob->dam_type = 7;        break;  /* pound */
                        case (3): mob->dam_type = 11;       break;  /* pierce */
                        }
                for (i = 0; i < 4; i++)
                        mob->armor[i]        = pMobIndex->ac[i];
                mob->off_flags                = pMobIndex->off_flags;
                mob->imm_flags                = pMobIndex->imm_flags | race->getImm( );
                mob->res_flags                = pMobIndex->res_flags | race->getRes( );
                mob->vuln_flags                = pMobIndex->vuln_flags | race->getVuln( );
                mob->wearloc.set(race->getWearloc());
                mob->start_pos                = pMobIndex->start_pos;
                mob->default_pos        = pMobIndex->default_pos;
                mob->setSex( pMobIndex->sex );
                if (mob->getSex( ) == SEX_EITHER) /* random sex */
                        mob->setSex( number_range(1,2) );
                mob->setRace( race->getName() );
                mob->form                = pMobIndex->form;
                mob->parts                = pMobIndex->parts;
                mob->size                = pMobIndex->size;
                mob->material                = str_dup(pMobIndex->material);
                mob->extracted                = false;

                override_description(mob, pMobIndex->player_name, &NPCharacter::setName);
                override_description(mob, pMobIndex->short_descr, &NPCharacter::setShortDescr);
                override_description(mob, pMobIndex->long_descr, &NPCharacter::setLongDescr);
                override_description(mob, pMobIndex->description, &NPCharacter::setDescription);

                // Configure perm stats, they can be further adjusted in a global onInit.
                // Race and class modifications are applied on-the-fly inside NPCharacter::getCurrStat
                for (i = 0; i < stat_table.size; i++)
                    mob->perm_stat[i] = BASE_STAT; 


/*
                TODO: review and either move to global onInit or discard

                if (IS_SET(mob->off_flags,OFF_FAST))
                        mob->perm_stat[STAT_DEX] += 2;

                mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
                mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;
*/

                /* let's get some spell action */
                if (!IS_SET(flags, FCREATE_NOAFFECTS))
                    create_mob_affects(mob);
        }
        else /* read in old format and convert */
        {
                mob->act                = pMobIndex->act;
                mob->affected_by        = pMobIndex->affected_by;
                mob->detection                = pMobIndex->detection;
                mob->alignment                = pMobIndex->alignment;
                mob->setLevel( pMobIndex->level );
                mob->hitroll                = max(pMobIndex->hitroll,pMobIndex->level/4);
                mob->damroll                = pMobIndex->level /2 ;
                if (mob->getRealLevel( ) < 30)
                        mob->max_hit                = mob->getRealLevel( ) * 20 + number_range(
                                mob->getRealLevel( ),mob->getRealLevel( ) * 5);
                else if (mob->getRealLevel( ) < 60)
                        mob->max_hit                = mob->getRealLevel( ) * 50 + number_range(
                                mob->getRealLevel( ) * 10,mob->getRealLevel( ) * 50);
                else
                        mob->max_hit                = mob->getRealLevel( ) * 100 + number_range(
                                mob->getRealLevel( ) * 20,mob->getRealLevel( ) * 100);
                if (IS_SET(mob->act,ACT_MAGE | ACT_CLERIC))
                        mob->max_hit = ( int )( mob->max_hit * 0.9 );
                mob->hit                = mob->max_hit;
                mob->max_mana                = 100 + dice(mob->getRealLevel( ),10);
                mob->mana                = mob->max_mana;
                switch(number_range(1,3))
                {
                case (1): mob->dam_type = 3;         break;  /* slash */
                case (2): mob->dam_type = 7;        break;  /* pound */
                case (3): mob->dam_type = 11;        break;  /* pierce */
                }
                for (i = 0; i < 3; i++)
                        mob->armor[i]        = interpolate(mob->getRealLevel( ),100,-100);
                mob->armor[3]                = interpolate(mob->getRealLevel( ),100,0);
                mob->setRace( pMobIndex->race );
                mob->off_flags                = pMobIndex->off_flags;
                mob->imm_flags                = pMobIndex->imm_flags;
                mob->res_flags                = pMobIndex->res_flags;
                mob->vuln_flags                = pMobIndex->vuln_flags;
                mob->start_pos                = pMobIndex->start_pos;
                mob->default_pos        = pMobIndex->default_pos;
                mob->setSex( pMobIndex->sex );
                mob->form                = pMobIndex->form;
                mob->parts                = pMobIndex->parts;
                mob->size                = SIZE_MEDIUM;
                mob->material                = str_empty;
                mob->extracted                = false;

                for (i = 0; i < stat_table.size; i ++)
                        mob->perm_stat[i] = BASE_STAT;
        }

        mob->position = mob->start_pos;

        mob->setReligion( god_chronos );
        mob->setProfession( prof_mobile );
        
        /* link the mob to the world list */
        char_to_list( mob, &char_list );

        if (!IS_SET(flags, FCREATE_NOCOUNT))
            pMobIndex->count++;
        
        /* assign behavior */        
        if (pMobIndex->behavior) 
            MobileBehaviorManager::assign( mob );
        else
            MobileBehaviorManager::assignBasic( mob );
        
        /* Fenia initialization: call global onInit followed by specific 'init' defined for this mob index data. */
        if (!IS_SET(flags, FCREATE_NOCOUNT)) {
            
            gprog("onInit", "C", mob);

            WrapperBase *w = get_wrapper(pMobIndex->wrapper);
            if (w) {
                static Scripting::IdRef initId( "init" );
                try {
                    w->call(initId, "C", mob);
                } catch (const Exception &e) {
                    LogStream::sendError( ) 
                            << "create_mobile #" << pMobIndex->vnum 
                            << ": " <<  e.what( ) << endl;
                }
            }
        }
        
        return mob;
}

/** Transform affect bits from the prototype into real affects. Not called for mobs read from disk. */
void create_mob_affects(NPCharacter *mob)
{
    if (IS_AFFECTED(mob, AFF_SANCTUARY))
    {
        Affect af;
        if (IS_EVIL(mob)) {
			affect_strip( mob, gsn_sanctuary );
			REMOVE_BIT(mob->affected_by, AFF_SANCTUARY);
			af.type      = gsn_dark_shroud;
		}
		else {
        	af.type      = gsn_sanctuary; 
        	af.bitvector.setValue(AFF_SANCTUARY);
        	af.bitvector.setTable(&affect_flags);
		}
        af.level     = mob->getRealLevel( );
        af.duration  = -1;
        affect_to_char( mob, &af );
    }

    if (IS_AFFECTED(mob,AFF_HASTE))
    {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type      = gsn_haste; 
        af.level     = mob->getRealLevel( );
        af.duration  = -1;
        af.location = APPLY_DEX;
        af.modifier = 1 + (mob->getRealLevel( ) >= 18) + (mob->getRealLevel( ) >= 25) +
                (mob->getRealLevel( ) >= 32);
        af.bitvector.setValue(AFF_HASTE);
        affect_to_char( mob, &af );
    }

    if (IS_AFFECTED(mob,AFF_PROTECT_EVIL))
    {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type         = gsn_protection_evil;
        af.level         = mob->getRealLevel( );
        af.duration         = -1;
        af.location = APPLY_SAVES; 
        af.modifier = -1;
        af.bitvector.setValue(AFF_PROTECT_EVIL);
        affect_to_char(mob,&af);
    }

    if (IS_AFFECTED(mob,AFF_PROTECT_GOOD))
    {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type      = gsn_protection_good; 
        af.level     = mob->getRealLevel( );
        af.duration  = -1;
        af.location = APPLY_SAVES; 
        af.modifier = -1;
        af.bitvector.setValue(AFF_PROTECT_GOOD);
        affect_to_char(mob,&af);
    }
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(NPCharacter *parent, NPCharacter *clone)
{
    int i;

    if (parent == 0 || clone == 0)
        return;

    /* start fixing values */
    DLString name( parent->getNameC() );
    clone->setName( name );

    if (parent->getRealShortDescr( ))
        clone->setShortDescr( parent->getShortDescr( ) );
    if (parent->getRealLongDescr( ))
        clone->setLongDescr( parent->getLongDescr( ) );
    if (parent->getRealDescription( ))
        clone->setDescription( parent->getDescription( ) );

    clone->group        = parent->group;
    clone->setSex( parent->getSex( ) );
    clone->setRace( parent->getRace( )->getName( ) );
    clone->setLevel( parent->getRealLevel( ) );
    clone->timer        = parent->timer;
    clone->wait                = parent->wait;
    clone->hit                = parent->hit;
    clone->max_hit        = parent->max_hit;
    clone->mana                = parent->mana;
    clone->max_mana        = parent->max_mana;
    clone->move                = parent->move;
    clone->max_move        = parent->max_move;
    clone->gold                = parent->gold;
    clone->silver        = parent->silver;
    clone->exp                = parent->exp;
    clone->act                = parent->act;
    clone->comm                = parent->comm;
    clone->imm_flags        = parent->imm_flags;
    clone->res_flags        = parent->res_flags;
    clone->vuln_flags        = parent->vuln_flags;
    clone->invis_level        = parent->invis_level;
    clone->affected_by        = parent->affected_by;
    clone->detection        = parent->detection;
    clone->position        = parent->position;
    clone->saving_throw        = parent->saving_throw;
    clone->alignment        = parent->alignment;
    clone->hitroll        = parent->hitroll;
    clone->damroll        = parent->damroll;
    clone->wimpy        = parent->wimpy;
    clone->form                = parent->form;
    clone->parts        = parent->parts;
    clone->size                = parent->size;
    clone->material        = str_dup(parent->material);
    clone->extracted        = parent->extracted;
    clone->off_flags        = parent->off_flags;
    clone->dam_type        = parent->dam_type;
    clone->start_pos        = parent->start_pos;
    clone->default_pos        = parent->default_pos;
    clone->spec_fun        = parent->spec_fun;

    for (i = 0; i < 4; i++)
            clone->armor[i]        = parent->armor[i];

    for (i = 0; i < stat_table.size; i++)
    {
        clone->perm_stat[i]        = parent->perm_stat[i];
        clone->mod_stat[i]        = parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
        clone->damage[i]        = parent->damage[i];

    /* now add the affects */
    for (auto &paf: parent->affected)
        affect_to_char(clone,paf);

}

/*
 * Create an object with modifying the count
 */
Object *create_object( OBJ_INDEX_DATA *pObjIndex, short level )
{
  return create_object_org(pObjIndex,level,true);
}

/*
 * for player load/quit
 * Create an object and do not modify the count
 */
Object *create_object_nocount(OBJ_INDEX_DATA *pObjIndex, short level )
{
  return create_object_org(pObjIndex,level,false);
}

/*
 * Create an instance of an object.
 */
Object *create_object_org( OBJ_INDEX_DATA *pObjIndex, short level, bool Count )
{
        Object *obj;

        if ( pObjIndex == 0 )
        {
                bug( "Create_object: 0 pObjIndex.", 0 );
                exit( 1 );
        }

        obj = ObjectManager::getObject( );

        obj->pIndexData        = pObjIndex;
        obj->in_room        = 0;
        obj->enchanted        = false;
        obj->updateCachedNoun( );

        pObjIndex->instances.push_back(obj);

        if ( ( obj->pIndexData->limit != -1 )
                && ( obj->pIndexData->count >  obj->pIndexData->limit ) )
                if ( pObjIndex->new_format == 1 )
                    LogStream::sendWarning( ) << "Obj limit exceeded for vnum " << pObjIndex->vnum << endl;

        /*    if ( pObjIndex->new_format == 1 ) */
        if (pObjIndex->new_format)
                obj->level = pObjIndex->level;
        else
                obj->level = max(static_cast<short>( 0 ),level);

        obj->item_type        = pObjIndex->item_type;
        obj->extra_flags= pObjIndex->extra_flags;
        obj->wear_flags        = pObjIndex->wear_flags;
        obj->value0(pObjIndex->value[0]);
        obj->value1(pObjIndex->value[1]);
        obj->value2(pObjIndex->value[2]);
        obj->value3(pObjIndex->value[3]);
        obj->value4(pObjIndex->value[4]);
        obj->weight        = pObjIndex->weight;
        obj->extracted        = false;
        obj->from       = str_dup(""); /* used with body parts */
        obj->condition        = pObjIndex->condition;

        if (level == 0 || pObjIndex->new_format)
                if (obj->cost > 1000)
                    obj->cost        = pObjIndex->cost / 10;
                else
                    obj->cost        = pObjIndex->cost;
                
        else
                obj->cost        = number_fuzzy( level );

        /*
         * Mess with object properties.
         */
        switch ( obj->item_type ) {
        case ITEM_LIGHT:
                if ( obj->value2() == 999 )
                        obj->value2(-1);
                break;

        case ITEM_CORPSE_PC:
                obj->value3(ROOM_VNUM_ALTAR);
                break;
        }
        
        obj_to_list( obj );

        if ( Count )
                pObjIndex->count++;

        /* assign behavior */
        if (pObjIndex->behavior) 
            ObjectBehaviorManager::assign( obj );
        else
            ObjectBehaviorManager::assignBasic( obj );
        
        /* fenia objprog initialization */
        if (Count) {
            WrapperBase *w = get_wrapper(pObjIndex->wrapper);
            if (w) {
                static Scripting::IdRef initId( "init" );
                try {
                    w->call(initId, "O", obj);
                } catch (const Exception &e) {
                    LogStream::sendError( ) 
                            << "create_object #" << pObjIndex->vnum 
                            << ": " <<  e.what( ) << endl;
                }
            }
        }

        // Notify item creation listeners.
        eventBus->publish(ItemCreatedEvent(obj, Count));
        return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(Object *parent, Object *clone)
{
    int i;
    EXTRA_DESCR_DATA *ed,*ed_new;

    if (parent == 0 || clone == 0)
        return;

    /* start fixing the object */
    if (parent->getRealName( ))
        clone->setName( parent->getName( ) );
    if (parent->getRealShortDescr( ))
        clone->setShortDescr( parent->getShortDescr( ) );
    if (parent->getRealDescription( ))
        clone->setDescription( parent->getDescription( ) );
    if (parent->getRealMaterial( ))
        clone->setMaterial( parent->getMaterial( ) );
    if (parent->getOwner( ))
        clone->setOwner( parent->getOwner( ) );

    clone->item_type        = parent->item_type;
    clone->extra_flags        = parent->extra_flags;
    clone->wear_flags        = parent->wear_flags;
    clone->weight        = parent->weight;
    clone->cost                = parent->cost;
    clone->level        = parent->level;
    clone->condition        = parent->condition;

    clone->pocket = parent->pocket;
    clone->timer        = parent->timer;
    clone->from         = str_dup(parent->from);
    clone->extracted    = parent->extracted;

    for (i = 0;  i < 5; i ++)
        clone->valueByIndex(i, parent->valueByIndex(i));

    /* affects */
    clone->enchanted        = parent->enchanted;

    for (auto &paf: parent->affected)
        affect_to_obj( clone, paf);

    /* extended desc */
    for (ed = parent->extra_descr; ed != 0; ed = ed->next)
    {
        ed_new                  = new_extra_descr();
        ed_new->keyword            = str_dup( ed->keyword);
        ed_new->description     = str_dup( ed->description );
        ed_new->next                   = clone->extra_descr;
        clone->extra_descr          = ed_new;
    }

    for (auto &p: parent->properties)
        clone->properties[p.first] = p.second;
}



