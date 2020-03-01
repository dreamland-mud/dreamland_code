/* $Id: practice.h,v 1.1.2.4.6.2 2007/09/11 00:28:23 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef PRACTICE_H
#define PRACTICE_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;
class NPCharacter;
class Skill;

class CPractice : public CommandPlugin, public DefaultCommand {
public:
        typedef ::Pointer<CPractice> Pointer;
    
        CPractice( );

        virtual void run( Character *, const DLString & );

private:
        static const DLString COMMAND_NAME;
                
        void pracShow( PCharacter * );
        void pracLearn( PCharacter *, DLString & );
        void pracHere( PCharacter * );
        PCharacter * findTeacher( PCharacter *, Skill * = NULL );
        NPCharacter * findPracticer( PCharacter *, Skill * = NULL );
};

#endif

