#include <algorithm>
#include "playerwrapper.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "xmlkillingattribute.h"
#include "xmlattributestatistic.h"
#include "player_utils.h"
#include "nativeext.h"
#include "regcontainer.h"
#include "reglist.h"
#include "lex.h"
#include "idcontainer.h"
#include "wrappermanager.h"
#include "subr.h"
#include "schedulerwrapper.h"
#include "characterwrapper.h"
#include "structwrappers.h"
#include "wrap_utils.h"
#include "areaquest.h"
#include "areaquestutils.h"
#include "xmlattributeareaquest.h"
#include "lasthost.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

NMI_INIT(PlayerWrapper, "Player, персонаж оффлайн или онлайн");

PlayerWrapper::PlayerWrapper(const DLString &n)
                  : name( n )
{
}

Register PlayerWrapper::wrap(const DLString &name)
{
    PCMemoryInterface *player = PCharacterManager::find(name);
    if (!player)
        return Register();
        
    return Register::handler<PlayerWrapper>(player->getName());
}

PCMemoryInterface * PlayerWrapper::getTarget() const
{
    PCMemoryInterface *player = PCharacterManager::find(name);
    if (!player)
        throw Scripting::Exception(name + ": player no longer exists");

    return player;
}

void PlayerWrapper::save() const
{
    PCMemoryInterface *player = getTarget();
    PCharacterManager::saveMemory(player);
}

NMI_INVOKE(PlayerWrapper, api, "(): печатает этот api")
{
    ostringstream buf;
    
    Scripting::traitsAPI<PlayerWrapper>(buf);
    return Scripting::Register(buf.str());
}

NMI_GET(PlayerWrapper, name, "английское имя") 
{
    return getTarget()->getName();
}

NMI_GET(PlayerWrapper, russianName, "русское имя с падежами") 
{
    return getTarget()->getRussianName().getFullForm();
}

NMI_SET(PlayerWrapper, russianName, "русское имя с падежами") 
{
    getTarget()->setRussianName(arg2string(arg));
    save();
}

NMI_GET(PlayerWrapper, clan, "клан (структура .Clan)")
{
    return ClanWrapper::wrap(getTarget()->getClan()->getName());
}

NMI_SET(PlayerWrapper, clan, "клан (структура .Clan)")
{
    getTarget()->setClan(wrapper_cast<ClanWrapper>(arg)->name);
    save();
}

NMI_GET(PlayerWrapper, petition, "петиция в клан (структура .Clan)")
{
    return ClanWrapper::wrap(getTarget()->getPetition()->getName());
}

NMI_SET(PlayerWrapper, petition, "петиция в клан (структура .Clan)")
{
    getTarget()->setPetition(wrapper_cast<ClanWrapper>(arg)->name);
    save();
}

NMI_GET(PlayerWrapper, clanLevel, "клановый уровень, число от 0 до 8")
{
    return getTarget()->getClanLevel();
}

NMI_SET(PlayerWrapper, clanLevel, "клановый уровень, число от 0 до 8")
{
    getTarget()->setClanLevel(arg2number(arg, 0, 8));
    save();
}

NMI_GET(PlayerWrapper, profession, "класс (структура .Profession)")
{
    return Register::handler<ProfessionWrapper>(getTarget()->getProfession()->getName());
}

NMI_SET(PlayerWrapper, profession, "класс (структура .Profession)")
{
    getTarget()->setProfession(wrapper_cast<ProfessionWrapper>(arg)->name);
    save();
}

NMI_GET(PlayerWrapper, religion, "религия (структура .Religion)")
{
    return Register::handler<ReligionWrapper>(getTarget()->getReligion()->getName());
}

NMI_SET(PlayerWrapper, religion, "религия (структура .Religion)")
{
    getTarget()->setReligion(wrapper_cast<ReligionWrapper>(arg)->name);
    save();
}

NMI_GET(PlayerWrapper, hometown, "родной город (структура .Hometown)")
{
    return Register::handler<HometownWrapper>(getTarget()->getHometown()->getName());
}

