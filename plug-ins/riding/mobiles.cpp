/* $Id$
 *
 * ruffina, 2004
 */
bool Rideable::canMount( Character *rider )
{
    Skill *skill = getRidingSkill( );

    if (!skill->usable( rider )) {
	rider->println("Ты не умеешь кататься на таких существах.");
	return false;
    }

    if (ch->leader && ch->leader != rider) {
	if (!gsn_steal_mount->usable( rider )) {
	    rider->pecho("%^C1 - чужая собственность, а ты не умеешь воровать.", ch);
	    return false;
	}
    }
}


