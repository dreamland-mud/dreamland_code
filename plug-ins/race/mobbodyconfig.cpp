/*
 * Mob reform: loaders for fight/mob_forms.json and fight/mob_tiers.json.
 *
 * Both files are optional. While one is absent (master's world has neither
 * until the data phase) the engine logs it and keeps today's behaviour: races
 * without <forms> and every race while mob_forms.json is missing keep their
 * authored bodies, and prototypes keep their authored numbers while
 * mob_tiers.json is missing. A broken edit never replaces a working config.
 *
 * Fenia reads both through .config("fight/mob_forms") / .config("fight/mob_tiers").
 */
#include <jsoncpp/json/json.h>
#include "configurable.h"
#include "logstream.h"
#include "mobbody.h"

CONFIGURABLE_LOADED(fight, mob_forms)
{
    Body::Config cfg;
    std::string error;

    if (cfg.fromJson(value, error)) {
        MobBody::forms() = cfg;
        LogStream::sendNotice() << "mob_forms: " << cfg.forms.size() << " forms, "
                                << cfg.parts.size() << " parts loaded." << endl;
    } else if (MobBody::forms().loaded) {
        LogStream::sendError() << "mob_forms: " << error << ", keeping the previous config." << endl;
    } else {
        LogStream::sendNotice() << "mob_forms: " << error << ", races keep their authored bodies." << endl;
    }

    MobBody::touch();
}

CONFIGURABLE_LOADED(fight, mob_tiers)
{
    MobTiers::Config cfg;
    std::string error;

    if (cfg.fromJson(value, error)) {
        MobBody::tiers() = cfg;
        LogStream::sendNotice() << "mob_tiers: loaded, default tier "
                                << cfg.name(cfg.defaultTier) << "." << endl;
    } else if (MobBody::tiers().loaded) {
        LogStream::sendError() << "mob_tiers: " << error << ", keeping the previous config." << endl;
    } else {
        LogStream::sendNotice() << "mob_tiers: " << error << ", mobs keep their authored numbers." << endl;
    }

    MobBody::touch();
}
