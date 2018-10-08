/* $Id: mobiles.h,v 1.1.2.2 2005/11/26 16:59:51 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef INVASIONMOB_H
#define INVASIONMOB_H

#include "basicmobilebehavior.h"

class Invasion;
class PCharacter;

class InvasionMob : public BasicMobileDestiny {
XML_OBJECT    
public:    
        typedef ::Pointer<InvasionMob> Pointer;
        
        InvasionMob( );

        virtual bool death( Character * );
        
protected:    
        virtual void actDeath( Character * );
};

class InvasionHelper : public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<InvasionHelper> Pointer;
    
        InvasionHelper( );

        virtual bool death( Character * );
        virtual void tell( Character *, const char * );

protected:
        virtual void actWrongSpeech( PCharacter * );
        virtual void actTooMuch( PCharacter *, Object * );
        virtual void actGiveInstrument( PCharacter *, Object * );
};

#endif

