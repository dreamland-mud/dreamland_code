
#include "pcharacter.h"
#include "social.h"
#include "socialmanager.h"
#include "xmltableloader.h"

#include "socedit.h"
#include "olc.h"
#include "security.h"

#include "merc.h"
#include "websocketrpc.h"
#include "gsn_plugin.h"
#include "commandflags.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

OLC_STATE(OLCStateSocial);


OLCStateSocial::OLCStateSocial() : isChanged(false)
{
}

OLCStateSocial::OLCStateSocial(Social *s) 
    : isChanged(false)
{
    socialName = s->getName();
}

OLCStateSocial::~OLCStateSocial() 
{
}

void OLCStateSocial::commit() 
{
    if (!isChanged)
        return;

    Social *original = getOriginal();
    if (!original)
        return;
    
    original->save();
    if (owner)
        owner->character->pecho("Изменения сохранены на диск.");
}

Social * OLCStateSocial::getOriginal()
{
    Social *s = SocialManager::getThis()->find(socialName);
    if (!s)
        throw Exception("Attached social doesn't exist");

    return s;
}

void OLCStateSocial::statePrompt(Descriptor *d) 
{
    d->send( "Social> " );
}

void OLCStateSocial::changed( PCharacter *ch )
{
    isChanged = true;
}

void OLCStateSocial::show( PCharacter *ch )
{
    Social *s = getOriginal();

    ptc(ch, "Социал:      {C%s\r\n", s->getName().c_str());
    ptc(ch, "По-русски:   {C%s{x %s {D(russian help){x\r\n",
            s->getRussianName().c_str(),
            web_edit_button(ch, "russian", "web").c_str());
    ptc(ch, "Описание:    {C%s{x %s {D(short help){x\r\n",
            s->getShortDesc().c_str(),
            web_edit_button(ch, "short", "web").c_str());
    ptc(ch, "Синонимы:    {C%s{x %s {D(alias help){x\r\n",
            s->aliases.toList().toString().c_str(),
            web_edit_button(ch, "aliases", "").c_str());
    ptc(ch, "Позиция:     {C%s {D(position){x\r\n", 
            s->position.name().c_str());

    ptc(ch, "charNoArgument:   %s  %s\r\n", web_edit_button(ch, "charNoArgument", "web").c_str(), s->msgCharNoArgument.c_str());
    ptc(ch, "othersNoArgument: %s  %s\r\n", web_edit_button(ch, "othersNoArgument", "web").c_str(), s->msgOthersNoArgument.c_str());
    ptc(ch, "charFound:        %s  %s\r\n", web_edit_button(ch, "charFound", "web").c_str(), s->msgCharFound.c_str());
    ptc(ch, "othersFound:      %s  %s\r\n", web_edit_button(ch, "othersFound", "web").c_str(), s->msgOthersFound.c_str());
    ptc(ch, "victimFound:      %s  %s\r\n", web_edit_button(ch, "victimFound", "web").c_str(), s->msgVictimFound.c_str());
    ptc(ch, "charNotFound:     %s  %s\r\n", web_edit_button(ch, "charNotFound", "web").c_str(), s->msgCharNotFound.c_str());
    ptc(ch, "othersAuto:       %s  %s\r\n", web_edit_button(ch, "othersAuto", "web").c_str(), s->msgOthersAuto.c_str());
    ptc(ch, "charFound2:       %s  %s\r\n", web_edit_button(ch, "charFound2", "web").c_str(), s->msgCharFound2.c_str());
    ptc(ch, "othersFound2:     %s  %s\r\n", web_edit_button(ch, "othersFound2", "web").c_str(), s->msgOthersFound2.c_str());
    ptc(ch, "victimFound2:     %s  %s\r\n", web_edit_button(ch, "victimFound2", "web").c_str(), s->msgVictimFound2.c_str());
    ptc(ch, "victimObj:        %s  %s\r\n", web_edit_button(ch, "victimObj", "web").c_str(), s->msgVictimObj.c_str());
    ptc(ch, "charVictimObj:    %s  %s\r\n", web_edit_button(ch, "charVictimObj", "web").c_str(), s->msgCharVictimObj.c_str());
    ptc(ch, "othersVictimObj:  %s  %s\r\n", web_edit_button(ch, "othersVictimObj", "web").c_str(), s->msgOthersVictimObj.c_str());
    ptc(ch, "charObj:          %s  %s\r\n", web_edit_button(ch, "charObj", "web").c_str(), s->msgCharObj.c_str());
    ptc(ch, "othersObj:        %s  %s\r\n", web_edit_button(ch, "othersObj", "web").c_str(), s->msgOthersObj.c_str());

    ptc(ch, "\r\n{WКоманды{x: {hc{ycommands{x, {hc{yshow{x, {hc{ydone{x, {hc{y?{x\r\n");                    
}


