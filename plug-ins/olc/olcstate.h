/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OLCSTATE_H__
#define __OLCSTATE_H__

#include <iomanip>

using namespace std;

#include "commandbase.h"
#include "interprethandler.h"
#include "staticlist.h"
#include "commandinterpreter.h"
#include "sedit.h"
#include "xmlboolean.h"
#include "xmldocument.h"
#include "pcharacter.h"

class Descriptor;
struct area_data;

class OLCCommand : public CommandBase {
public:    
    OLCCommand( const DLString & );

    virtual const DLString& getName( ) const;
    virtual short getLog( ) const;
    virtual bool matches( const DLString & ) const;
    virtual bool properOrder( Character * );
    virtual bool dispatch( const InterpretArguments & );
    virtual bool dispatchOrder( const InterpretArguments & );
    virtual void run( Character *, const DLString & );

private:
    virtual void run( PCharacter *, char * ) = 0;

    DLString name;
};

class OLCInterpretLayer : public InterpretLayer {
public:
    virtual void putInto( );
    virtual bool process( InterpretArguments &iargs );
};

class OLCState : public InterpretHandler {
XML_OBJECT
    friend class OLCStringEditor;
public:
    typedef ::Pointer<OLCState> Pointer;

    OLCState();

    virtual int handle(Descriptor *d, char *arg);
    virtual void prompt( Descriptor *d );
    
    virtual CommandBase::Pointer findCommand( PCharacter *, const DLString & ) = 0;

    void attach( PCharacter * );
    void detach( PCharacter * );
    
    static bool can_edit( Character *, int );
    static bool can_edit( Character *, area_data * );
    
    /* returns corresponding area pointer for mob/room/obj vnum */
    static area_data *get_vnum_area( int );

protected:
    virtual void statePrompt( Descriptor *d ) = 0;
    virtual void changed( PCharacter * ) = 0;
    virtual void commit( ) = 0;

    bool sedit(DLString &);
    bool sedit(XMLString &);
    bool sedit(char *&);
    bool xmledit(XMLDocument::Pointer &xml);

    void seditDone( );

    bool mapEdit( Properties &map, DLString &args );
    bool flagBitsEdit(const FlagTable &table, int &field);
    bool flagValueEdit(const FlagTable &table, int &field);
    bool numberEdit(int minValue, int maxValue, int &field);
    bool numberEdit(long minValue, long maxValue, long &field);
    bool diceEdit(int *field);
    bool extraDescrEdit(EXTRA_DESCR_DATA *&list);
    bool editorCopy(const DLString &original);
    bool editorPaste(DLString &original);
    bool editorPaste(char *&field);
    bool editor(const char *command, DLString &original);
    bool editor(const char *command, char *&field);

    Descriptor *owner;
    XML_VARIABLE XMLBoolean inSedit;
    XML_VARIABLE OLCStringEditor strEditor;
    XML_VARIABLE XMLString lastCmd, lastArgs;
};

template <typename T>
class OLCStateTemplate : public virtual OLCState {
public:    
    typedef bool (T::*Method)( PCharacter *, char * );
    struct SubCommandInfo {
        SubCommandInfo(Method m, const char *r, const char *h) 
                 : method(m), rname(r), help(h) {}
        Method method;
        DLString rname;
        DLString help;
    };
    typedef StaticList<DLString, SubCommandInfo> Chain;

    class Command : public OLCCommand {
    public:        
        typedef ::Pointer<Command> Pointer;
    
        Command( ::Pointer<T> s, const DLString &n, Method m )
                : OLCCommand( n ), state( s ), method( m )
        {
        }

    private:
        virtual void run( PCharacter *pch, char *args )
        {
            state->lastCmd.setValue( getName( ) );
            state->lastArgs.setValue( args );

            if (((*state)->*method)( pch, args ))
                state->changed( pch );
        }
        
        ::Pointer<T> state;
        Method method;
    };
    
    void do_commands(PCharacter *ch) {
        Chain *cmd;
        ostringstream os;
        int i;
// TODO draw a table with name / rname / help 
        for (cmd = Chain::begin( ), i = 0; cmd; cmd = cmd->getNext( ), i++) {
            if(i && i % 4 == 0)
                os << endl;
            os << std::setw(19) << std::setiosflags(ios::left) << cmd->getKey( );
        }
        os << endl;

        ch->send_to( os.str() );
    }
    
    virtual CommandBase::Pointer findCommand( PCharacter *pch, const DLString &name )
    {
        Chain *cmd;

        for (cmd = Chain::begin( ); cmd; cmd = cmd->getNext( ))
            if (name.strPrefix( cmd->getKey( ) ))
                return typename Command::Pointer( NEW, 
                                         (T *)this, 
                                         cmd->getKey( ), 
                                         cmd->getVal( ).method );
        
        return CommandBase::Pointer( );
    }
};    

#define OLC_STATE(State) \
template <> \
OLCStateTemplate<State>::Chain *OLCStateTemplate<State>::Chain::first = 0

#define OLC_CMD( State, Cmd, rname, help ) \
namespace olc { struct Cmd##_type; } \
template <> bool State::cmd<olc::Cmd##_type>( PCharacter *ch, char *argument ); \
OLCStateTemplate<State>::Chain olc_##State##_##Cmd##_registrator( #Cmd, OLCStateTemplate<State>::SubCommandInfo(&State::cmd<olc::Cmd##_type>, rname, help)); \
template <> bool State::cmd<olc::Cmd##_type>( PCharacter *ch, char *argument )


#endif