NMI_SET(PlayerWrapper, hometown, "родной город (структура .Hometown)")
{
    getTarget()->setHometown(wrapper_cast<HometownWrapper>(arg)->name);
    save();
}

NMI_GET(PlayerWrapper, race, "раса (структура .Race)")
{
    return RaceWrapper::wrap(getTarget()->getRace()->getName());
}

NMI_SET(PlayerWrapper, race, "раса (структура .Race)")
{
    getTarget()->setRace(wrapper_cast<RaceWrapper>(arg)->name);
    save();
}

NMI_GET(PlayerWrapper, level, "настоящий уровень")
{
    return getTarget()->getLevel();
}

NMI_SET(PlayerWrapper, level, "настоящий уровень")
{
    getTarget()->setLevel(arg2number(arg, 1, MAX_LEVEL));
    save();
}

NMI_GET(PlayerWrapper, remorts, "структура с ремортами")
{
    PCMemoryInterface *player = getTarget();
    Register remortsReg = Register::handler<IdContainer>();
    IdContainer *remorts = remortsReg.toHandler().getDynamicPointer<IdContainer>();
    RegList::Pointer lifes(NEW);

    for (auto r: player->getRemorts()) {
        Register lifeReg = Register::handler<IdContainer>();
        IdContainer *life = lifeReg.toHandler().getDynamicPointer<IdContainer>();

        life->setField(IdRef("race"), r.race);
        life->setField(IdRef("class"), r.classCh);
        life->setField(IdRef("time"), (int)r.time);
        life->setField(IdRef("bonus"), r.bonus.getValue());

        lifes->push_back(lifeReg);
    }

    remorts->setField(IdRef("lifes"), ::wrap(lifes));
    remorts->setField(IdRef("pretitle"), player->getRemorts().pretitle.getValue());
    remorts->setField(IdRef("hp"), player->getRemorts().hp.getValue());
    remorts->setField(IdRef("mana"), player->getRemorts().mana.getValue());
    remorts->setField(IdRef("level"), player->getRemorts().level.getValue());
    remorts->setField(IdRef("owners"), player->getRemorts().owners.getValue());

    for (int i = 0; i < stat_table.size; i++) {
        remorts->setField(IdRef(stat_table.fields[i].name), player->getRemorts().stats[i].getValue());
    }

    return remortsReg;
}

NMI_GET(PlayerWrapper, sex, "пол (таблица .tables.sex_table)")
{
    return getTarget()->getSex();
}

NMI_SET(PlayerWrapper, sex, "пол (таблица .tables.sex_table)")
{
    getTarget()->setSex(arg2flag(arg, sex_table));
    save();
}

NMI_GET(PlayerWrapper, alignment, "натура, от -1000 до 1000")
{
    return getTarget()->getAlignment();
}

NMI_SET(PlayerWrapper, alignment, "натура, от -1000 до 1000")
{
    getTarget()->setAlignment(arg2number(arg, ALIGN_EVIL, ALIGN_GOOD));
    save();
}

NMI_GET(PlayerWrapper, ethos, "этос")
{
    return getTarget()->getEthos();
}

NMI_SET(PlayerWrapper, ethos, "этос")
{
    getTarget()->setEthos(arg2flag(arg, ethos_table));
    save();
}

NMI_GET(PlayerWrapper, description, "описание персонажа")
{
    return getTarget()->getDescription(LANG_DEFAULT);
}

NMI_SET(PlayerWrapper, description, "описание персонажа")
{
    getTarget()->setDescription(arg2string(arg), LANG_DEFAULT);
    save();
}

NMI_GET(PlayerWrapper, title, "титул персонажа")
{
    return getTarget()->getTitle();
}

NMI_SET(PlayerWrapper, title, "титул персонажа")
{
    getTarget()->setTitle(arg2string(arg));
    save();
}

NMI_GET(PlayerWrapper, pretitle, "предтитул персонажа")
{
    return getTarget()->getPretitle();
}

NMI_SET(PlayerWrapper, pretitle, "предтитул персонажа")
{
    getTarget()->setPretitle(arg2string(arg));
    save();
}

