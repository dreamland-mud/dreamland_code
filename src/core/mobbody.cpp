/*
 * Mob reform: engine side of the body model, see mobbody.h.
 */
#include "mobbody.h"

namespace MobBody {

static Body::Config formsConfig;
static MobTiers::Config tiersConfig;
static unsigned long configGeneration = 1;

Body::Config &forms()
{
    return formsConfig;
}

MobTiers::Config &tiers()
{
    return tiersConfig;
}

unsigned long generation()
{
    return configGeneration;
}

void touch()
{
    configGeneration++;
}

}
