#include <map>
#include "defaultreligion.h"
#include "religionattribute.h"
#include "religionflags.h"
#include "logstream.h"
#include "calendar_utils.h"

#include "behavior_utils.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "commandtemplate.h"
#include "pcharacter.h"
#include "race.h"
#include "core/object.h"
#include "skill.h"
#include "skillmanager.h"
#include "liquid.h"

#include "act.h"
#include "alignment.h"
#include "dreamland.h"
#include "fight.h"
#include "gsn_plugin.h"
#include "../anatolia/handler.h"
#include "interp.h"
#include "itemflags.h"
#include "loadsave.h"
#include "wearloc_utils.h"
#include "mercdb.h"
#include "merc.h"
#include "save.h"
#include "terrains.h"
#include "wiznet.h"
#include "def.h"
#include "skill_utils.h"

#define OBJ_VNUM_ALTAR 88

RELIG(none);
RELIG(fili);
GSN(sacrifice);
BONUS(experience);
BONUS(learning);
BONUS(mana);

const int default_threshold = 1000;

int sacrifice_obj( Character *ch, Object *obj, bool needSpam );
int rescue_nosac_items( Object *container, Room *room );

struct Offering {
    Offering(const DefaultReligion *religion, Object *altar) {
        this->religion = religion;
        smite = false;

        for (Object *obj = altar->contains; obj; obj = obj->next_content) 
            estimateItem(obj);
        
        sum = 0;
        for (ItemStats::const_iterator i = cost.begin(); i != cost.end(); i++) {
            LogStream::sendNotice() << "Sacrifice " << i->first << " cost=" << i->second << ", quantity=" << quantity[i->first] << endl;
            sum += i->second;
        }
    }

    int getSum() const {
        return sum;
    }
    
    bool smites() const {
        return smite;
    }

    const DLString & getSmiteMessage() const {
        return smiteMessage;
    }
protected:
    void estimateItem(Object *obj)
    {
        if (is_name("book", obj->getName())) {
            bookNoItemType(obj);
            return;
        }

        if (religion->ignoresItem(obj))
            return;

        switch (obj->item_type) {
        case ITEM_MONEY:    
            money(obj); 
            break;

        case ITEM_LIGHT:
            light(obj);
            break;

        case ITEM_WARP_STONE:
        case ITEM_GEM:      
            gem(obj); 
            break;

        case ITEM_JEWELRY:
        case ITEM_TREASURE: 
            treasure(obj); 
            break;

        case ITEM_ARMOR:    
        case ITEM_CLOTHING: 
            armor(obj);    
            break;    

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            potion(obj); 
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            wand(obj);
            break;
        
        case ITEM_FOOD:       food(obj); break;
        case ITEM_DRINK_CON:  drink(obj); break;
        case ITEM_WEAPON:     weapon(obj); break;
        case ITEM_CORPSE_PC:  corpse_pc(obj); break;
        case ITEM_CORPSE_NPC: corpse_npc(obj); break;

        case ITEM_PARCHMENT:
        case ITEM_SPELLBOOK:
        case ITEM_TEXTBOOK:
            book(obj);
            break;
        } 
    
        if (religion->likesStolen(obj))         
            cost["stolen"] += 100;
    }
    
    void light(Object *obj) {
        if (religion->likesItem(obj)) {
            quantity["light"]++;
            cost["light"] = min(500, cost["light"] + 300);
        }
    }

    void book(Object *obj) {
        if (religion->likesItem(obj)) {
            quantity["book"]++;
            cost["book"] = min(500, cost["book"] + 300);
        }
    }

    void bookNoItemType(Object *obj) {
        if (religion->likesBook(obj)) {
            quantity["book"]++;
            cost["book"] = min(500, cost["book"] + 300);
        }
    }

    void money(Object *obj) {
        quantity["money"] = 1;
        int item_cost =  obj->value0() / 100 + obj->value1();
        cost["money"] = min(200, item_cost);
    }
    
    void gem(Object *obj) {
        quantity["gem"]++;
        int item_cost = max(50, obj->cost / 100);
        int total_cost = min(200, cost["gem"] + item_cost);
        cost["gem"] = total_cost;
    }
    
    void treasure(Object *obj) {
        quantity["treasure"]++;
        int item_cost = max(50, obj->cost / 100);
        int total_cost = min(200, cost["treasure"] + item_cost);
        cost["treasure"] = total_cost;
    }