NMI_GET(PlayerWrapper, russianPretitle, "русский предтитул персонажа")
{
    return getTarget()->getRussianPretitle();
}

NMI_SET(PlayerWrapper, russianPretitle, "русский предтитул персонажа")
{
    getTarget()->setRussianPretitle(arg2string(arg));
    save();
}

NMI_GET(PlayerWrapper, parsedTitle, "титул персонажа как мы его видим")
{
    return Player::title(getTarget());
}

NMI_GET(PlayerWrapper, questpoints, "квестовые очки")
{
    return getTarget()->getQuestPoints();
}

NMI_SET(PlayerWrapper, questpoints, "квестовые очки")
{
    getTarget()->setQuestPoints(arg2number(arg));
    save();
}

NMI_GET(PlayerWrapper, trust, "уровень привилегий")
{
    return getTarget()->get_trust();
}

NMI_GET(PlayerWrapper, lastAccessHost, "последний IP адрес")
{
    return getTarget()->getLastAccessHost();
}

NMI_GET(PlayerWrapper, lastAccessTime, "время (в секундах) последнего захода в мир")
{
    return (int)getTarget()->getLastAccessTime().getTime();
}

NMI_GET(PlayerWrapper, online, "true если персонаж в мире")
{
    return getTarget()->isOnline();
}

NMI_INVOKE(PlayerWrapper, hasAttribute, "(attr): true если установлен аттрибут с именем attr")
{
    return getTarget()->getAttributes().isAvailable(args2string(args));
}

NMI_INVOKE(PlayerWrapper, eraseAttribute, "(attr): удаляет аттрибут с именем attr")
{
    getTarget()->getAttributes().eraseAttribute(args2string(args));
    save();
    return Register();
}

NMI_INVOKE(PlayerWrapper, attribute, "(name): вернуть аттрибут с данным именем, в виде строки или структуры, либо null")
{
    DLString name = args2string(args);
    PCMemoryInterface *player = getTarget();

    if (!player->getAttributes().isAvailable(name))
        return Register();

    XMLAttribute::Pointer attr = player->getAttributes().find(name)->second;
    return attr->toRegister();
}

NMI_GET(PlayerWrapper, ips, "массив (.Array), ключ - IP адес, значение - количество заходов")
{
    PCMemoryInterface *player = getTarget();

    Register ipsReg = Register::handler<RegContainer>();
    RegContainer *ips = ipsReg.toHandler().getDynamicPointer<RegContainer>();

    auto lastHostAttr = player->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");  
    if (!lastHostAttr)
        return ipsReg;
        
    auto hosts = lastHostAttr->getHosts();  

    for (auto h: hosts)
        ips->setField(h.first, h.second.getValue());

    return ipsReg;
}


NMI_GET(PlayerWrapper, alts, "массив (.Array), ключ - IP, значение - Array из имен альтов и количества их заходов с этого IP")
{
    PCMemoryInterface *player = getTarget();
    Register altsReg = Register::handler<RegContainer>();
    RegContainer *alts = altsReg.toHandler().getDynamicPointer<RegContainer>();

    auto myLastHostAttr = player->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");  
    if (!myLastHostAttr)
        return altsReg;
    auto myHosts = myLastHostAttr->getHosts();  
    
    std::map<DLString, std::map<DLString, int> > loginsByHostByPlayer;

    // Collect stats about matching IPs for all players
    const PCharacterMemoryList &players = PCharacterManager::getPCM();    
    for (auto p: players) {
        PCMemoryInterface *alt = p.second;
        DLString altName = p.first;

        if (altName == player->getName())
            continue;

        auto altLastHostAttr = alt->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");  
        if (!altLastHostAttr)
            continue;

        // For each player, count how many times they logged in from the same IP addresses as the target player
        for (auto h: altLastHostAttr->getHosts()) {
            DLString altHost = h.first;
            int altLogins = h.second.getValue();

            if (myHosts.find(altHost) != myHosts.end())
                loginsByHostByPlayer[altHost][altName] += altLogins;
        }
    }

    // Populate Fenia array
    for (auto &lhp: loginsByHostByPlayer) {
        Register loginsByPlayerReg = Register::handler<RegContainer>();
        RegContainer *loginsByPlayerContainer = loginsByPlayerReg.toHandler().getDynamicPointer<RegContainer>();
        
        DLString altHost = lhp.first;
        auto &loginsByPlayer = lhp.second;

        for (auto &lp: loginsByPlayer) {
            DLString altName = lp.first;
            int altLogins = lp.second;
            loginsByPlayerContainer->setField(altName, altLogins);
        }

        alts->setField(altHost, loginsByPlayerReg);
    }

    return altsReg;
}


