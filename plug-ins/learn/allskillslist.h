/* $Id: allskillslist.h,v 1.1.2.5.10.3 2008/05/27 21:30:04 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __ALLSKILLSLIST_H__
#define __ALLSKILLSLIST_H__

#include <list>
#include <ostream>

#include "dlstring.h"

class Character;
class SkillGroup;

struct SkillInfo 
{
    DLString name;
    int learned;
    int real;
    int level; 
    int mana;
    int adept;
    int maximum;
    int help_id;
    bool spell;
    bool available;

    static bool cmp_by_name( SkillInfo a, SkillInfo b )
    {
        return a.name < b.name;
    }
    static bool cmp_by_level( SkillInfo a, SkillInfo b )
    {
        return a.level < b.level;
    }
    static bool cmp_by_learned( SkillInfo a, SkillInfo b )
    {
        return a.learned < b.learned;
    }

    const char * colorLearned( );
    const char * colorReal( );
    const char * color( int );
};

struct AllSkillsList : public std::list<SkillInfo> {
    bool parse( DLString &, std::ostream &, Character * );
    void make( Character *ch );
    void display( std::ostream & );
    
    DLString cmd, rcmd;
    int levHigh, levLow;
    SkillGroup *group;
    bool fSpells;
    bool (*criteria) ( SkillInfo, SkillInfo );
    bool fUsableOnly;
    bool fRussian;
    bool fShowHint;
    bool fCurrentProfAll;
};

#endif
