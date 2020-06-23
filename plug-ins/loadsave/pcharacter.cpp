/* $Id$
 *
 * ruffina, 2004
 */

#include "save.h"
#include "fread_utils.h"
#include "loadsave.h"
#include "dlfile.h"
#include "dlfileop.h"
#include "commonattributes.h"

#include "logstream.h"
#include "profiler.h"
#include "dreamland.h"
#include "pcrace.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "affect.h"
#include "skillreference.h"
#include "room.h"

#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

GSN(sanctuary);
GSN(stardust);
GSN(spell_resistance);
GSN(magic_resistance);
GSN(dispel_affects);
GSN(dispel_magic);
GSN(bat_sworm);
GSN(bat_swarm);
GSN(ground_strike);
GSN(critical_strike);
GSN(dominate);
GSN(control_animal);

static void skill_exchange( PCharacter *ch, SkillReference &skill1, SkillReference &skill2 )
{
    int &learn1 = ch->getSkillData( skill1 ).learned;
    int &learn2 = ch->getSkillData( skill2 ).learned;
    
    if (learn1 > 1 && !skill1->visible( ch ) && skill2->visible( ch )) {
        learn2 = learn1;
        learn1 = 1;
    }
}

static void update_exp( PCharacter *ch ) 
{
    int exp_tonl = ch->getExpToLevel( );
    int exp_perlvl = ch->getExpPerLevel( );
    
    if (exp_tonl < 0) {
        notice("Fixing exp for %s: exp_tonl=%d exp_perlvl=%d exp=%d", 
                ch->getName( ).c_str( ), exp_tonl, exp_perlvl, ch->exp);
        ch->exp = exp_perlvl;
    }
}    

static void update_stats( PCharacter *ch )
{
    int i, max_stat;

    for (i = 0; i < stat_table.size; i++) {        
        max_stat = ch->getMaxTrain( i );

        if (ch->perm_stat[i] > max_stat) {
            int diff = ch->perm_stat[i] - max_stat;
            notice("Fixing stats for %s: %s %d -> %d",
                ch->getName().c_str(), stat_table.name(i).c_str(), ch->perm_stat[i], max_stat);

            ch->train += diff;
            ch->perm_stat[i] = max_stat;
        }
    }
}

static void clear_fenia_skills( PCharacter *ch )
{
    PCSkills skills = ch->getSkills();
    PCSkills::iterator s;
    
    for (s = skills.begin(); s != skills.end(); s++)
        if (s->origin == SKILL_FENIA)
            s->clear();
}

void PCharacter::updateSkills( )
{
    int availCounter = 0;

    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill *skill = SkillManager::getThis( )->find( sn );
        PCSkillData &data = getSkillData(sn);

        // Ensure skill learned percentage is always within limits.
        if (skill->visible( this )) {
            int &percent = data.learned;

            percent = std::max( 1, percent );
            percent = std::max( skill->getLearned( this ), percent );
        }

        // For historical 'temporary' skills, set up proper skill origin value.
        if (data.temporary) {
            data.temporary = false;
            data.origin.setValue(SKILL_DREAM);
        }

        // Count and store total number of skills available at this level.
        if (skill->available(this))
            availCounter++;
    }

    getAttributes().getAttr<XMLIntegerAttribute>("skillCount")->setValue(availCounter);
}                        

/*-------------------------------------------------------------------------
 *  work with profiles 
 *------------------------------------------------------------------------*/
