/* $Id: playerattributes.h,v 1.1.2.4.6.3 2008/03/04 07:24:12 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __PLAYERATTRIBUTES_H__
#define __PLAYERATTRIBUTES_H__

#pragma interface

#include <sstream>
#include <list>

#include "xmlattribute.h"
#include "xmlattributes.h"

class Character;
class PCharacter;

struct ScoreArguments {
    ScoreArguments(PCharacter *ch, list<DLString> &l)
            : pch(ch), lines(l)
    {
    }

        
    PCharacter *pch;
    list<DLString> &lines;
};

struct DeathArguments {
    DeathArguments( PCharacter *ch, Character *k )
            : pch( ch ), killer( k )
    {
    }
    
    PCharacter *pch;
    Character *killer;
};

struct RemortArguments {
    RemortArguments( PCharacter *ch, XMLAttributes *n )
            : pch( ch ), newAttributes( n )
    {
    }
    
    PCharacter *pch;
    XMLAttributes *newAttributes;
};

class RemortAttribute : public EventHandler<RemortArguments> {
public:
    virtual bool handle( const RemortArguments & );
};


struct PromptArguments {
    PromptArguments( PCharacter *ch, char c, ostringstream &buf )
                        : pch( ch ), letter( c ), buffer( buf )
    {
    }

    PCharacter *pch;
    char letter;
    ostringstream &buffer;
};  
                
struct WhoisArguments {
    WhoisArguments( PCharacter *ch, PCharacter *lch, list<DLString> &l )
           : pch( ch ), looker( lch ), lines( l )
    {
    }

    PCharacter *pch;
    PCharacter *looker;
    list<DLString> &lines;
};

extern template class EventHandler<RemortArguments>;
extern template class EventHandler<DeathArguments>;
extern template class EventHandler<PromptArguments>;
extern template class EventHandler<WhoisArguments>;

#endif