    void armor(Object *obj) {
        quantity["armor"]++;
        int item_cost = max(50, obj->cost / 100);
        int total_cost = min(300, cost["armor"] + item_cost);
        cost["armor"] = total_cost;
    }

    void potion(Object *obj) {
        Skill *skill;

        // Favour spells present in tattoos or closely related.
        for (int i = 1; i <= 4; i++)
            if ((skill = SkillManager::getThis( )->find( obj->valueByIndex(i) ) )) {
                if (religion->likesSpell(skill)) {
                    quantity["magic"]++;
                    cost["magic"] = min(500, cost["magic"] + 200);
                    return;
                }
            }
    }

    void wand(Object *obj) {
        Skill *skill;

        // Favour spells present in tattoos or closely related.
        if ((skill = SkillManager::getThis( )->find( obj->value3() ) ))
            if (religion->likesSpell(skill)) {
                quantity["magic"]++;
                cost["magic"] = min(500, cost["magic"] + 200);
                return;
            }
    }

    void weapon(Object *obj) {
        // Some gods like weapons, some are indifferent.
        if (religion->getAlign() == N_ALIGN_GOOD && IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)) 
            return;
        if (religion->getAlign() == N_ALIGN_EVIL && IS_WEAPON_STAT(obj, WEAPON_HOLY))
            return;
        if (religion->likesItem(obj)) {
            quantity["weapon"]++;
            // Penalize for worn out weapons.
            int item_cost = max(100, 500 - (100 - obj->condition) * 10);
            cost["weapon"] = min(500, cost["weapon"] + item_cost);
        }
    }

    void food(Object *obj) {
        // Smite for offering poisonous food.
        if (obj->value3()) {
            smite = true;
            smiteMessage = "ядовитую еду";
            return;
        }
        // Use double food's "full" value as its cost.
        quantity["food"]++;
        cost["food"] = min(200, cost["food"] + obj->value0() * 2);
    }

    void drink(Object *obj) {
        // Smite for poisonous or empty drinks.
        if (IS_SET( obj->value3(), DRINK_POISONED )) {
            smite = true;
            smiteMessage = "отравленную жидкость";
            return;
        }
        if (obj->value1() <= 0 || obj->value0() <= 0) {
            smite = true;
            smiteMessage = "пустую тару";
            return;
        }
        if (obj->value1() < obj->value0() / 4) {
            smite = true;
            smiteMessage = "полупустую тару";
            return;
        }
        // Favourite drinks (wine, milk) give more points.
        quantity["drink"]++;
        int item_cost;
        if (religion->likesDrink(liquidManager->find(obj->value2()))) {
            // Penalize for non-full containers.
            item_cost = 500 * obj->value1() / obj->value0();
        } else {
            item_cost = min(100, obj->value1());
        }
        cost["drink"] = min(500, cost["drink"] + item_cost);
    }

    void corpse_pc(Object *obj) {
        smite = true;
        smiteMessage = fmt(0, "%O4", obj);    
    }

    void corpse_npc(Object *obj) {
        bitnumber_t align = ALIGN_NUMBER(obj->value4());
        MOB_INDEX_DATA *pMob = get_mob_index(obj->value3());
        if (!pMob)
            return;

        // Very pleased if corpse's align is not allowed for religion.
        if (!religion->getAlign().isSetBitNumber(align)) {
            // Penalty for smaller corpses.
            int sizePenalty = max(0, SIZE_MEDIUM - pMob->getSize());    
            int item_cost = 500 - sizePenalty * 50; 
            cost["goodcorpse"] = min(800, cost["goodcorpse"] + item_cost);
            quantity["goodcorpse"]++;
            return;
        }
        
        // Look closely if it's an "ally" corpse or if we ignore align completely.
        switch (align) {
        case N_ALIGN_GOOD:
            if (religion->getAlign().isSetBitNumber(N_ALIGN_GOOD)
                && !religion->getAlign().isSetBitNumber(N_ALIGN_EVIL))
            {
                smite = true;
                smiteMessage = "труп доброго существа";
                return;
            } 
            break;

        case N_ALIGN_EVIL:
            if (religion->getAlign().isSetBitNumber(N_ALIGN_EVIL)
                && !religion->getAlign().isSetBitNumber(N_ALIGN_GOOD))
            {
                smite = true;
                smiteMessage = "труп злого существа";
                return;
            } 
            break;

        case N_ALIGN_NEUTRAL:
            if (religion->getAlign().isSetBitNumber(N_ALIGN_NEUTRAL)
                && (!religion->getAlign().isSetBitNumber(N_ALIGN_GOOD)
                    || !religion->getAlign().isSetBitNumber(N_ALIGN_EVIL)))
            {
                smite = true;
                smiteMessage = "труп нейтрального существа";
                return;
            } 
            break;
        }

        quantity["corpse"]++;
        cost["corpse"] = min(500, cost["corpse"] + 200);       
    }

    typedef map<DLString, int> ItemStats;
    ItemStats quantity;
    ItemStats cost;
    const DefaultReligion *religion;
    int sum;
    bool smite;
    DLString smiteMessage;
};




