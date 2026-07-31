
#include "pcharacter.h"
#include "social.h"
#include "socialmanager.h"
#include "xmltableloader.h"

#include "socedit.h"
#include "olc.h"
#include "security.h"

#include "merc.h"
#include "websocketrpc.h"

#include "commandflags.h"
#include "act.h"

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

lang_t OLCStateSocial::getLang() const
{
    DLString value = editLang.getValue();

    if (value == "en")
        return EN;
    if (value == "ua")
        return UA;

    return RU;
}

void OLCStateSocial::show( PCharacter *ch )
{
    Social *s = getOriginal();
    lang_t lang = getLang();

    ptc(ch, "Социал:      {C%s\r\n", s->getName().c_str());
    ptc(ch, "По-русски:   {C%s{x %s {D(russian help){x\r\n",
            s->getRussianName().c_str(),
            web_edit_button(ch, "russian", "web").c_str());
    ptc(ch, "Язык полей:  {C%s{x {D(language){x\r\n", lang2attr(lang).c_str());
    ptc(ch, "Описание:    {C%s{x %s {D(short help){x\r\n",
            s->shortDesc.get(lang).c_str(),
            web_edit_button(ch, "short", "web").c_str());
    ptc(ch, "Синонимы:    {C%s{x %s {D(alias help){x\r\n",
            s->aliases.toList().toString().c_str(),
            web_edit_button(ch, "aliases", "").c_str());
    ptc(ch, "Позиция:     {C%s {D(position){x\r\n", 
            s->position.name().c_str());

    ptc(ch, "charNoArgument:   %s  %s\r\n", web_edit_button(ch, "charNoArgument", "web").c_str(), s->msgCharNoArgument.get(lang).c_str());
    ptc(ch, "othersNoArgument: %s  %s\r\n", web_edit_button(ch, "othersNoArgument", "web").c_str(), s->msgOthersNoArgument.get(lang).c_str());
    ptc(ch, "charFound1:        %s  %s\r\n", web_edit_button(ch, "charFound1", "web").c_str(), s->msgCharFound.get(lang).c_str());
    ptc(ch, "othersFound1:      %s  %s\r\n", web_edit_button(ch, "othersFound1", "web").c_str(), s->msgOthersFound.get(lang).c_str());
    ptc(ch, "victimFound1:      %s  %s\r\n", web_edit_button(ch, "victimFound1", "web").c_str(), s->msgVictimFound.get(lang).c_str());
    ptc(ch, "charNotFound:     %s  %s\r\n", web_edit_button(ch, "charNotFound", "web").c_str(), s->msgCharNotFound.get(lang).c_str());
    ptc(ch, "othersAuto:       %s  %s\r\n", web_edit_button(ch, "othersAuto", "web").c_str(), s->msgOthersAuto.get(lang).c_str());
    ptc(ch, "charFound2:       %s  %s\r\n", web_edit_button(ch, "charFound2", "web").c_str(), s->msgCharFound2.get(lang).c_str());
    ptc(ch, "othersFound2:     %s  %s\r\n", web_edit_button(ch, "othersFound2", "web").c_str(), s->msgOthersFound2.get(lang).c_str());
    ptc(ch, "victimFound2:     %s  %s\r\n", web_edit_button(ch, "victimFound2", "web").c_str(), s->msgVictimFound2.get(lang).c_str());
    ptc(ch, "victimObj:        %s  %s\r\n", web_edit_button(ch, "victimObj", "web").c_str(), s->msgVictimObj.get(lang).c_str());
    ptc(ch, "charVictimObj:    %s  %s\r\n", web_edit_button(ch, "charVictimObj", "web").c_str(), s->msgCharVictimObj.get(lang).c_str());
    ptc(ch, "othersVictimObj:  %s  %s\r\n", web_edit_button(ch, "othersVictimObj", "web").c_str(), s->msgOthersVictimObj.get(lang).c_str());
    ptc(ch, "charObj:          %s  %s\r\n", web_edit_button(ch, "charObj", "web").c_str(), s->msgCharObj.get(lang).c_str());
    ptc(ch, "othersObj:        %s  %s\r\n", web_edit_button(ch, "othersObj", "web").c_str(), s->msgOthersObj.get(lang).c_str());

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

SOCEDIT(language, "язык", "какой язык правят команды полей: en, ru или ua")
{
    DLString arg = DLString(argument).getOneArgument().toLower();

    if (arg != "en" && arg != "ru" && arg != "ua") {
        ptc(ch, "Язык полей сейчас {C%s{x. Укажи en, ru или ua.\r\n",
                lang2attr(getLang()).c_str());
        return false;
    }

    editLang = arg;
    ptc(ch, "Команды полей теперь правят {C%s{x.\r\n", arg.c_str());
    return true;
}

SOCEDIT(shortdesc, "кратко", "краткое описание социала")
{
    return editor(argument, getOriginal()->shortDesc[getLang()], ED_NO_NEWLINE);
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
    return editor(argument, getOriginal()->msgCharNoArgument[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(othersNoArgument, "", "поле msgOthersNoArgument")
{
    return editor(argument, getOriginal()->msgOthersNoArgument[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(charFound1, "", "поле msgCharFound")
{
    return editor(argument, getOriginal()->msgCharFound[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(othersFound1, "", "поле msgOthersFound")
{
    return editor(argument, getOriginal()->msgOthersFound[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(victimFound1, "", "поле msgVictimFound")
{
    return editor(argument, getOriginal()->msgVictimFound[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(charNotFound, "", "поле msgCharNotFound")
{
    return editor(argument, getOriginal()->msgCharNotFound[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(othersAuto, "", "поле msgOthersAuto")
{
    return editor(argument, getOriginal()->msgOthersAuto[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(charFound2, "", "поле msgCharFound2")
{
    return editor(argument, getOriginal()->msgCharFound2[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(othersFound2, "", "поле msgOthersFound2")
{
    return editor(argument, getOriginal()->msgOthersFound2[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(victimFound2, "", "поле msgVictimFound2")
{
    return editor(argument, getOriginal()->msgVictimFound2[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(victimObj, "", "поле msgVictimObj")
{
    return editor(argument, getOriginal()->msgVictimObj[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(charVictimObj, "", "поле msgCharVictimObj")
{
    return editor(argument, getOriginal()->msgCharVictimObj[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(othersVictimObj, "", "поле msgOthersVictimObj")
{
    return editor(argument, getOriginal()->msgOthersVictimObj[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(charObj, "", "поле msgCharObj")
{
    return editor(argument, getOriginal()->msgCharObj[getLang()], ED_NO_NEWLINE);
}

SOCEDIT(othersObj, "", "поле msgOthersObj")
{
    return editor(argument, getOriginal()->msgOthersObj[getLang()], ED_NO_NEWLINE);
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

