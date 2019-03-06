/* $Id: forget.cpp,v 1.1.2.5.6.8 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 */

#include "forget.h"

#include "commandtemplate.h"
#include "schedulerqueue.h"
#include "dlscheduler.h"
#include "skill.h"
#include "skillmanager.h"
#include "pcharacter.h"
#include "act.h"
#include "loadsave.h"
#include "mercdb.h"
#include "merc.h"

PROF(universal);

CMDRUN( forget )
{
    int sn;
    Skill *skill;
    DLString argument = constArguments;
    PCharacter *pch = ch->getPC( );
    SkillManager *manager = SkillManager::getThis( );
    
    if (!pch) {
        ch->send_to( "А у тебя в мозгах есть чего забывать??\r\n" );
        return;
    }
    
    if (ch->getProfession( ) != prof_universal) {
        ch->send_to( "Ты ничего не можешь забыть.\r\n" );
        return;
    }
    
    if (argument.empty( )) {
        ch->println( "Что именно ты хочешь забыть?" );
        return;
    }
    
    if (arg_is_all( argument )) {
        ostringstream buf;

        for (sn = 0; sn < manager->size( ); sn++) {
            bool &forgetting = pch->getSkillData( sn ).forgetting;
            skill = manager->find( sn );

            if (!skill->canForget( pch ))
                continue;
            
            if (forgetting)
                continue;
                
            forgetting = true;

            if (!buf.str( ).empty( ))
                buf << ", ";

            buf << skill->getNameFor( ch );
        }

        if (buf.str( ).empty( ))
            ch->send_to( "У тебя отличная память: ты ничего не можешь забыть\r\n" );
        else 
            ch->printf( "Теперь ты будешь забывать '%s'.\r\n", buf.str( ).c_str( ) );
        
        return;
    }

    sn = manager->unstrictLookup( argument, ch );
    skill = manager->find( sn );
    
    if (!skill) {
        ch->send_to( "Среди доступных тебе умений нет ничего похожего.\r\n" );
        return;
    }

    if (!skill->canForget( pch )) {
        ch->printf( "Ты не можешь забыть исскуство '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
        return;
    }

    bool &forgetting = pch->getSkillData( sn ).forgetting;

    if (forgetting) {
        ch->printf( "Ты и так только и делаешь, что забываешь '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
        return;
    }

    forgetting = true;
    ch->printf( "Теперь ты будешь забывать '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
}

CMDRUN( remember )
{ 
    int sn;
    Skill *skill;
    DLString argument = constArguments;

    if (ch->is_npc( )) {
        ch->send_to("Тебе тяжело вспомнить даже папу и маму..\n\r");
        return;
    }

    if (argument.empty( )) {
        ch->println( "Что именно ты хочешь вспомнить?" );
        return;
    }
    
    if (arg_is_all( argument )) {
        DLString buf;

        for (sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
            PCSkillData &data = ch->getPC( )->getSkillData( sn );
            skill = SkillManager::getThis( )->find( sn );
            
            if (!data.forgetting || data.learned <= 1) 
                continue;
            
            data.forgetting = false;

            if (!buf.empty( ))
                buf += ", ";

            buf += skill->getNameFor( ch );
        }

        if (buf.empty( ))
            ch->send_to( "Тебе нечего вспоминать.\r\n" );
        else 
            ch->printf( "Ты вспоминаешь '%s'.\r\n", buf.c_str( ) );

        return;
    }

    sn = SkillManager::getThis( )->unstrictLookup( argument, ch );
    skill = SkillManager::getThis( )->find( sn );
    
    if (!skill) {
        ch->send_to( "Разве можно вспомнить то, чего не знаешь?\n\r");
        return;
    }
    
    PCSkillData &data = ch->getPC( )->getSkillData( sn );

    if (!data.forgetting) {
        act_p( "Ты и не пытал$gось|ся|ась забыть $t.", ch, skill->getNameFor( ch ).c_str( ), 0, TO_CHAR, POS_DEAD );
        return;
    }
    
    if (data.learned <= 1) {
        ch->send_to( "Иди сначала попрактикуйся.\n\r");
        return;
    }

    data.forgetting = false;

    ch->printf( "Ты вспоминаешь '%s'.\n\r", skill->getNameFor( ch ).c_str( ) );
}

void SkillTimerUpdate::run( PCharacter *ch ) 
{
    int sn_max = 0, timer_max = 0;
    int sn_max_forget = 0, timer_max_forget = 0;

    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill *skill = SkillManager::getThis( )->find( sn );
        PCSkillData &data = ch->getSkillData( sn );
        
        if (!skill->canForget( ch ) || ch->getProfession( ) != prof_universal) {
            if (data.forgetting) {
                ch->printf( "Ты прекращаешь забывать '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
                data.forgetting = false;
            }

            continue;
        }
        
        if (data.timer.getValue( ) < 10000) {
            if (data.forgetting)
                data.timer += 10;
            else
                data.timer++;
        }
        
        if (timer_max < data.timer.getValue( )) {
            sn_max = sn;
            timer_max = data.timer;
        }

        if (timer_max_forget < data.timer.getValue( ) && data.forgetting) {
            sn_max_forget = sn;
            timer_max_forget = data.timer;
        }

        if (data.forgetting && number_percent( ) < 10)
            forget( ch, sn );
    }

    if (ch->skill_points( ) > ch->max_skill_points) {
        if (sn_max_forget)
            forget( ch, sn_max_forget );
        else
            forget( ch, sn_max );
    }
}

void SkillTimerUpdate::after( ) 
{
    DLScheduler::getThis( )->putTaskInSecond( 60, SkillTimerUpdate::Pointer( this ) );
}

void SkillTimerUpdate::forget( PCharacter *ch, int sn )
{
    PCSkillData &data = ch->getSkillData( sn );
    
    data.learned -= 5;
    
    if (data.forgetting)
        data.timer--;
    else
        data.timer -= 5;
        
    data.learned = std::max(1, data.learned.getValue( ));

    ch->printf( "{gТы забываешь искусство '%s'.{x\n\r", 
                SkillManager::getThis( )->find( sn )->getNameFor( ch ).c_str( ) ); 
}
