#include "xmlkillingattribute.h"
#include "fenia/register-impl.h"
#include "idcontainer.h"
#include "lex.h"
#include "regcontainer.h"
#include "wrapperbase.h"
#include "wrappertarget.h"

#include "merc.h"
#include "def.h"

using namespace Scripting;

const DLString XMLKillingAttribute::TYPE = "XMLKillingAttribute";

XMLKillingAttribute::XMLKillingAttribute()
                            : align(&align_table)
{

}

XMLKillingAttribute::~XMLKillingAttribute()
{

}

Scripting::Register XMLKillingAttribute::toRegister() const
{
    Register killedReg = Register::handler<IdContainer>();
    IdContainer *killed = killedReg.toHandler().getDynamicPointer<IdContainer>();

    killed->setField(IdRef("good"), align.at(N_ALIGN_GOOD));
    killed->setField(IdRef("evil"), align.at(N_ALIGN_EVIL));
    killed->setField(IdRef("neutral"), align.at(N_ALIGN_NEUTRAL));
    killed->setField(IdRef("noalign"), align.at(N_ALIGN_NULL));

    Register vnumReg = Register::handler<RegContainer>();
    RegContainer *vnumContainer = vnumReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto pair: vnum)
        vnumContainer->setField(pair.first, pair.second);
        
    killed->setField(IdRef("vnum"), vnumReg);

    return killedReg;
}    
