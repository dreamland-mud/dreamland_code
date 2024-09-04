/* $Id$
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "xmlmobilefactory.h"
#include "grammar_entities_impl.h"
#include "skillgroup.h"
#include "religion.h"
#include "behavior.h"
#include "race.h"
#include "merc.h"

#include "def.h"

XMLMobileFactory::XMLMobileFactory( ) : 
                          act(&act_flags),
                          aff(&affect_flags),
                          dam_type(DAMW_NONE, &weapon_flags),
                          off(&off_flags),
                          imm(&imm_flags),
                          res(&res_flags),
                          vuln(&vuln_flags),
                          start_pos(POS_STANDING, &position_table),
                          default_pos(POS_STANDING, &position_table),
                          sex(SEX_NEUTRAL, &sex_table),
                          form(&form_flags),
                          parts(&part_flags),
                          size(NO_FLAG, &size_table),
                          detection(&detect_flags),
                          practicer(skillGroupManager),
                          religion(religionManager),
                          affects(skillManager),
                          behaviors(behaviorManager),
                          properties( false )
{
}

void
XMLMobileFactory::init(const mob_index_data *mob)
{
    Race *mobrace = raceManager->find(mob->race);

    player_name.setValue(mob->player_name);
    short_descr.setValue(mob->short_descr);
    long_descr.setValue(mob->long_descr);
    description.setValue(mob->description);
    race.setValue(mob->race);
    act.setRealBase(mob->act, mobrace->getAct( ));
    aff.setRealBase(mob->affected_by, mobrace->getAff( ));
    alignment.setValue(mob->alignment);
    group.setValue(mob->group);
    level.setValue(mob->level);
    hitroll.setValue(mob->hitroll);
    hit.set(mob->hit[DICE_NUMBER], mob->hit[DICE_TYPE], mob->hit[DICE_BONUS]);
    mana.set(mob->mana[DICE_NUMBER], mob->mana[DICE_TYPE], mob->mana[DICE_BONUS]);
    damage.set(mob->damage[DICE_NUMBER], mob->damage[DICE_TYPE], mob->damage[DICE_BONUS]);
    dam_type.setValue(mob->dam_type);
    
    ac.pierce = mob->ac[AC_PIERCE] / 10;
    ac.bash = mob->ac[AC_BASH] / 10;
    ac.slash = mob->ac[AC_SLASH] / 10;
    ac.exotic = mob->ac[AC_EXOTIC] / 10;

    off.setRealBase(mob->off_flags, mobrace->getOff( ));
    imm.setRealBase(mob->imm_flags, mobrace->getImm( ));
    res.setRealBase(mob->res_flags, mobrace->getRes( ));
    vuln.setRealBase(mob->vuln_flags, mobrace->getVuln( ));

    start_pos.setValue(mob->start_pos);
    default_pos.setValue(mob->default_pos);

    sex.setValue(mob->sex);
    wealth.setValue(mob->wealth);
    form.setRealBase(mob->form, mobrace->getForm( ));
    parts.setRealBase(mob->parts, mobrace->getParts( ));

    size.setValue(mob->size);
    material.setValue(mob->material);

    detection.setRealBase(mob->detection, mobrace->getDet( ));

    const char *c = 0;
    
    if(mob->spec_fun.func)
        c = spec_name(mob->spec_fun.func);

    if(!c)
        c = mob->spec_fun.name.c_str();

    if(!c)
        c = "";

    spec.setValue(c);
    practicer.set(mob->practicer);
    religion.set(mob->religion);
    affects.set(mob->affects);
    behaviors.set(mob->behaviors);
    
    if (mob->gram_number != Grammar::Number::SINGULAR)
        gram_number.setValue(mob->gram_number.toString());

    clan.assign(mob->clan);

    smell.setValue(mob->smell);

    if(!mob->behavior.isEmpty( ))
        behavior.setNode(mob->behavior->getFirstNode( ));

    for (Properties::const_iterator p = mob->properties.begin( ); p != mob->properties.end( ); p++)
        properties.insert( *p );
}

mob_index_data *
XMLMobileFactory::compat( )
{
    static MOB_INDEX_DATA zeroMobIndex;
    MOB_INDEX_DATA *mob = new MOB_INDEX_DATA;
    *mob = zeroMobIndex;

    compat(mob);

    return mob;
}

void
XMLMobileFactory::compat(mob_index_data *mob)
{

    Race *mobrace = raceManager->find(race.getValue( ).c_str( ));

    mob->player_name = str_dup(player_name.getValue( ).c_str( ));
    mob->short_descr = str_dup(short_descr.getValue( ).c_str( ));
    mob->long_descr = str_dup(long_descr.getValue( ).c_str( ));
    mob->description = str_dup(description.getValue( ).c_str( ));
    mob->race = str_dup(race.getValue( ).c_str( ));
    mob->act = act.get(mobrace->getAct( )) | ACT_IS_NPC;
    mob->affected_by = aff.get(mobrace->getAff( ));
    mob->alignment = alignment.getValue( );
    mob->group = group.getValue( );
    mob->level = level.getValue( );
    mob->hitroll = hitroll.getValue( );
    mob->hit[DICE_NUMBER] = hit.number;
    mob->hit[DICE_TYPE] = hit.type;
    mob->hit[DICE_BONUS] = hit.bonus;
    mob->mana[DICE_NUMBER] = mana.number;
    mob->mana[DICE_TYPE] = mana.type;
    mob->mana[DICE_BONUS] = mana.bonus;
    mob->damage[DICE_NUMBER] = damage.number;
    mob->damage[DICE_TYPE] = damage.type;
    mob->damage[DICE_BONUS] = damage.bonus;
    mob->dam_type = dam_type.getValue( );
    
    mob->ac[AC_PIERCE] = ac.pierce * 10;
    mob->ac[AC_BASH] = ac.bash * 10;
    mob->ac[AC_SLASH] = ac.slash * 10;
    mob->ac[AC_EXOTIC] = ac.exotic * 10;

    mob->off_flags = off.get(mobrace->getOff( ));
    mob->imm_flags = imm.get(mobrace->getImm( ));
    mob->res_flags = res.get(mobrace->getRes( ));
    mob->vuln_flags = vuln.get(mobrace->getVuln( ));

    mob->start_pos = start_pos.getValue( );
    mob->default_pos = default_pos.getValue( );

    mob->sex = sex.getValue( );
    mob->wealth = wealth.getValue( );
    mob->form = form.get(mobrace->getForm( ));
    mob->parts = parts.get(mobrace->getParts( ));

    mob->size = size.getValue( );
    mob->material = str_dup(material.getValue( ).c_str( ));

    mob->detection = detection.get(mobrace->getDet( ));

    if(!spec.getValue( ).empty( )) {
        mob->spec_fun.name = spec.getValue( );
        mob->spec_fun.func = spec_lookup(mob->spec_fun.name.c_str( ));

        if(!mob->spec_fun.func) {
            LogStream::sendError() 
                << "special " << mob->spec_fun.name
                << " not found." << endl;
        }
    }

    mob->practicer.set(practicer);
    mob->religion.set(religion);
    mob->affects.set(affects);
    mob->behaviors.set(behaviors);
    
    if(!gram_number.getValue( ).empty( ))
        mob->gram_number = Grammar::Number(gram_number.getValue( ).c_str( ));
 
    if(!smell.getValue( ).empty( ))
        mob->smell = smell.getValue( );

    mob->clan = clan;
    
    if(behavior.getNode( )) {
        mob->behavior.construct( );
        XMLNode::Pointer p = behavior.getNode( );
        mob->behavior->appendChild(p);
    }

    for (XMLMapBase<XMLString>::iterator p = properties.begin( ); p != properties.end( ); p++)
        mob->properties.insert( *p );
}
