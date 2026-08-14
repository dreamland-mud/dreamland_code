/* $Id$
 *
 * ruffina, 2004
 */
#include "scenarios_impl.h"
#include "locatequest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "msgformatter.h"
#include "interp.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

/*-----------------------------------------------------------------------------
 * LocateMousesScenario
 *----------------------------------------------------------------------------*/
void LocateMousesScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf ) const
{
    lang_t lang = viewerLang( hero );

    buf << fmt( hero, _("%1$s жалуется на банду грызунов, которые растащили %2$s из ее кладовки."),
                russian_case( quest->customerName.getForLang( lang ), '1' ).c_str( ),
                russian_case( quest->itemMltName.getForLang( lang ), '4' ).c_str( ) ) << endl
        << fmt( hero, _("Пропало довольно много, но она будет благодарна тебе, если ты принесешь ей хотя бы {Y%1$d{x штук%1$Iу|и|."),
                quest->total.getValue( ) ) << endl;
}

void LocateMousesScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    oldact(_("$c1, всплеснув руками, бросается тебе навстречу."), ch, 0, hero, TO_VICT);    
    oldact(_("$c1, всплеснув руками, бросается навстречу $C3."), ch, 0, hero, TO_NOTVICT);    
    tell_raw( hero, ch, _("Подлые мыши, спасу от них нет никакого. Чем я их только не травила!"));
    tell_act( hero, ch, _("Растащили из кладовки {W$n4{G. Всех их уже, конечно, не отыскать."), 
              quest->itemMltName.getForLang( viewerLang( hero ) ).c_str( ) );
    tell_raw( hero, ch, _("Но попробуй собери хотя бы {W%d{G штук. Жду с нетерпением."),
              quest->total.getValue( ) );
}

bool LocateMousesScenario::applicable( PCharacter *ch ) const
{
    return !IS_EVIL(ch);
}

/*-----------------------------------------------------------------------------
 * LocateSecretaryScenario
 *----------------------------------------------------------------------------*/
void LocateSecretaryScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf ) const
{
    lang_t lang = viewerLang( hero );

    buf << fmt( hero, _("%1$s просит тебя собрать пачку %2$s, которую ветром расшвыряло по окрестностям. Всего их было {Y%3$d{x штук%3$Iу|и|."),
                russian_case( quest->customerName.getForLang( lang ), '1' ).c_str( ),
                russian_case( quest->itemMltName.getForLang( lang ), '2' ).c_str( ),
                quest->total.getValue( ) ) << endl;
}

void LocateSecretaryScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    oldact(_("$c1 смотрит на тебя широко раскрытыми от ужаса глазами."), ch, 0, hero, TO_VICT);    
    oldact(_("$c1 смотрит на $C4 широко раскрытыми от ужаса глазами."), ch, 0, hero, TO_NOTVICT);    
    tell_act( hero, ch, _("Случилось ужасное. С моего рабочего стола сквозняком выдуло в окно пачку {W$n2{G и расшвыряло по окрестностям!"),
              quest->itemMltName.getForLang( viewerLang( hero ) ).c_str( ) );
    tell_raw( hero, ch, _("Если я их не соберу, меня казнят, а не то и уволят."));
    oldact(_("$c1 жалобно всхлипывает."), ch, 0, 0, TO_ROOM);
    tell_raw( hero, ch, _("Всего их было {W%d{G. Пожалуйста, отыщи их и принеси мне! Ты моя последняя надежда!"),
              quest->total.getValue( ) );
}

bool LocateSecretaryScenario::applicable( PCharacter *ch ) const
{
    return !IS_EVIL(ch);
}

/*-----------------------------------------------------------------------------
 * LocateAlchemistScenario
 *----------------------------------------------------------------------------*/
void LocateAlchemistScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf ) const
{
    lang_t lang = viewerLang( hero );

    buf << fmt( hero, _("В лаборатории %1$s взрывом расшвыряло %2$s, в количестве {Y%3$d{x штук%3$Iи||."),
                russian_case( quest->customerName.getForLang( lang ), '2' ).c_str( ),
                russian_case( quest->itemMltName.getForLang( lang ), '4' ).c_str( ),
                quest->total.getValue( ) ) << endl
        << fmt( hero, _("%1$s просит тебя попытаться собрать их."),
                russian_case( quest->customerName.getForLang( lang ), '1' ).c_str( ) ) << endl;
}

void LocateAlchemistScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    oldact(_("$c1 отрывает взгляд от пробирок и поворачивается к тебе."), ch, 0, hero, TO_VICT);    
    oldact(_("$c1 отрывает взгляд от пробирок и поворачивается к $C2."), ch, 0, hero, TO_NOTVICT);    
    tell_raw(hero, ch, _("Недавно я что-то смешал не в тех пропорциях.."));
    oldact(_("$c1 думает о чем-то, уставившись в одну точку."), ch, 0, 0, TO_ROOM);
    tell_act(hero, ch, _("Да, так вот.. в моей лаборатории прогремел взрыв, и {W$n4{G расшвыряло в разные стороны."),
             quest->itemMltName.getForLang( viewerLang( hero ) ).c_str( ));
    tell_raw(hero, ch, _("По моим подсчетам, их около {W%d{G. Поищи, вдруг тебе повезет."),
            quest->total.getValue( ));
    oldact(_("$c1 снова возвращается к работе."), ch, 0, 0, TO_ROOM);
}

/*-----------------------------------------------------------------------------
 * LocateTorturerScenario
 *----------------------------------------------------------------------------*/
void LocateTorturerScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf ) const
{
    lang_t lang = viewerLang( hero );

    buf << fmt( hero, _("Поставщик не донес %1$s орудия пыток, растеряв их на полпути от %2$s."),
                russian_case( quest->customerName.getForLang( lang ), '3' ).c_str( ),
                quest->targetArea.getForLang( lang ).c_str( ) ) << endl
        << fmt( hero, _("Всего их было {Y%1$d{x штук%1$Iи||."),
                quest->total.getValue( ) ) << endl
        << fmt( hero, _("%1$s просит тебя собрать их и отдать ему."),
                russian_case( quest->customerName.getForLang( lang ), '1' ).c_str( ) ) << endl;
}

void LocateTorturerScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    tell_act(hero, ch, _("Заказа$Gло|л|ла я на днях набор отменнейших пыточных приспособлений. "
                    "Железные девы 'от кутюр', наручники под ключ.. ну, вы меня понимаете."));
    tell_act(hero, ch, _("Но балбес поставщик растерял все по пути от {W$t{G сюда. "
                   "На нем мне пришлось опробовать старые средства, а вот заказанное добро "
                   "до сих пор валяется где-то на дороге."),
            quest->targetArea.getForLang( viewerLang( hero ) ).c_str( ));
    tell_raw(hero, ch, _("Всего там {W%d{G железок. Приволоки их сюда, если ты действительно "
                   "такой хороший сыщик, как о тебе рассказывают."),
            quest->total.getValue( ));
}

bool LocateTorturerScenario::applicable( PCharacter *ch ) const
{
    return !IS_GOOD(ch);
}

