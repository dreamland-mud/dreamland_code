
#include "affectmanager.h"
#include "affect.h"

AffectManager* AffectManager::thisClass = 0;
AffectManager::ExtractList AffectManager::extractList;

AffectManager::AffectManager() 
{
    checkDuplicate(thisClass);
    thisClass = this;
}

AffectManager::~AffectManager() 
{
    extractList.clear_delete();
    thisClass = 0;
}

void AffectManager::extract(Affect *paf) 
{
    paf->extract();
    extractList.push_back(paf);
}

Affect* AffectManager::getAffect() 
{
    Affect *paf;

    if (!extractList.empty()) {
        paf = *extractList.begin();
        paf->extracted = false;
        extractList.pop_front();
    } else {
        paf = dallocate(Affect);
    }

    return paf;
}