/*
 * 'sacrifice' command
 */
static void altar_clear(Object *altar)
{
    Object *obj, *obj_next;
    for (obj = altar->contains; obj; obj = obj_next) {
        obj_next = obj->next_content;
        rescue_nosac_items(obj, altar->in_room);
        extract_obj(obj); 
    }
}

void sacrifice_at_altar(Character *ch, Object *altar, const char *arg)
{
    if (ch->is_npc()) {
        ch->pecho("Изыди, глупое животное.");
        return;
    }
    
    PCharacter *pch = ch->getPC();
    const Religion &religion = *(pch->getReligion());
    const char *rname = religion.getRussianName().c_str();

    if (religion.getIndex() == god_none) {
        ch->pecho("Но ты же закоренел%1$Gое|ый|ая атеист%1$G||ка.", ch);
        ch->recho("%^C1 совершает бессмысленные манипуляции с %O5.", ch, altar);
        return;
    }
    
    if (religion.getName() != altar->getOwner()) {
        ch->pecho("%^O1 посвящен другому божеству.", altar);
        return;
    }

    if (ch->isAffected(gsn_sacrifice)) {
        ch->pecho("Ты все еще под впечатлением от предыдущего жертвоприношения.");
        return;
    }

    if (!altar->contains) {
        ch->pecho("Но на %O6 совсем пусто!", altar);
        return;
    }

    XMLAttributeReligion::Pointer attr = pch->getAttributes().getAttr<XMLAttributeReligion>("religion");
    if (day_of_epoch(time_info) <= attr->prevBonusEnds) {
        ch->pecho("Ты все еще пользуешься плодами предыдущего жертвоприношения.");
        return;
    }

    Bonus *bonus = bonusManager->findUnstrict(arg);
    if (!bonus) {
        ch->pecho("Ты можешь попросить богов об одной из таких вещей: {lRопыт, мана, обучаемость, воровские умения{lEexp, mana, learning, thief skills{x.");
        ch->pecho("Но помни, что не все они для тебя подходят.");
        return;
    }

    if (!bonus->available(pch)) {
        ch->pecho("Ты слышишь насмешливый голос с неба, говорящий тебе, что то, о чем ты просишь, тебе ни к чему.");
        return;
    }

    if(ch->getRace()->getName() == "human" && bonus->getName() == "learning") {
        ch->pecho("Ты и так учишься быстрее других. Попроси божество об иной милости.");
        return;
    }


    if (attr->prevBonus == bonus->getIndex()) {
        ch->pecho("Не стоит просить %N4 об одном и том же два раза подряд -- попроси %p2 о чем-то еще.", rname, religion.getSex());
        return;
    }

    ch->pecho("Ты приносишь содержимое %O2 в жертву %N3 и надеешься на %p2 милость.",
              altar, rname, religion.getSex());
    ch->recho("%^C1 приносит содержимое %O2 в жертву своим богам.", ch, altar);

    DefaultReligion *drelig = dynamic_cast<DefaultReligion *>(pch->getReligion().getElement());
    if (!drelig || !drelig->flags.isSet(RELIG_CULT)) {
        ch->pecho("Похоже, %N1 совершенно равнодуш%gно|ен|на к жертвоприношениям.", 
                   rname, religion.getSex());
        ch->recho("...но ничего не происходит.");
        return;
    }

    attr->attempts++;

    Offering offering(drelig, altar);
    LogStream::sendNotice() << ch->getName() << " sacrifices altar to " << drelig->getName() << ", total " << offering.getSum() << endl;
    if (offering.smites()) {
        ch->pecho("{R%^N1 гневается на тебя за попытку принести в жертву %s!{x",
                       rname, offering.getSmiteMessage().c_str());
        ch->recho("Гнев %^N2 обрушивается на %C4!", rname, ch);
        rawdamage(ch, ch, DAM_OTHER, ch->hit / 4, false);
        ch->pecho("Это было действительно {rБОЛЬНО{x!"); 
        altar_clear(altar);

        postaffect_to_char(ch, gsn_sacrifice, 5);
        attr->angers++;
        ch->setWaitViolence(2);
        pch->save();
        return;
    }
    
    if (offering.getSum() <  default_threshold) {
        ch->pecho("%^N1 игнорирует твое скудное подношение.", rname);
        ch->recho("...но ничего не происходит.");
        ch->setWaitViolence(1);
        pch->save();
        return;
    }

    ch->pecho("{Y%^N1 благосклонно принимает твою жертву.{x", rname);
    ch->pecho("{YВсю следующую неделю тебе будет сопутствовать удача в %s.{x", bonus->getShortDescr().ruscase('6').c_str());
    ch->recho("%1$^C1 выглядит просветленн%1$Gым|ым|ой.", ch);
    altar_clear(altar);
    
    PCBonusData &data = pch->getBonuses().get(bonus->getIndex());
    data.start = day_of_epoch(time_info);
    data.end = data.start + 8;
    attr->prevBonusEnds = data.end;
    attr->prevBonus = bonus->getIndex();
    attr->successes++;
    ch->setWaitViolence(2);
    pch->save();
}

