/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "class.h"
#include "xmlvariableregistrator.h"
#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "mocregistrator.h"
#include "dlxmlloader.h"
#include "xmlattributeplugin.h"
#include "descriptorstatelistener.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "wiznet.h"
#include "def.h"

#include "templeman.h"
#include "gods_impl.h"
#include "tattoo.h"
#include "defaultreligion.h"
#include "religionattribute.h"

RELIG(none);

class OldGodListener : public DescriptorStateListener {
public:
    typedef ::Pointer<OldGodListener> Pointer;
    
    virtual void run(int oldState, int newState, Descriptor *d)
    {
        PCharacter *ch;

        if (!d->character || !( ch = d->character->getPC( ) ))
            return;

        if (oldState != CON_READ_MOTD || newState != CON_PLAYING) 
            return;

        if (ch->getReligion() == god_none)
            return;

        DefaultReligion *rel = dynamic_cast<DefaultReligion *>(ch->getReligion().getElement());
        if (!rel)
            return;

        if (rel->available(ch))
            return;

        ch->pecho("\r\n{RВнимание!{x Религия {C%N1{x больше не подходит тебе. Обратись к богам за помощью.",
                  rel->getRussianName().c_str());
        wiznet(WIZ_RELIGION, 0, 0, "%^C1 имеет неподходящую религию %N1.", 
                  ch, rel->getRussianName().c_str());
    }
};

extern "C"
{
    SO::PluginList initialize_religion( ) 
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<MobileBehaviorRegistrator<Templeman> >( ppl );
        Plugin::registerPlugin<ObjectBehaviorRegistrator<ReligionTattoo> >( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<ReligionHelp> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeReligion> >( ppl );
        
        Plugin::registerPlugin<MocRegistrator<DefaultReligion> >( ppl );
        Plugin::registerPlugin<MocRegistrator<AtumRaGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ZeusGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<SiebeleGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<AhuramazdaGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ShamashGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<EhrumenGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<VenusGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<SethGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<OdinGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<PhobosGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<TeshubGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<AresGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<HeraGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<DeimosGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ErosGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<EnkiGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<GoktengriGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<BastGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<RavenQueenGod> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ErevanGod> >( ppl );
        Plugin::registerPlugin<OldGodListener>(ppl);
        Plugin::registerPlugin<ReligionLoader>( ppl );

        return ppl;
    }
}