SOCEDIT(show, "показать", "показать все поля")
{
    show(ch);
    return false;
}

SOCEDIT(russian, "русское", "русское название социала")
{
    return editor(argument, getOriginal()->rusName, ED_NO_NEWLINE);
}

SOCEDIT(shortdesc, "кратко", "краткое описание социала")
{
    return editor(argument, getOriginal()->shortDesc, ED_NO_NEWLINE);
}

SOCEDIT(position, "позиция", "мин. положение тела для социала (? position_table)")
{
    return flagValueEdit(position_table, getOriginal()->position);
}

SOCEDIT(aliases, "синонимы", "русские и английские синонимы через пробел")
{
    return stringListEdit(getOriginal()->aliases);
}

SOCEDIT(charNoArgument, "", "поле msgCharNoArgument")
{
    return editor(argument, getOriginal()->msgCharNoArgument, ED_NO_NEWLINE);
}

SOCEDIT(othersNoArgument, "", "поле msgOthersNoArgument")
{
    return editor(argument, getOriginal()->msgOthersNoArgument, ED_NO_NEWLINE);
}

SOCEDIT(charFound, "", "поле msgCharFound")
{
    return editor(argument, getOriginal()->msgCharFound, ED_NO_NEWLINE);
}

SOCEDIT(othersFound, "", "поле msgOthersFound")
{
    return editor(argument, getOriginal()->msgOthersFound, ED_NO_NEWLINE);
}

SOCEDIT(victimFound, "", "поле msgVictimFound")
{
    return editor(argument, getOriginal()->msgVictimFound, ED_NO_NEWLINE);
}

SOCEDIT(charNotFound, "", "поле msgCharNotFound")
{
    return editor(argument, getOriginal()->msgCharNotFound, ED_NO_NEWLINE);
}

SOCEDIT(othersAuto, "", "поле msgOthersAuto")
{
    return editor(argument, getOriginal()->msgOthersAuto, ED_NO_NEWLINE);
}

SOCEDIT(charFound2, "", "поле msgCharFound2")
{
    return editor(argument, getOriginal()->msgCharFound2, ED_NO_NEWLINE);
}

SOCEDIT(othersFound2, "", "поле msgOthersFound2")
{
    return editor(argument, getOriginal()->msgOthersFound2, ED_NO_NEWLINE);
}

SOCEDIT(victimFound2, "", "поле msgVictimFound2")
{
    return editor(argument, getOriginal()->msgVictimFound2, ED_NO_NEWLINE);
}

SOCEDIT(victimObj, "", "поле msgVictimObj")
{
    return editor(argument, getOriginal()->msgVictimObj, ED_NO_NEWLINE);
}

SOCEDIT(charVictimObj, "", "поле msgCharVictimObj")
{
    return editor(argument, getOriginal()->msgCharVictimObj, ED_NO_NEWLINE);
}

SOCEDIT(othersVictimObj, "", "поле msgOthersVictimObj")
{
    return editor(argument, getOriginal()->msgOthersVictimObj, ED_NO_NEWLINE);
}

SOCEDIT(charObj, "", "поле msgCharObj")
{
    return editor(argument, getOriginal()->msgCharObj, ED_NO_NEWLINE);
}

SOCEDIT(othersObj, "", "поле msgOthersObj")
{
    return editor(argument, getOriginal()->msgOthersObj, ED_NO_NEWLINE);
}



SOCEDIT(commands, "команды", "показать список встроенных команд")
{
    do_commands(ch);
    return false;
}

SOCEDIT(done, "готово", "выйти из редактора") 
{
    commit();
    detach(ch);
    return false;
}


CMD(socedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, "Online social editor.")
{
    DLString args = argument;
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        stc("Формат:  socedit социал\r\n", ch);
        return;
    }

    DLString arg = DLString(argument).toLower().stripWhiteSpace();    
    Social *social = SocialManager::getThis()->findUnstrict(arg);

    if (!social) {
        ch->pecho("Социал с таким именем не найден.");
        return;
    }
    
    OLCStateSocial::Pointer se(NEW, social);
    se->attach(ch);
    se->show(ch);
}