NMI_GET(PlayerWrapper, start_room, "vnum комнаты, в которой зайдут в мир")
{
    return getTarget()->getStartRoom();
}

NMI_SET(PlayerWrapper, start_room, "vnum комнаты, в которой зайдут в мир")
{
    getTarget()->setStartRoom(arg2number(arg, 1, 1000000));
    save();
}

NMI_GET(PlayerWrapper, killed, "статистика убийств мобов")
{
    PCMemoryInterface *player = getTarget();
    auto killingAttr = player->getAttributes().getAttr<XMLKillingAttribute>("killed");
    return killingAttr->toRegister();
}

NMI_GET(PlayerWrapper, loyalty, "лояльность по отношению к закону")
{
    return getTarget()->getLoyalty();
}

NMI_SET(PlayerWrapper, loyalty, "лояльность по отношению к закону")
{
    getTarget()->setLoyalty(arg2number(arg));
    save();
}

NMI_GET(PlayerWrapper, gquest, "статистика побед в глобальных квестах")
{
    PCMemoryInterface *player = getTarget();
    auto statAttr = player->getAttributes().findAttr<XMLAttributeStatistic>("gquest");
    if (!statAttr)
        return Register();
    return statAttr->toRegister(player, "gquest");
}

NMI_GET(PlayerWrapper, quest, "статистика побед в авто квестах")
{
    PCMemoryInterface *player = getTarget();
    auto statAttr = player->getAttributes().findAttr<XMLAttributeStatistic>("questdata");
    if (!statAttr)
        return Register();
    return statAttr->toRegister(player, "questdata");
}

NMI_GET(PlayerWrapper, aquest, "статистика побед в арийных квестах, без учета квестов онбординга")
{
    PCMemoryInterface *player = getTarget();
    Register victoryReg = Register::handler<IdContainer>();
    IdContainer *victory = victoryReg.toHandler().getDynamicPointer<IdContainer>();
    int victoriesThisLife = 0;
    int victoriesTotal = 0;
    int availableThisLife = 0;
    int availableTotal = 0;

    for (auto &q: areaQuests) {
        AreaQuest *quest = q.second;

        if (quest->flags.isSet(AQUEST_ONBOARDING))
            continue;

        availableTotal++;

        if (aquest_can_participate_ever(player, quest))
            availableThisLife++;

        AreaQuestData &qdata = aquest_data(player, quest->vnum.toString());

        if (qdata.total > 0)
            victoriesTotal++;

        if (qdata.thisLife > 0)
            availableThisLife++;
    }

    victory->setField(IdRef("victoriesTotal"), victoriesTotal);
    victory->setField(IdRef("victoriesThisLife"), victoriesThisLife);
    victory->setField(IdRef("availableTotal"), availableTotal);
    victory->setField(IdRef("availableThisLife"), availableThisLife);

    return victoryReg;
}

NMI_GET(PlayerWrapper, attributes, "Array всех аттрибутов, ключ - имя аттрибута, значение - Map с полями аттрибута либо пустая строка")
{
    PCMemoryInterface *player = getTarget();
    return player->getAttributes().toRegister();
}

NMI_GET(PlayerWrapper, newbie, "true если нет ремортов, <50 квестов")
{
    PCMemoryInterface *player = getTarget();
    return Player::isNewbie(player);
}

NMI_GET(PlayerWrapper, player, "экземпляр персонажа, если online, иначе null")
{
    PCMemoryInterface *pci = getTarget();
    PCharacter *player = pci->getPlayer();
    if (player)
        return ::wrap(player);
    else
        return Register();
}
