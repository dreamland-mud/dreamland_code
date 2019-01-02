#include <map>
#include "defaultreligion.h"
#include "religionattribute.h"
#include "logstream.h"
#include "calendar_utils.h"

#include "behavior_utils.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "commandtemplate.h"
#include "pcharacter.h"
#include "race.h"
#include "object.h"
#include "skill.h"
#include "skillmanager.h"
#include "liquid.h"

#include "act.h"
#include "alignment.h"
#include "dreamland.h"
#include "fight.h"
#include "gsn_plugin.h"
#include "handler.h"
#include "interp.h"
#include "itemflags.h"
#include "loadsave.h"
#include "mercdb.h"
#include "merc.h"
#include "save.h"
#include "terrains.h"
#include "wiznet.h"
#include "def.h"

#define OBJ_VNUM_ALTAR 88

RELIG(none);
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

        // TODO: body parts, flowers.

        switch (obj->item_type) {
        case ITEM_MONEY:    
            money(obj); 
            return;

        case ITEM_LIGHT:
            light(obj);
            return;

        case ITEM_WARP_STONE:
        case ITEM_GEM:      
            gem(obj); 
            return;

        case ITEM_JEWELRY:
        case ITEM_TREASURE: 
            treasure(obj); 
            return;

        case ITEM_ARMOR:    
        case ITEM_CLOTHING: 
            armor(obj);    
            return;    

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            potion(obj); 
            return;

        case ITEM_WAND:
        case ITEM_STAFF:
            wand(obj);
            return;
        
        case ITEM_FOOD:       food(obj); return;
        case ITEM_DRINK_CON:  drink(obj); return;
        case ITEM_WEAPON:     weapon(obj); return;
        case ITEM_CORPSE_PC:  corpse_pc(obj); return;
        case ITEM_CORPSE_NPC: corpse_npc(obj); return;

        case ITEM_PARCHMENT:
        case ITEM_SPELLBOOK:
        case ITEM_TEXTBOOK:
            book(obj);
            return;
        } 
    
        // Item we don't care about, no cost added.
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
        int item_cost =  obj->value[0] / 100 + obj->value[1];
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
            if ((skill = SkillManager::getThis( )->find( obj->value[i] ) )) {
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
        if ((skill = SkillManager::getThis( )->find( obj->value[3] ) ))
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
        if (obj->value[3]) {
            smite = true;
            smiteMessage = "ядовитую еду";
            return;
        }
        // Use double food's "full" value as its cost.
        quantity["food"]++;
        cost["food"] = min(200, cost["food"] + obj->value[0] * 2);
    }

    void drink(Object *obj) {
        // Smite for poisonous or empty drinks.
        if (IS_SET( obj->value[3], DRINK_POISONED )) {
            smite = true;
            smiteMessage = "отравленную жидкость";
            return;
        }
        if (obj->value[1] <= 0 || obj->value[0] <= 0) {
            smite = true;
            smiteMessage = "пустую тару";
            return;
        }
        if (obj->value[1] < obj->value[0] / 4) {
            smite = true;
            smiteMessage = "полупустую тару";
            return;
        }
        // Favourite drinks (wine, milk) give more points.
        quantity["drink"]++;
        int item_cost;
        if (religion->likesDrink(liquidManager->find(obj->value[2]))) {
            // Penalize for non-full containers.
            item_cost = 500 * obj->value[1] / obj->value[0];
        } else {
            item_cost = min(100, obj->value[1]);
        }
        cost["drink"] = min(500, cost["drink"] + item_cost);
    }

    void corpse_pc(Object *obj) {
        smite = true;
        smiteMessage = fmt(0, "%O4", obj);    
    }

    void corpse_npc(Object *obj) {
        bitnumber_t align = ALIGN_NUMBER(obj->value[4]);
        MOB_INDEX_DATA *pMob = get_mob_index(obj->value[3]);
        if (!pMob)
            return;

        // Very pleased if corpse's align is not allowed for religion.
        if (!religion->getAlign().isSetBitNumber(align)) {
            // Penalty for smaller corpses.
            int sizePenalty = max(0, SIZE_MEDIUM - pMob->size);    
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
        ch->println("Изыди, глупое животное.");
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
        ch->println("Ты можешь попросить богов об одной из таких вещей: {lRопыт, мана, обучаемость, воровские умения{lEexp, mana, learning, thief skills{x.");
        ch->println("Но помни, что не все они для тебя подходят.");
        return;
    }

    if (!bonus->available(pch)) {
        ch->pecho("Ты слышишь насмешливый голос с неба, говорящий тебе, что то, о чем ты просишь, тебе ни к чему.");
        return;
    }

    if (attr->prevBonus == bonus->getIndex()) {
        ch->pecho("Не стоит просить %N4 об одном и том же два раза подряд - попроси %p2 о чем-то еще.", rname, religion.getSex());
        return;
    }

    ch->pecho("Ты приносишь содержимое %O2 в жертву %N3 и надеешься на %p2 милость.",
              altar, rname, religion.getSex());
    ch->recho("%^C1 приносит содержимое %O2 в жертву своим богам.", ch, altar);

    DefaultReligion *drelig = dynamic_cast<DefaultReligion *>(pch->getReligion().operator ->());
    if (!drelig) {
        ch->pecho("Похоже, %N1 совершенно равнодуш%gно|ен|на к жертвоприношениям.", 
                   rname, religion.getSex());
        ch->recho("... но ничего не происходит.");
        return;
    }

    attr->attempts++;

    Offering offering(drelig, altar);
    LogStream::sendNotice() << ch->getName() << " sacrifices altar to " << drelig->getName() << ", total " << offering.getSum() << endl;
    if (offering.smites()) {
        ch->pecho("{R%^N1 гневается на тебя за попытку принести в жертву %s!{x",
                       rname, offering.getSmiteMessage().c_str());
        ch->recho("Гнев божий обрушивается на %C2!", ch);
        rawdamage(ch, ch, DAM_HOLY, ch->hit / 4, false);
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
        ch->recho("... но ничего не происходит.");
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
                        ch->send_to("Богам это не понравится.\n\r");
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
                    ch->pecho( "%1$^O1 использу%1$nется|ются.", obj );
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
        int silver = -1;

        if ( !can_sacrifice(ch, obj, needSpam) )
                return -1;

        silver = number_range(number_fuzzy(obj->level), obj->cost / 10);

        if (needSpam)
            act_p( "$c1 приносит в жертву богам $o4.", ch, obj, 0, TO_ROOM,POS_RESTING);

        if (oprog_sac( obj, ch ))
                return silver;
        
        if (needSpam)
            wiznet( WIZ_SACCING, 0, 0, "%^C1 приносит во всесожжение %O4.", ch, obj );

        if (rescue_nosac_items(obj, ch->in_room)) 
            if (needSpam)
                act( "Некоторые вещи, лежащие в $o6, не могут быть принесены в жертву и падают $T.", 
                     ch, obj, terrains[ch->in_room->sector_type].fall, TO_ALL );

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
        char buf[MAX_STRING_LENGTH];
        Object *obj, *next_obj;
        int silver, mana_gain;

        mana_gain=-1;

        argument = one_argument( argument, arg );
        
        if ( arg[0] == '\0' || is_name( arg, ch->getNameP( '7' ).c_str() ) )
        {
                act_p( "$c1 предлагает себя в жертву богам, но они вежливо отказываются.",
                        ch, 0, 0, TO_ROOM,POS_RESTING);
                ch->send_to("Боги оценили твою жертву и возможно примут ее позже.\n\r");
                return;
        }

        if (IS_SET( ch->in_room->room_flags, ROOM_NOSAC )) {
            ch->send_to("Бог не хочет принять твою жертву.\r\n");
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
                    act("Ты не наш$gло|ел|ла ничего подходящего для жертвоприношения.", ch, 0, 0, TO_CHAR);
                    return;
                }
                
                act( "$c1 приносит в жертву богам все, что находится $T.", ch, 0, terrains[ch->in_room->sector_type].where, TO_ROOM );
                wiznet( WIZ_SACCING, 0, 0, "%^C1 sends up all items in %s as a burnt offering.", ch, ch->in_room->name );

                if (silver==0) {
                    return;
                }
        }
        else
        {
                obj = get_obj_list( ch, arg, ch->in_room->contents );
                if ( obj == 0 )
                {
                        ch->send_to("Ты не находишь это.\n\r");
                        return;
                }

                if (obj->pIndexData->vnum == OBJ_VNUM_ALTAR) {
                    sacrifice_at_altar(ch, obj, argument);
                    return;
                }

                if ( (  obj->item_type == ITEM_CORPSE_NPC  )
                        && number_percent() < gsn_crusify->getEffective( ch ) )
                {
                        mana_gain = ch->getModifyLevel();
                        gsn_crusify->improve( ch, true );
                }         

                if ( ( silver=sacrifice_obj(ch, obj, true) )<0 )
                        return;
        }

        if (mana_gain != -1 )
        {
                ch->mana += mana_gain;
                sprintf(buf,"Боги дают тебе %d энергии за сожжение.\n\r", mana_gain);
                ch->send_to(buf);
        }

        sprintf(buf,"Боги дают тебе %d серебрян%s за жертвоприношение.\n\r",
                silver,GET_COUNT(silver,"ую монету","ые монеты","ых монет"));
        ch->send_to(buf);

        ch->silver += silver;

        if (IS_SET(ch->act,PLR_AUTOSPLIT))
            if (silver > 1)
                if (party_members_room( ch ).size( ) > 1)
                    interpret_raw( ch, "split", "%d", silver );
}