static bool can_sacrifice( Character *ch, Object *obj, bool needSpam ) 
{
        if (!ch->can_see( obj ))
            return false;

        if (obj->item_type == ITEM_CORPSE_PC)
        {
                if (needSpam)
                        ch->pecho("Трупы игроков жертвовать запрещено.");
                return false;
        }

        if ( !obj->can_wear(ITEM_TAKE) || obj->can_wear(ITEM_NO_SAC) 
             || IS_SET(obj->extra_flags, ITEM_NOSAC))
        {
                if (needSpam) 
                    ch->pecho( "%1$^O1 не подлеж%1$nит|ат жертвоприношению.", obj );
                return false;
        }

        if ( IS_SET(obj->item_type,ITEM_FURNITURE)
                && ( count_users(obj) > 0 ) )
        {
                if (needSpam) 
                    ch->pecho( "%1$^O1 в данный момент использу%1$nется|ются.", obj );
                return false;
        }

  return true;
}


int rescue_nosac_items ( Object *container, Room *room )
{
    Object *item;
    Object *obj_next;
    int count = 0;
    
    for ( item = container->contains; item; item = obj_next ) {
        obj_next = item->next_content;
        
        if (item->contains) 
            count += rescue_nosac_items(item, room);

        if (item->can_wear(ITEM_NO_SAC) || IS_SET(item->extra_flags, ITEM_NOSAC)) {
            obj_from_obj(item);
            obj_to_room(item, room);        
            count++;
        }
    }
    
    return count;
}

static bool oprog_sac( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Sac", "C", ch )
    FENIA_NDX_CALL( obj, "Sac", "OC", obj, ch )
    BEHAVIOR_CALL( obj, sac, ch )
    return false;
}


int sacrifice_obj( Character *ch, Object *obj, bool needSpam )
{
    	DLString rname;
    	if (ch->is_npc() || ch->getPC()->getReligion() == god_none)
			rname = "бог|и|ов|ам|ов|ами|ах";
		else rname = ch->getPC()->getReligion()->getRussianName();
		
        int silver = -1;

        if ( !can_sacrifice(ch, obj, needSpam) )
			return -1;

		// sac can't yield more $$ than obj->cost
        silver = ::min(number_range(1, obj->level), obj->cost);

        if (needSpam)
			ch->recho("%^C1 приносит %O4 в жертву %N3.", ch, obj, rname);

        if (oprog_sac( obj, ch ))
			return silver;
        
        if (needSpam)
            wiznet( WIZ_SACCING, 0, 0, "%^C1 приносит %O4 в жертву %N3.", ch, obj, rname );

        if (rescue_nosac_items(obj, ch->in_room)) {
            if (needSpam) {
				DLString fall = terrains[ch->in_room->getSectorType()].fall;
				ch->in_room->echo("Некоторые вещи внутри %O2 не могут быть принесены в жертву и падают %s.", obj, fall);
			}
		}
        extract_obj( obj );
        return silver;
}

