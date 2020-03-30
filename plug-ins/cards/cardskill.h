/* $Id: cardskill.h,v 1.1.2.8.6.3 2008/05/27 21:30:01 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __CARDSKILL_H__
#define __CARDSKILL_H__

#include "basicskill.h"

#include "xmlmap.h"
#include "xmlflags.h"

class CardSkill : public BasicSkill {
XML_OBJECT
friend class CardSkillLoader;
public:
    typedef ::Pointer<CardSkill> Pointer;
    
    CardSkill( );

    virtual SkillGroupReference & getGroup( );
    
    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );

    virtual void show( PCharacter *, std::ostream & ); 

    virtual const DLString & getCategory( ) const
    {
        return CATEGORY;
    }

protected:
    static bool isCard( Character * );
    static int findCardLevel( Character * );

    static const DLString CATEGORY;

    XML_VARIABLE XMLString  hint;
    XML_VARIABLE XMLInteger cardLevel;
};


#endif
