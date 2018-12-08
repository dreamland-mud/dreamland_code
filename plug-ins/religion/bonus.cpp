
        if (gch->getReligion()->hasBonus(gch, RB_KILLEXP, time_info))
            gch->pecho("{c%^N1 дарует тебе больше опыта.{x", gch->getReligion()->getRussianName().c_str());
        else
            gch->pecho("{cСегодня %s благосклонны к тебе, даруя тебе больше опыта.{x", 
                       IS_GOOD(gch) ? "силы добра" : IS_NEUTRAL(gch) ? "нейтральные силы" : "силы зла");
