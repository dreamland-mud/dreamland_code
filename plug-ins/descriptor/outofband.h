#ifndef OUTOFBAND_H_
#define OUTOFBAND_H_

#include <sstream>
#include <map>
#include <list>
#include "plugin.h"
#include "oneallocate.h"
#include "dlstring.h"

class DLString;

struct OutOfBandArgs;
struct Descriptor;

using namespace std;

class OutOfBandCommand  : public virtual Plugin {
public:
    typedef ::Pointer<OutOfBandCommand> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual const DLString &getCommandType( ) const = 0;
    virtual void run( const OutOfBandArgs &args ) const = 0;
};

struct OutOfBandArgs  {
    OutOfBandArgs(Descriptor *d) {
        this->d = d;
    }
    Descriptor *d;
};

struct CharToRoomArgs : public OutOfBandArgs {
    CharToRoomArgs(Descriptor *d) : OutOfBandArgs(d) {
    }
};

struct ProtoInitArgs : public OutOfBandArgs {
    ProtoInitArgs(Descriptor *d, const char *proto) : OutOfBandArgs(d) {
        this->proto = proto;
    }
    DLString proto;
};

class OutOfBandManager : public virtual Plugin, public OneAllocate {
public:
        typedef ::Pointer<OutOfBandManager> Pointer;
        typedef multimap<DLString, OutOfBandCommand::Pointer> Commands;

        OutOfBandManager( );
        virtual ~OutOfBandManager( );
    
        void registrate( OutOfBandCommand::Pointer );
        void unregistrate( OutOfBandCommand::Pointer );
        void run(const DLString &commandType, const OutOfBandArgs& args) const;

        virtual void initialization( );
        virtual void destruction( );

protected:
        Commands commands;               
};

extern OutOfBandManager *outOfBandManager;    

#endif


