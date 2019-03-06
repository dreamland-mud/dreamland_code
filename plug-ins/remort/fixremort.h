/* $Id: fixremort.h,v 1.1.2.4 2006/01/15 00:09:00 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef FIXREMORT_H
#define FIXREMORT_H

#include "descriptorstatelistener.h"

class Object;
class PCharacter;

class FixRemortListener : public DescriptorStateListener {
public:
        typedef ::Pointer<FixRemortListener> Pointer;
        
        virtual void run( int, int, Descriptor * );
        
private:
        void fixNewRemort( PCharacter * );
        void fixOldRemort( PCharacter * );
        Object * makeCorpse( PCharacter * );
        void adjustObjects( PCharacter *, Object *, Object * );
        void adjustExp( PCharacter * );
        void adjustBonuses( PCharacter * );
};

#endif

