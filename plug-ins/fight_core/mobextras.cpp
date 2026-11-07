/*
 * Random extra defenses of champion, boss and legend prototypes, see mobextras.h.
 *
 * They live on the index (instances copy imm/res/vuln at creation), so medit,
 * stat, lore tools and the loot code see what the fight uses. The roll is
 * here and not in src/core because it needs the mob's attack damage classes
 * (attack_table, immune_bit), which only fight_core has.
 */
#include <vector>

#include "mobextras.h"
#include "attacks.h"
#include "immunity.h"
#include "mobilefactory.h"
#include "mobbody.h"
#include "race.h"
#include "flagtable.h"
#include "merc.h"
#include "def.h"

void mob_index_rebuild(mob_index_data *pMobIndex)
{
    pMobIndex->resolveBody();
    pMobIndex->deriveNumbers();
    mob_index_random_extras(pMobIndex);
}

/* splitmix64: a local stream, so the roll never touches the game's RNG state
 * and comes out the same on every boot of the month. */
static unsigned long long extras_next(unsigned long long &state)
{
    unsigned long long z = (state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

void mob_index_random_extras(mob_index_data *pMobIndex)
{
    const MobTiers::Config &tc = MobBody::tiers();
    const MobTiers::Tier &row = tc.get(pMobIndex->tier);

    for (int s = 0; s < MOBSET_MAX; s++)
        pMobIndex->randomExtra[s] = 0;

    if (row.randomRes <= 0 && row.randomVuln <= 0)
        return;

    // A unique race and a reviewed defense set are what the builder decided.
    int reviewedDefenses = (1 << Body::BS_IMM) | (1 << Body::BS_RES) | (1 << Body::BS_VULN);
    if (pMobIndex->reviewed & reviewedDefenses)
        return;
    if (MobBody::raceFormless(raceManager->find(pMobIndex->race)))
        return;

    bitstring_t taken = (unsigned int)pMobIndex->imm_flags
                        | (unsigned int)pMobIndex->res_flags
                        | (unsigned int)pMobIndex->vuln_flags;

    // Its own attacks stay unresisted: a fire drake does not shrug off fire.
    std::vector<int> attacks = mob_index_attacks(pMobIndex);
    attacks.push_back(pMobIndex->dam_type);
    for (int a: attacks)
        taken |= immune_bit(attack_damage(a));

    std::vector<bitstring_t> pool;
    for (auto &name: tc.randomPool) {
        bitstring_t bit = MobBody::bits(&::imm_flags, Body::NameSet { name });
        if (bit && !(taken & bit)) {
            taken |= bit;
            pool.push_back(bit);
        }
    }

    unsigned long long state = (unsigned long long)pMobIndex->vnum * 0x100000001B3ULL
                               ^ ((unsigned long long)MobBody::monthSalt() << 32);
    bitstring_t imm = 0, res = 0, vuln = 0;

    for (int i = 0; i < row.randomRes && !pool.empty(); i++) {
        size_t pick = extras_next(state) % pool.size();
        bitstring_t bit = pool[pick];
        pool.erase(pool.begin() + pick);
        bool immune = (int)(extras_next(state) % 100) < row.randomImmPct;
        if (immune && !imm)
            imm = bit;
        else
            res |= bit;
    }

    for (int i = 0; i < row.randomVuln && !pool.empty(); i++) {
        size_t pick = extras_next(state) % pool.size();
        vuln |= pool[pick];
        pool.erase(pool.begin() + pick);
    }

    pMobIndex->randomExtra[MOBSET_IMM] = imm;
    pMobIndex->randomExtra[MOBSET_RES] = res;
    pMobIndex->randomExtra[MOBSET_VULN] = vuln;

    // Into the snapshot too, so asave never writes them out as authored adds.
    for (int s: { MOBSET_IMM, MOBSET_RES, MOBSET_VULN }) {
        pMobIndex->bodyBits(s) |= (int)pMobIndex->randomExtra[s];
        pMobIndex->bodySnapshot[s] = (unsigned int)pMobIndex->bodyBits(s);
    }
}