bool PCharacter::load( )
{
    static const char * METHOD = "PCharacter::load( )";
//    ProfilerBlock be(METHOD);
    
    DLFileRead profile( dreamland->getPlayerDir( ), 
                        getName( ).toLower( ), 
                        PCharacterManager::ext );

    if (!profile.open( )) {
        LogStream::sendError( ) << METHOD << " bad profile for " << getName( ) << endl;
        return false;
    }

    FILE *fp = profile.getFP( );
    
    for (int iNest = 0; iNest < MAX_NEST; iNest++ )
        rgObjNest[iNest] = 0;

    for ( ; ; ) {
        char letter;
        char *word;

        letter = fread_letter( fp );
        if ( letter == '*' ) {
            fread_to_eol( fp );
            continue;
        }

        if ( letter != '#' ) {
            LogStream::sendError( ) 
                << METHOD <<  ": found [" << letter << "], not #" << endl;
            break;
        }

        word = fread_word( fp );
        if ( !str_cmp( word, "PLAYER" ) ) {
            PCharacterManager::load( this );
            fread_char( this, fp );
        }
        else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( this, NULL, fp );
        else if ( !str_cmp( word, "O"      ) ) fread_obj  ( this, NULL, fp );
        else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( this, fp );
        else if ( !str_cmp( word, "MLT"    ) ) fread_mlt  ( this, fp );
        else if ( !str_cmp( word, "End"    ) ) break;
        else
        {
            LogStream::sendError( ) 
                << METHOD << ": bad section " << word << endl;
            break;
        }
    }
    
    if (!get_room_index( start_room )) {
        if (is_immortal( ))
            start_room = ROOM_VNUM_CHAT;
        else
            start_room = ROOM_VNUM_TEMPLE;
    }
                
    /* now restore the character to his/her true condition */
    mod_stat.clear( );
    mod_skills.clear();
    mod_skill_groups.clear();
    mod_level_all = 0;
    mod_level_skills.clear();
    mod_level_groups.clear();
    max_hit         = perm_hit;
    max_mana        = perm_mana;
    max_move        = perm_move;
    armor.clear( );
    armor.fill( 100 );
    hitroll                = 0;
    damroll                = 0;
    saving_throw        = 0;

    size        = getRace( )->getSize( );
    detection   = getRace( )->getDet( );
    affected_by = getRace( )->getAff( );
    imm_flags   = getRace( )->getImm( );
    res_flags   = getRace( )->getRes( );
    vuln_flags  = getRace( )->getVuln( );
    form        = getRace( )->getForm() ;
    parts       = getRace( )->getParts( );
    wearloc.set(getRace()->getWearloc());

    LogStream::sendNotice( ) << getName( ) << " has race " << getRace( )->getName( ) << " and level " << getLevel( ) << endl;

    clear_fenia_skills( this );
    
    // Put player to a room, so that onEquip mobprog that send messages or spellbane won't crash
    char_to_room(this, get_room_index(ROOM_VNUM_LIMBO));
   
    /* Now add back spell effects. */
    for (Affect *af = affected; af != 0; af = af->next)
        affect_modify( this, af, true );
    
    /* Now start adding back the effects from items. Some of the items may add their own affects via onEquip progs,
     * so it's important to execute these two loops in this particular order, to avoid calling affect_modify twice.
     */
    for (Object *obj = carrying; obj != 0; obj = obj->next_content) 
        obj->wear_loc->reset( obj );

    position = (position == POS_FIGHTING ? POS_STANDING: position);
    REMOVE_BIT(act, PLR_NO_EXP|PLR_DIGGED); 
    update_stats(this);
    updateSkills( );
    update_exp( this );

    /* fix renamed skills */
    skill_exchange( this, gsn_sanctuary, gsn_stardust );
    skill_exchange( this, gsn_magic_resistance, gsn_spell_resistance );
    skill_exchange( this, gsn_dispel_magic, gsn_dispel_affects );
    skill_exchange( this, gsn_bat_sworm, gsn_bat_swarm );
    skill_exchange( this, gsn_ground_strike, gsn_critical_strike );
    skill_exchange( this, gsn_control_animal, gsn_dominate );

    // Move player out of the room, to be placed to the start room correctly further down the way.
    char_from_room(this);
    return true;
}

void PCharacter::save( )
{
    static const char * METHOD = " PCharacter::save()";
    ProfilerBlock profiler(getName() + METHOD, 100);

    DLFileWrite tmpfile( dreamland->getBasePath( ), dreamland->getTempFile( ) );

    if (!tmpfile.open( )) {
        LogStream::sendError( ) << METHOD << " bad tmp file" << endl;
        return;
    }

    FILE *fp = tmpfile.getFP( );

    fwrite_char( this, fp );

    fwrite_obj( this, carrying, fp, 0 );

    if (pet)
        fwrite_pet( pet, fp );

    fprintf( fp, "#END\n" );

    if (!tmpfile.close( ))
        return;
    
    DLFile profile( dreamland->getPlayerDir( ), 
                    getName( ).toLower( ), 
                    PCharacterManager::ext );

    if (!tmpfile.rename( profile )) 
        return;


    if (!in_room || IS_SET(in_room->room_flags, ROOM_NOQUIT)) 
        start_room = ROOM_VNUM_TEMPLE;
    else
        start_room = in_room->vnum;

    setLastAccessTime( );
    PCharacterManager::save( this );

    for (Character *ch = char_list; ch; ch = ch->next)
        if (IS_CHARMED(ch) && ch->master == this && ch->is_npc( ))
            if (ch->getNPC( )->behavior)
                ch->getNPC( )->behavior->save( );
}

