#ifndef WRAPPEDCOMMAND_H
#define WRAPPEDCOMMAND_H

#include "wrappertarget.h"
#include "command.h"

/** A command that allows 'run' method overrides in Fenia.
 */
class WrappedCommand : public virtual Command, public WrapperTarget {
public:
    typedef ::Pointer<WrappedCommand> Pointer;

        /** Gets unique command ID for Fenia */
        virtual long long getID() const;

        /** Main entry point for command interpreter, with fenia override */
        virtual void entryPoint( Character *, const DLString & );

protected:
        bool feniaOverride(Character *, const DLString &);

        void linkWrapper();

        void unlinkWrapper();

};


#endif