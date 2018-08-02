/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ALIGN_GAIN_H
#define ALIGN_GAIN_H

#include <list>

class Character;
class Skill;

void align_gain( Character *, int );

void align_gain_cast( Character *, Character *, Skill * );
void align_gain_heal( Character *, Character *, Skill * );
void align_gain_kill( Character *, Character * );
void align_gain_vampiric( Character *, Character *, Object * );
void align_gain_attack( Character *, Character * );
void align_gain_rescue( Character *, Character * );
void align_gain_groupkill( Character *, Character *, list<Character *> & );

#endif
