/*
 * Mob reform: loaders for fight/mob_forms.json and fight/mob_tiers.json.
 *
 * Both files are required: every race body goes through the resolver and every
 * prototype takes its numbers from the tier curve. A missing or broken file at
 * first load is an error, and the game runs on empty configs (bare bodies, 1 hp
 * mobs) until it is fixed. A broken edit never replaces a working config.
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
        LogStream::sendError() << "mob_forms: " << error << ", no config loaded: every body resolves empty." << endl;
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
        LogStream::sendError() << "mob_tiers: " << error << ", no config loaded: every mob gets minimal numbers." << endl;
    }

    MobBody::touch();
}
