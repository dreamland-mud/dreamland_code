/* $Id$
 *
 * ruffina, 2004
 */
#include "scenarios_impl.h"
#include "locatequest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "interp.h"
#include "mercdb.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*-----------------------------------------------------------------------------
 * LocateMousesScenario
 *----------------------------------------------------------------------------*/
void LocateMousesScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf ) const
{
    buf << russian_case( quest->customerName.getValue( ), '1' ) << " "
        << "жалуется на банду грызунов, которые растащили " << russian_case( quest->itemMltName.getValue( ), '4' ) << " "
        << "из ее кладовки." << endl
        << "Пропало довольно много, но она будет благодарна тебе, если ты принесешь ей "
        << "хотя бы {Y" << quest->total << "{x штук" << GET_COUNT(quest->total, "у", "и", "") << "." << endl;
}

void LocateMousesScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    act("$c1, всплеснув руками, бросается тебе навстречу.", ch, 0, hero, TO_VICT);    
    act("$c1, всплеснув руками, бросается навстречу $C3.", ch, 0, hero, TO_NOTVICT);    
    tell_raw( hero, ch, "Подлые мыши, спасу от них нет никакого. Чем я их только не травила!");
    tell_act( hero, ch, "Растащили из кладовки {W$n4{G. Всех их уже, конечно, не отыскать.", 
              quest->itemMltName.c_str( ) );
    tell_raw( hero, ch, "Но попробуй собери хотя бы {W%d{G штук. Жду с нетерпением.",
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
    buf << russian_case( quest->customerName.getValue( ), '1' ) << " "
        << "просит тебя собрать пачку " << russian_case( quest->itemMltName.getValue( ), '2' )
        << ", которую ветром расшвыряло по окрестностям. Всего их было {Y"
        << quest->total << "{x штук" << GET_COUNT(quest->total, "у", "и", "") << "." << endl;
}

void LocateSecretaryScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    act("$c1 смотрит на тебя широко раскрытыми от ужаса глазами.", ch, 0, hero, TO_VICT);    
    act("$c1 смотрит на $C4 широко раскрытыми от ужаса глазами.", ch, 0, hero, TO_NOTVICT);    
    tell_act( hero, ch, "Случилось ужасное. С моего рабочего стола сквозняком выдуло в окно пачку {W$n2{G и расшвыряло по окрестностям!",
              quest->itemMltName.c_str( ) );
    tell_raw( hero, ch, "Если я их не соберу, меня казнят, а не то и уволят.");
    act("$c1 жалобно всхлипывает.", ch, 0, 0, TO_ROOM);
    tell_raw( hero, ch, "Всего их было {W%d{G. Пожалуйста, отыщи их и принеси мне! Ты моя последняя надежда!",
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
    buf << "В лаборатории " << russian_case( quest->customerName.getValue( ), '2' ) << " "
        << "взрывом расшвыряло " << russian_case( quest->itemMltName.getValue( ), '4' ) << ", "
        << "в количестве {Y" << quest->total << "{x штук" << GET_COUNT(quest->total, "и", "", "") << "." << endl
        << russian_case( quest->customerName.getValue( ), '1' ) << " просит тебя попытаться собрать их." << endl;
}

void LocateAlchemistScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    act("$c1 отрывает взгляд от пробирок и поворачивается к тебе.", ch, 0, hero, TO_VICT);    
    act("$c1 отрывает взгляд от пробирок и поворачивается к $C2.", ch, 0, hero, TO_NOTVICT);    
    tell_raw(hero, ch, "Недавно я что-то смешал не в тех пропорциях..");
    act("$c1 думает о чем-то, уставившись в одну точку.", ch, 0, 0, TO_ROOM);
    tell_act(hero, ch, "Да, так вот.. в моей лаборатории прогремел взрыв, и {W$n4{G расшвыряло в разные стороны.",
             quest->itemMltName.c_str( ));
    tell_raw(hero, ch, "По моим подсчетам, их около {W%d{G. Поищи, вдруг тебе повезет.",
            quest->total.getValue( ));
    act("$c1 снова возвращается к работе.", ch, 0, 0, TO_ROOM);
}

/*-----------------------------------------------------------------------------
 * LocateTorturerScenario
 *----------------------------------------------------------------------------*/
void LocateTorturerScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf ) const
{
    buf << "Поставщик не донес " << russian_case( quest->customerName.getValue( ), '3' ) 
        << " орудия пыток, растеряв их на полпути от " << quest->targetArea << "." << endl
        << "Всего их было {Y" << quest->total << "{x штук" << GET_COUNT(quest->total, "и", "", "") << "." << endl
        << russian_case( quest->customerName.getValue( ), '1' ) << " просит тебя собрать их и отдать ему." << endl;
}

void LocateTorturerScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    tell_act(hero, ch, "Заказа$Gло|л|ла я на днях набор отменнейших пыточных приспособлений. "
                    "Железные девы 'от кутюр', наручники под ключ.. ну, вы меня понимаете.");
    tell_act(hero, ch, "Но балбес поставщик растерял все по пути от {W$t{G сюда. "
                   "На нем мне пришлось опробовать старые средства, а вот заказанное добро "
                   "до сих пор валяется где-то на дороге.",
            quest->targetArea.c_str( ));
    tell_raw(hero, ch, "Всего там {W%d{G железок. Приволоки их сюда, если ты действительно "
                   "такой хороший сыщик, как о тебе рассказывают.",
            quest->total.getValue( ));
}

bool LocateTorturerScenario::applicable( PCharacter *ch ) const
{
    return !IS_GOOD(ch);
}

