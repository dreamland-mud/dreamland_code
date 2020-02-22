/* $Id$
 *
 * ruffina, 2019
 */
#include "dlfilestream.h"
#include "json/json.h"
#include "iconvmap.h"

#include "dlscheduler.h"
#include "schedulertaskroundplugin.h"
#include "plugininitializer.h"
#include "areabehaviormanager.h"
#include "commandtemplate.h"
#include "affect.h"
#include "core/object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "race.h"
#include "room.h"
#include "searcher_val.h"    
#include "profiler.h"

#include "websocketrpc.h"
#include "dreamland.h"
#include "merc.h"
#include "act.h"
#include "comm.h"
#include "handler.h"
#include "mercdb.h"
#include "def.h"

static IconvMap koi2utf("koi8-r", "utf-8");

GSN(none);

using namespace std;

static void csv_escape( DLString &name ) {
    name.colourstrip( );
    name.replaces( "\"", "'");
//    name.replaces( "\'", "\\\'");
}

static bool get_obj_resets( int vnum, AREA_DATA *&pArea, DLString &where )
{
    // Scan resets for each room.
    for (Room *room = room_list; room; room = room->rnext) {
        int mobVnum = -1;
        for(RESET_DATA *pReset = room->reset_first;pReset;pReset = pReset->next)
            switch(pReset->command) {
                case 'M':
                    // Remember potential carrier in the room.
                    mobVnum = pReset->arg1;
                    break;
                case 'G':
                case 'E':
                    // Found who carries the object.
                    if (pReset->arg1 == vnum && mobVnum > 0) {
                        MOB_INDEX_DATA *pMob = get_mob_index( mobVnum );
                        if (pMob) {
                            // Return success
                            pArea = room->area;
                            where = russian_case(pMob->short_descr, '1');
                            return true;
                        }
                    }
                    break;
                case 'O':
                    if (pReset->arg1 == vnum) { 
                        // Object is on the floor, return success.
                        pArea = room->area;
                        where = room->name;
                        return true;
                    }
                    break; 
                case 'P':
                    // Found where the object is placed.
                    if (pReset->arg1 == vnum) {
                        OBJ_INDEX_DATA *in = get_obj_index( pReset->arg3 );
                        if (in) {
                            // Return success.
                            pArea = room->area;
                            where = room->name;
                            return true;
                        }
                    }
                    break;
            }
     }

    // No candidates have been found, return a failure.
    return false;
}            


static StringList searcher_param_anti(OBJ_INDEX_DATA *pObj)
{
    StringList anti;
    if (IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL)) {
        if (!IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD)) anti.push_back("{YG{x");
        if (!IS_SET(pObj->extra_flags, ITEM_ANTI_EVIL)) anti.push_back("{RE{x");
        if (!IS_SET(pObj->extra_flags, ITEM_ANTI_NEUTRAL)) anti.push_back("N");
    }
    return anti;                    
}

static DLString searcher_param_asterix(OBJ_INDEX_DATA *pObj)
{
    bool clr_aff = !p.aff.empty() || !p.det.empty() || !p.vuln.empty() || !p.res.empty() || !p.imm.empty();
    bool clr_fenia = !p.fenia.empty();
    bool clr_skills = !p.learned.empty();

    DLString aff = " ";
    if (clr_skills) {
        aff = (clr_aff || clr_fenia) ? "{M*{x" : "{m*{x";
    } else if (clr_fenia) {
        aff = clr_aff ? "{G*{x" : "{g*{x";
    } else if (clr_aff) {
        aff = "{C*{x";
    }
    return aff;
}

class SearcherDumpTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<SearcherDumpTask> Pointer;
 
    virtual int getPriority( ) const
    {
        return SCDP_ROUND + 30;
    }

    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInSecond( 4 * Date::SECOND_IN_HOUR, Pointer( this ) );
    }

    virtual void run( )
    {   
        if (dreamland->hasOption(DL_BUILDPLOT))
            return;

        LogStream::sendNotice() << "Dumping searcher db to disk." << endl;
        dumpArmor();
        dumpWeapon();
        dumpMagic();
        dumpPets();
    }

    bool dumpPets() 
    {
        ostringstream buf;
        Json::Value dump;

        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob; pMob = pMob->next) {
            if (!pMob->behavior)
                continue;
            if (area_is_clan(pMob->area))
                continue;

            DLString type = pMob->behavior->getFirstNode()->getAttribute(XMLNode::ATTRIBUTE_TYPE);
            if (type.find("Pet") == DLString::npos && type != "Rat")
                continue;
            
            DLString aname = pMob->area->name;
            csv_escape(aname);

            Json::Value pet;
            pet["vnum"] = pMob->vnum;
            pet["name"] = russian_case(pMob->short_descr, '1').colourStrip();
            pet["level"] = (type == "LevelAdaptivePet" || type == "Rat" ? -1 : pMob->level);
            pet["act"] = act_flags.names(REMOVE_BIT(pMob->act, ACT_IS_NPC|ACT_NOALIGN|ACT_OUTDOORS|ACT_INDOORS|ACT_SENTINEL|ACT_SCAVENGER|ACT_NOPURGE|ACT_STAY_AREA|ACT_NOTRACK|ACT_SAGE|ACT_NOWHERE));
            pet["aff"] = affect_flags.names(REMOVE_BIT(pMob->affected_by, AFF_INFRARED));
            pet["off"] = off_flags.names(REMOVE_BIT(pMob->off_flags, ASSIST_ALIGN|ASSIST_VNUM|ASSIST_RACE|OFF_FADE));
            pet["area"] = aname;
            dump.append(pet);
        }

        try {
            Json::FastWriter writer;
            DLFileStream("/tmp", "db_pets", ".json").fromString(
                koi2utf(writer.write(dump))
            );
        } catch (const ExceptionDBIO &ex) {
            LogStream::sendError() << ex.what() << endl;
            return false;
        }

        return true;
    }

    bool dumpArmor()
    {
        Json::Value dump;

        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
            bitstring_t wear = pObj->wear_flags;
            REMOVE_BIT(wear, ITEM_TAKE|ITEM_NO_SAC);
            DLString wearloc;
            
            // Limit obj level by 100.
            if (pObj->level > LEVEL_MORTAL)
                continue;

            // Ignore items you can't even take.
            if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                continue;
            // Ignore items without wear flags.
            if (wear == 0 && pObj->item_type != ITEM_LIGHT)
                continue;
            // Ignore weapons, they're shown in another table.
            if (pObj->item_type == ITEM_WEAPON) 
                continue;

            // Quirk with light wearlocation.
            if (wear == 0) 
                wearloc = "light";
            else
                wearloc = wear_flags.messages(wear);

            // Format object item type and damage roll.
            DLString itemtype = item_table.message( pObj->item_type );
            
            // Format object name.
            DLString name = russian_case(pObj->short_descr, '1').toLower( );
            csv_escape( name );
        
            // Find all bonuses.
            int hr=0, dr=0, hp=0, svs=0, mana=0, move=0;
            int str=0, inta=0, wis=0, dex=0, con=0, cha=0;
            DLString aff,det,imm,res,vuln;
            DLString align;

            for (Affect *paf = pObj->affected; paf; paf = paf->next) {
                int m = paf->modifier;

                switch (paf->location) {
                case APPLY_STR: str+=m; break;
                case APPLY_INT: inta+=m; break;
                case APPLY_WIS: wis+=m; break;
                case APPLY_DEX: dex+=m; break;
                case APPLY_CON: con+=m; break;
                case APPLY_CHA: cha+=m; break;
                case APPLY_HIT: hp+=m; break;
                case APPLY_MANA: mana+=m; break;
                case APPLY_MOVE: move+=m; break;
                case APPLY_HITROLL: hr+=m; break;
                case APPLY_DAMROLL: dr+=m; break;
                case APPLY_SAVES:         
                case APPLY_SAVING_ROD:    
                case APPLY_SAVING_PETRI:  
                case APPLY_SAVING_BREATH: 
                case APPLY_SAVING_SPELL:  svs+=m; break;
                }
                
                if (paf->bitvector) {
                    bitstring_t b = paf->bitvector;

                    switch(paf->where) {
                    case TO_DETECTS: det << detect_flags.names(b) << " "; break;
                    case TO_AFFECTS: aff << affect_flags.names(b) << " "; break;
                    case TO_IMMUNE:  imm << imm_flags.names(b) << " "; break;
                    case TO_RESIST:  res << res_flags.names(b) << " "; break;
                    case TO_VULN:    vuln << vuln_flags.names(b) << " "; break;
                    }
                }
            }

            if (IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL)) {
                if (!IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD)) align << "G";
                if (!IS_SET(pObj->extra_flags, ITEM_ANTI_EVIL)) align << "E";
                if (!IS_SET(pObj->extra_flags, ITEM_ANTI_NEUTRAL)) align << "N";
            }


            // Potions, containers etc often can be held but do nothing - ignore them.
            bool useless = false;
            if (wear == ITEM_HOLD) {
                useless = str == 0 && inta == 0 && wis == 0 && dex == 0 && con == 0 && cha == 0
                    && hp == 0 && mana == 0 && move == 0 && hr == 0 && dr == 0
                    && svs == 0;
                // TODO check affects
            }

            if (useless)
                continue;

            // Find item resets and ignore items without resets and from clan areas.
            DLString where;
            AREA_DATA *pArea;
            useless = !get_obj_resets( pObj->vnum, pArea, where );
            if (useless)
                continue;
            if (area_is_clan(pArea))
                continue;
            if (IS_SET(pArea->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            DLString area = pArea->name;
            csv_escape( area );
            csv_escape( where );

            Json::Value a;
            a["vnum"] = pObj->vnum;
            a["name"] = name;
            a["level"] = pObj->level;
            a["wearloc"] = wearloc;
            a["itemtype"] = itemtype;
            a["hr"] = hr;
            a["dr"] = dr;
            a["hp"] = hp;
            a["mana"] = mana;
            a["move"] = move;
            a["saves"] = svs;
            a["stat_str"] = str;
            a["stat_int"] = inta;
            a["stat_wis"] = wis;
            a["stat_dex"] = dex;
            a["stat_con"] = con;
            a["stat_cha"] = cha;
            a["align"] = align;
            a["area"] = area;
            a["where"] = where;
            a["limit"] = pObj->limit;
            dump.append(a);
        }

        try {
            Json::FastWriter writer;
            DLFileStream("/tmp", "db_armor", ".json").fromString(
                koi2utf(writer.write(dump))
            );
        } catch (const ExceptionDBIO &ex) {
            LogStream::sendError() << ex.what() << endl;
            return false;
        }

        return true;
    }

    bool dumpWeapon()
    {
        Json::Value dump;

        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
            bitstring_t wear = pObj->wear_flags;
            REMOVE_BIT(wear, ITEM_TAKE|ITEM_NO_SAC);
            DLString wearloc;
            
            // Limit obj level by 100.
            if (pObj->level > LEVEL_MORTAL)
                continue;

            // Ignore items you can't even take.
            if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                continue;
            // Only dump weapons. An arrow can be 'held' so can't ignore non-wieldable ones.
            if (wear == 0 || pObj->item_type != ITEM_WEAPON)
                continue;

            // Format weapon class, special flags and damage.
            DLString weaponClass = weapon_class.name( pObj->value[0] );
            DLString special = weapon_type2.messages( pObj->value[4] );
            int d1 = pObj->value[1];
            int d2 = pObj->value[2];
            int ave = (1 + pObj->value[2]) * pObj->value[1] / 2; 
            
            // Format object name.
            DLString name = russian_case(pObj->short_descr, '1').toLower( );
            csv_escape( name );
        
            // Find all bonuses.
            int hr=0, dr=0, hp=0, svs=0, mana=0, move=0;
            int str=0, inta=0, wis=0, dex=0, con=0, cha=0, size=0;
            DLString align;

            for (Affect *paf = pObj->affected; paf; paf = paf->next) {
                int m = paf->modifier;

                switch (paf->location) {
                case APPLY_STR: str+=m; break;
                case APPLY_INT: inta+=m; break;
                case APPLY_WIS: wis+=m; break;
                case APPLY_DEX: dex+=m; break;
                case APPLY_CON: con+=m; break;
                case APPLY_CHA: cha+=m; break;
                case APPLY_HIT: hp+=m; break;
                case APPLY_MANA: mana+=m; break;
                case APPLY_MOVE: move+=m; break;
                case APPLY_HITROLL: hr+=m; break;
                case APPLY_DAMROLL: dr+=m; break;
                case APPLY_SIZE: size+=m; break;
                case APPLY_SAVES:         
                case APPLY_SAVING_ROD:    
                case APPLY_SAVING_PETRI:  
                case APPLY_SAVING_BREATH: 
                case APPLY_SAVING_SPELL:  svs+=m; break;
                }
            }
            
            if (IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL)) {
                if (!IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD)) align << "G";
                if (!IS_SET(pObj->extra_flags, ITEM_ANTI_EVIL)) align << "E";
                if (!IS_SET(pObj->extra_flags, ITEM_ANTI_NEUTRAL)) align << "N";
            }

            // Find item resets and ignore items without resets and from clan areas.
            DLString where;
            AREA_DATA *pArea;
            bool useless = !get_obj_resets( pObj->vnum, pArea, where );
            if (useless)
                continue;
            if (area_is_clan(pArea))
                continue;
            if (IS_SET(pArea->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            DLString area = pArea->name;
            csv_escape( area );
            csv_escape( where );

            Json::Value w;
            w["vnum"] = pObj->vnum;
            w["name"] = name;
            w["level"] = pObj->level;
            w["wclass"] = weaponClass;
            w["special"] = special;
            w["d1"] = d1;
            w["d2"] = d2;
            w["ave"] = ave;
            w["hr"] = hr;
            w["dr"] = dr;
            w["hp"] = hp;
            w["mana"] = mana;
            w["saves"] = svs;
            w["stat_str"] = str;
            w["stat_int"] = inta;
            w["stat_wis"] = wis;
            w["stat_dex"] = dex;
            w["stat_con"] = con;
            w["align"] = align;
            w["area"] = area;
            w["where"] = where;
            w["limit"] = pObj->limit;
            dump.append(w);
        }

        try {
            Json::FastWriter writer;
            DLFileStream("/tmp", "db_weapon", ".json").fromString(
                koi2utf(writer.write(dump))
            );
        } catch (const ExceptionDBIO &ex) {
            LogStream::sendError() << ex.what() << endl;
            return false;
        }

        return true;
    }

    bool dumpMagic()
    {
        Json::Value dump;
        // Collect distinct list of spells.
        std::set<DLString> allSpells;

        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
            // Limit obj level by 100.
            if (pObj->level > LEVEL_MORTAL)
                continue;

            // Ignore weird items.
            if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                continue;
             
            DLString itemtype = item_table.name( pObj->item_type );
            int spellLevel = 0;
            int charges = 0;
            DLString spells = "";
            Skill *skill;

            // Only dump magic items; collect properties.
            switch (pObj->item_type) {
            case ITEM_SCROLL:
            case ITEM_POTION:
            case ITEM_PILL:
                spellLevel = pObj->value[0];
                charges = 1;

                for (int i = 1; i <= 4; i++) 
                    if ((skill = SkillManager::getThis( )->find( pObj->value[i] ) ))
                        if (skill->getIndex( ) != gsn_none) {
                            spells += "'" + skill->getName( ) + "' ";
                            allSpells.insert( skill->getName( ) );
                        }
                
                break;

            case ITEM_WAND:
            case ITEM_STAFF:
                spellLevel = pObj->value[0];
                charges = pObj->value[2];
                
                if ((skill = SkillManager::getThis( )->find( pObj->value[3] ) ))
                    if (skill->getIndex( ) != gsn_none) {
                        spells = "'" + skill->getName( ) + "'";
                        allSpells.insert( skill->getName( ) );
                    }

                break;

            case ITEM_WARP_STONE:
                // Warpstones don't have any fields (although might consider 
                // introducing 'charges' field).
                break;

            case ITEM_SPELLBOOK:
                // For spellbooks, display max quality as spell level for now.
                // Show 'total pages' as 'charges' field.
                spellLevel = pObj->value[2];
                charges = pObj->value[0];
                break;

            default:
                continue;
            }

            // Ignore useless stuff. Leave it commented for debugging.
//            if (spells.empty( ) || charges == 0)
//                continue;

            // Format object name.
            DLString name = russian_case(pObj->short_descr, '1').toLower( );
            csv_escape( name );

            // Find item resets and ignore items without resets and from clan areas.
            DLString where;
            AREA_DATA *pArea;
            bool useless = !get_obj_resets( pObj->vnum, pArea, where );
            if (useless)
                continue;
            if (area_is_clan(pArea))
                continue;
            if (IS_SET(pArea->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            DLString area = pArea->name;
            csv_escape( area );
            csv_escape( where );
                
            Json::Value wand;
            wand["vnum"] = pObj->vnum;
            wand["name"] = name;
            wand["level"] = pObj->level;
            wand["itemtype"] = itemtype;
            wand["spellLevel"] = spellLevel;
            wand["charges"] = charges;
            wand["spells"] = spells;
            wand["area"] = area;
            wand["where"] = where;
            wand["limit"] = pObj->limit;
            dump.append(wand);
        }

        try {
            Json::FastWriter writer;
            DLFileStream("/tmp", "db_magic", ".json").fromString(
                koi2utf(
                    writer.write(dump))
            );
        } catch (const ExceptionDBIO &ex) {
            LogStream::sendError() << ex.what() << endl;
            return false;
        }

        return true;
    }
};