/*
 * sac <obj>
 * sac all
 * sac altar exp|learn|qp
 */
CMDRUNP( sacrifice )
{
        char arg[MAX_INPUT_LENGTH];
        Object *obj, *next_obj;
        int silver, mana_gain;
    	DLString rname;
    	if (ch->is_npc() || ch->getPC()->getReligion() == god_none)
			rname = "бог|и|ов|ам|ов|ами|ах";
		else rname = ch->getPC()->getReligion()->getRussianName();
	
        mana_gain=-1;

        argument = one_argument( argument, arg );
        
        if ( arg[0] == '\0' || is_name( arg, ch->getNameP( '7' ).c_str() ) )
        {
				ch->recho("%^C1 предлагает себя в жертву %N3, но слышит в ответ тактичное молчание.", ch, rname);
                ch->pecho("Ты предлагаешь себя в жертву %N3, но слышишь в ответ лишь тактичное молчание.", rname);
                return;
        }

        if (IS_SET( ch->in_room->room_flags, ROOM_NOSAC )) {
            ch->pecho("В этой местности %N3 не удастся принять твою жертву.", rname);
            return;
        }

        if (arg_is_all( arg ))
        {
                int count = 0;
                obj = ch->in_room->contents;

                silver = 0;
                dreamland->removeOption( DL_SAVE_OBJS );

                while( obj!=0 )
                {
                        if (can_sacrifice( ch, obj, false))
                        {
                                next_obj = obj->next_content;
                                silver += sacrifice_obj( ch, obj, false);
                                count++;
                                obj = next_obj;
                        }
                        else
                                obj = obj->next_content;
                }

                dreamland->resetOption( DL_SAVE_OBJS );

                save_items( ch->in_room );
                
                if (count == 0) {
					ch->pecho("Ты не находишь ничего подходящего для жертвоприношения.");
                    return;
                }
                
				DLString where = terrains[ch->in_room->getSectorType()].where;
				ch->recho("%^C1 приносит в жертву %N3 все, что находится %s.", ch, rname, where);
                wiznet( WIZ_SACCING, 0, 0, "%^C1 приносит в жертву %N3 все, что находится %s в %s.", ch, rname, where, ch->in_room->getName() );

                if (silver==0) {
                    return;
                }
        }
        else
        {
                obj = get_obj_list( ch, arg, ch->in_room->contents );
                if ( obj == 0 )
                {
                        ch->pecho("Ты не находишь этого.");
                        return;
                }

                if (obj->pIndexData->vnum == OBJ_VNUM_ALTAR) {
                    sacrifice_at_altar(ch, obj, argument);
                    return;
                }

                if (obj->item_type == ITEM_CORPSE_NPC) {
                    if (number_percent() < gsn_crusify->getEffective( ch )) {
                        mana_gain = skill_level(*gsn_crusify, ch);
                        gsn_crusify->improve( ch, true );
                    } else {
                        gsn_crusify->improve( ch, false );
                    }
                } 

                if ( ( silver=sacrifice_obj(ch, obj, true) )<0 )
                        return;
        }

        if (mana_gain != -1 )
        {
                ch->mana += mana_gain;
				ch->pecho("Ты устраиваешь торжественное сожжение во славу %1$N3, восстанавливая %2$d очк%2$Iо|а|ов энергии.", rname, mana_gain);
				ch->recho("%^C1 устраивает торжественное сожжение во славу %N3, восстанавливая энергию.", ch, rname);
        }
		
		if (silver > 0) {
			ch->pecho("Ты получаешь %1$d серебрян%1$Iую|ые|ых монет%1$Iу|ы| от %2$N2 за свое жертвоприношение.", silver, rname);
			ch->silver += silver;

			if (ch->getReligion() == god_fili && get_eq_char(ch, wear_tattoo)) {
            	int bonus = silver * 2;
            	ch->pecho("{Y%1^N1{x скупо кряхтит и добавляет тебе еще %2$d монет%2$Iу|ы|.", rname, bonus);
            	ch->silver += bonus;
        	}
		}
		else ch->pecho("Твое скудное жертвоприношение остается без награды от %N2.", rname);
	
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
            if (silver > 1)
                if (party_members_room( ch ).size( ) > 1)
                    interpret_raw( ch, "split", "%d", silver );
}

