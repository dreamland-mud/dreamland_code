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
#include "core/object.h"
#include "pcharacter.h"
#include "wearloc_utils.h"
#include "loadsave.h"
#include "wiznet.h"
#include "def.h"

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

        Object *tattoo = get_eq_char(ch, wear_tattoo);
        if (tattoo && tattoo->pIndexData->vnum != rel->tattooVnum) {
            const DLString tattooDefaultName = "татуировк|а|и|е|у|ой|е с изображением ";
            int qp = 200;

            if (tattoo->getRealShortDescr() && !tattooDefaultName.strPrefix(tattoo->getRealShortDescr()))
                qp += 200;

            ch->pecho("\r\nСтарая татуировка больше не может служить тебе и исчезает.", tattoo);
            ch->pecho("На твой счет перечислено %d qp для покупки нового знака твоей религии.", qp);
            wiznet(WIZ_RELIGION, 0, 0, "%^C1 лишается %O2.", ch, tattoo);
            
            ch->addQuestPoints(qp);

            extract_obj(tattoo);

            ch->save();
        }

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

        Plugin::registerPlugin<ObjectBehaviorRegistrator<ReligionTattoo> >( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<ReligionHelp> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeReligion> >( ppl );
        
        Plugin::registerPlugin<MocRegistrator<DefaultReligion> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ErevanGod> >( ppl );
        Plugin::registerPlugin<OldGodListener>(ppl);
        Plugin::registerPlugin<ReligionLoader>( ppl );

        return ppl;
    }
}