PluginInitializer<SearcherDumpTask> initSearcherDumpTask;

/* Example syntax:
 * searcher armor level > 50 and level < 60 and wearloc='neck' and vuln != ''
 * searcher armor level > 50 and level < 60 and extra='nodrop'
 *
 * Fields:
 *  level, lvl
 *  wearloc
 *  type
 *  str,int,wis,dex,con,cha
 *  hr,dr,hp,mana,svs,mov,heal_gain,mana_gain,size,age
 *  vuln,res,imm,det,aff
 *  material
 *  extra
 *  where : area,room
 * 
 *  Operands:
 *      and or () >= <= == !=
 *      in like contains
 */
CMDRUNP(searcher)
{
    DLString args = argument;
    DLString arg = args.getOneArgument();
    SearcherDumpTask task;

    if (arg_is_all(arg)) {
        task.run();
        ch->println("Created 4 JSON files in /tmp, check logs for any errors.");
        return;
    }

    if (arg_oneof(arg, "pets")) {
        if (task.dumpPets())
            ch->println("Created /tmp/db_pets.json file.");
        else
            ch->println("Error occurred, please check the logs.");
        
        return;
    }

    if (arg_oneof(arg, "armor")) {
        if (task.dumpArmor()) 
            ch->println("Created /tmp/db_armor.json file.");
        else
            ch->println("Error occurred, please check the logs.");

        return;
    }

    if (arg_oneof(arg, "magic")) {
        if (task.dumpMagic()) 
            ch->println("Created /tmp/db_magic.json file.");
        else
            ch->println("Error occurred, please check the logs.");

        return;
    }
    
    if (arg_oneof(arg, "weapon")) {
        if (task.dumpWeapon()) 
            ch->println("Created /tmp/db_weapon.json file.");
        else
            ch->println("Error occurred, please check the logs.");

        return;
    }

    if (arg_oneof(arg, "query")) {

        if (args.empty()) {
            ch->println("Usage: searcher q <query string>\nSee 'help searcher' for details.");
            return;
        }
    
        try {
            Profiler prof;
            int cnt = 0;
            vector<list<DLString> > output(MAX_LEVEL+1);
            DLString lineFormat = 
                web_cmd(ch, "oedit $1", "%5d") + " {C%3d{x {y%-10s{x {y%-10s{x %-20.20s %-3s %1s {%s%3d {%s%3d {%s%3d {%s%3d {%s%3d {D%s{x\n";

            prof.start();

            for (int i = 0; i < MAX_KEY_HASH; i++)
            for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
                if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                    continue;
                if (pObj->level > MAX_LEVEL)
                    continue;

                if (searcher_parse(pObj, args.c_str())) {
                    StringList anti = searcher_param_anti(pObj);
                    DLString aff = searcher_param_asterix(pObj);

                    DLString line = 
                        fmt(NULL, lineFormat.c_str(), 
                                    pObj->vnum,
                                    pObj->level, 
                                    p.itemtype.c_str(),
                                    p.wear.substr(0, 10).c_str(),
                                    russian_case(pObj->short_descr, '1').c_str(),
                                    anti.join("").c_str(),
                                    aff.c_str(),
                                    (p.hr != 0 ? "C": "w"), p.hr, 
                                    (p.dr != 0 ? "C": "w"), p.dr, 
                                    (p.hp != 0 ? "C": "w"), p.hp, 
                                    (p.mana != 0 ? "C": "w"), p.mana, 
                                    (p.saves != 0 ? "C": "w"), p.saves, 
                                    pObj->area->name);

                    DLString where;
                    AREA_DATA *pArea;
                    if (IS_SET(pObj->area->area_flag, AREA_HIDDEN) || !get_obj_resets(pObj->vnum, pArea, where)) {
                        line.colourstrip();
                        line = "{D" + line + "{x";
                    }

                    output[pObj->level].push_back(line);
                    cnt++;
                }
            } 
    
            ostringstream buf;
            buf << fmt(0, "{W%5s %3s %-10s %-10s %-20.20s %-3s %1s %3s %3s %3s %3s %3s %s{x\n", 
                            "VNUM", "LVL", "TYPE", "WEAR", "NAME", "ALG", "A", "HR", "DR", "HP", "MAN", "SVS", "AREA");
            for (size_t lvl = 0; lvl < output.size(); lvl++) {
                const list<DLString> &lines = output[lvl];
                for (list<DLString>::const_iterator l = lines.begin(); l != lines.end(); l++)
                    buf << *l;
            }

            prof.stop();
            buf << "Found " << cnt << " entries, search took " << prof.msec() << " ms." << endl;
            buf << "{WA{x field: {C*{x imm/res/vuln/det, {g*{x Fenia trigger, {m*{x skill bonus, "
                << "{G*{x Fenia trigger plus imm/res, {M*{x skill bonus plus anything else." << endl;
     
            page_to_char(buf.str().c_str(), ch);
        } catch (const Exception &ex) {
            ch->println(ex.what());
        }

        return;
    }

    if (arg_oneof(arg, "wquery")) {

        if (args.empty()) {
            ch->println("Usage: searcher wq <query string>\nSee 'help searcher' for details.");
            return;
        }
    
        try {
            Profiler prof;
            int cnt = 0;
            vector<list<DLString> > output(MAX_LEVEL+1);
            DLString lineFormat = 
                web_cmd(ch, "oedit $1", "%5d") + " {C%3d{x {y%-7s{x %-20.20s{x {W%2d %2d %3d{x %-10s {%s%3d {%s%3d {%s%3d{x %-3s %1s %s{x\r\n";

            prof.start();

            for (int i = 0; i < MAX_KEY_HASH; i++)
            for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
                if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
                    continue;
                if (pObj->level > MAX_LEVEL)
                    continue;
                if (pObj->item_type != ITEM_WEAPON)
                    continue;

                if (searcher_parse(pObj, args.c_str())) {
                    StringList anti = searcher_param_anti(pObj);
                    DLString aff = searcher_param_asterix(pObj);

                    DLString line = 
                        fmt(NULL, lineFormat.c_str(), 
                                    pObj->vnum,
                                    pObj->level, 
                                    p.wclass.c_str(),
                                    russian_case(pObj->short_descr, '1').c_str(),
                                    p.d1, p.d2, p.ave, 
                                    p.damage.c_str(),
                                    (p.hr != 0 ? "C": "w"), p.hr, 
                                    (p.dr != 0 ? "C": "w"), p.dr, 
                                    (p.hp != 0 ? "C": "w"), p.hp, 
                                    anti.join("").c_str(),
                                    aff.c_str(),
                                    p.wflags.c_str());

                    DLString where;
                    AREA_DATA *pArea;
                    if (IS_SET(pObj->area->area_flag, AREA_HIDDEN) || !get_obj_resets(pObj->vnum, pArea, where)) {
                        line.colourstrip();
                        line = "{D" + line + "{x";
                    }

                    output[pObj->level].push_back(line);
                    cnt++;
                }
            } 
    
            ostringstream buf;
            buf << fmt(0, "{W%5s %3s %-7s %-20.20s %-2s %-2s %3s %-10s %3s %3s %3s %-3s %1s %s{x\r\n",
                            "VNUM", "LVL", "WCLASS", "NAME", "D1", "D2", "AVE", "DAMAGE", "HR", "DR", "HP", "ALG", "A", "WFLAGS");
            for (size_t lvl = 0; lvl < output.size(); lvl++) {
                const list<DLString> &lines = output[lvl];
                for (list<DLString>::const_iterator l = lines.begin(); l != lines.end(); l++)
                    buf << *l;
            }

            prof.stop();
            buf << "Found " << cnt << " entries, search took " << prof.msec() << " ms." << endl;
            buf << "{WA{x field: {C*{x imm/res/vuln/det, {g*{x Fenia trigger, {m*{x skill bonus, "
                << "{G*{x Fenia trigger plus imm/res, {M*{x skill bonus plus anything else." << endl;
     
            page_to_char(buf.str().c_str(), ch);
        } catch (const Exception &ex) {
            ch->println(ex.what());
        }

        return;
    }

    ch->println("Usage:\nsearcher all\nsearcher armor|weapon|magic|pets\nsearcher q <item query>\nsearcher wq <weapon query>\n");
}
