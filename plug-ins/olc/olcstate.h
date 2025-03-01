/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OLCSTATE_H__
#define __OLCSTATE_H__

#include <iomanip>

using namespace std;

#include "logstream.h"
#include "grammar_entities.h"
#include "commandbase.h"
#include "interprethandler.h"
#include "staticlist.h"
#include "commandinterpreter.h"
#include "sedit.h"
#include "xmlboolean.h"
#include "xmldocument.h"
#include "pcharacter.h"
#include "arg_utils.h"

class Descriptor;
struct AreaIndexData;
class GlobalBitvector;
class XMLJsonValue;
class ExtraDescription;

typedef enum {
    ED_NO_FLAG=0,
    ED_UPPER_FIRST_CHAR=1,
    ED_NO_NEWLINE=2,
    ED_ADD_NEWLINE=4,
    ED_HELP_HINTS=8,
    ED_UPPERCASE=16,
    ED_JSON=32,
    ED_CALL_DONE=64
} editor_flags;

class OLCCommand : public CommandBase {
public:    
    OLCCommand( const DLString & );

    virtual const DLString& getName( ) const;
    virtual short getLog( ) const;
    virtual bool matches( const DLString & ) const;
    virtual int properOrder( Character * ) const;
    virtual int dispatch( const InterpretArguments & );
    virtual int dispatchOrder( const InterpretArguments & );
    virtual void entryPoint( Character *, const DLString & );

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
    static bool can_edit( Character *, AreaIndexData * );
    
    /** Find OLC input handler for descriptor. */
    static OLCState::Pointer getOLCState(Descriptor *d);

    /** returns corresponding area pointer for mob/room/obj vnum */
    static AreaIndexData *get_vnum_area( int );

protected:
    virtual void statePrompt( Descriptor *d ) = 0;
    virtual void changed( PCharacter * ) = 0;
    virtual void commit( ) = 0;

    bool sedit(DLString &);
    bool sedit(XMLString &);
    bool xmledit(XMLDocument::Pointer &xml);

    void seditDone( );

    bool flagBitsEdit(Flags &field);
    bool flagBitsEdit(const FlagTable &table, int &field);
    bool flagBitsEdit(const FlagTable &table, Flags &field);
    bool flagValueEdit(Enumeration &field);
    bool flagValueEdit(const FlagTable &table, int &field);
    bool flagValueEdit(const FlagTable &table, Flags &field);
    bool enumerationArrayEdit(EnumerationArray &field);
    bool enumerationArrayWebEdit(EnumerationArray &field);
    template<typename E> inline
    bool globalBitvectorEdit(GlobalBitvector &field);
    template<typename R, typename E> inline
    bool globalReferenceEdit(GlobalReference<R, E> &field);
    bool numberEdit(int minValue, int maxValue, int &field);
    bool numberEdit(long minValue, long maxValue, long &field);
    bool rangeEdit(int minValue, int maxValue, int &field1, int &field2);
    bool boolEdit(bool &field);
    bool diceEdit(int *field);
    bool genderEdit(Grammar::MultiGender &field);
    bool genderEdit(XMLString &field);
    bool stringListEdit(XMLStringList &values);
    bool extraDescrEdit(ExtraDescrList &list);
    bool editorCopy(const DLString &original);
    bool editorCopy(const char *field);
    bool editorPaste(DLString &original, editor_flags flags = ED_NO_FLAG);
    bool editorWeb(const DLString &original, const DLString &saveCommand, editor_flags flags = ED_NO_FLAG);
    bool editor(const char *argument, DLString &original, editor_flags flags = ED_NO_FLAG);
    bool editor(const char *argument, InflectedString &original, editor_flags flags = ED_NO_FLAG);
    bool editor(const char *argument, XMLStringList &values, editor_flags flags = ED_NO_FLAG);
    bool editor(const char *argument, XMLJsonValue &value, editor_flags flags = ED_NO_FLAG);
    bool editBehaviors(GlobalBitvector &behaviors, Json::Value &props);
    bool editProps(GlobalBitvector &behaviors, Json::Value &props, const DLString &arguments);
    
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

            try {
                if (((*state)->*method)( pch, args ))
                    state->changed( pch );
            } catch (const Exception &e) {
                LogStream::sendError() << "OLC " << e.what() << endl;
                pch->pecho(e.what());
                state->detach(pch);
            }
        }
        
        ::Pointer<T> state;
        Method method;
    };
   
// MOC_SKIP_BEGIN 
    void do_commands(PCharacter *ch) {
        Chain *cmd;
        ostringstream os;

        os << "{YСписок встроенных команд{x" << endl;

        for (cmd = Chain::begin( ); cmd; cmd = cmd->getNext( )) 
            os << "{g" << setiosflags(ios::left) << setw(20) << cmd->getKey() 
               << "{w" << resetiosflags(ios::left) << cmd->getVal().help << endl;

        ch->send_to( os.str() );
    }
// MOC_SKIP_END   
 
    virtual CommandBase::Pointer findCommand( PCharacter *pch, const DLString &name )
    {
        Chain *cmd;

        for (cmd = Chain::begin( ); cmd; cmd = cmd->getNext( ))
            if (name.strPrefix( cmd->getKey( ) ) || name.strPrefix(cmd->getVal().rname))
                return typename Command::Pointer( NEW, 
                                         (T *)this, 
                                         cmd->getKey( ), 
                                         cmd->getVal( ).method );
        
        return CommandBase::Pointer( );
    }
};    

extern DLString web_edit_button(bool showWeb, Character *ch, const DLString &editor, const DLString &args);

#define OLC_STATE(State) \
template <> \
OLCStateTemplate<State>::Chain *OLCStateTemplate<State>::Chain::first = 0

#define OLC_CMD( State, Cmd, rname, help ) \
namespace olc { struct Cmd##_type; } \
template <> bool State::cmd<olc::Cmd##_type>( PCharacter *ch, char *argument ); \
OLCStateTemplate<State>::Chain olc_##State##_##Cmd##_registrator( #Cmd, OLCStateTemplate<State>::SubCommandInfo(&State::cmd<olc::Cmd##_type>, rname, help)); \
template <> bool State::cmd<olc::Cmd##_type>( PCharacter *ch, char *argument )

template<typename E> inline
bool OLCState::globalBitvectorEdit(GlobalBitvector &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ch->pecho("Использование:\r\n{W%1$s{x значения - установить новые значения\r\n"
                   "{W%1$s clear{x - очистить поле\r\n"
                   "{W? %1$s{x - показать таблицу значений", cmd);
        return false;
    }

    if (arg_is_clear(args)) {
        field.clear();
        ch->pecho("Поле очищено.");
        return true;
    }

    GlobalRegistry<E> *registry = static_cast<GlobalRegistry<E>*>(field.getRegistry());
    
    GlobalBitvector newField(registry);
    StringSet values;
    values.fromString(args);
    for (auto &v: values) {
        E *elem = registry->findExisting(v);
        if (!elem)
            elem = registry->findUnstrict(v);

        if (!elem) {
            ch->pecho("Значение '%s' не найдено.", args.c_str());
            return false;
        }

        newField.set(elem->getIndex());
    }

    field.clear();
    field.set(newField);
    ch->pecho("Новое значение: {g%s{x", field.toString().c_str());
    return true;    
}

template<typename R, typename E> inline
bool OLCState::globalReferenceEdit(GlobalReference<R, E> &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ch->pecho("Использование:\r\n{W%1$s{x значение - установить новое значение", cmd);
        ch->pecho("{y{hc%1$s none{x - очистить значение", cmd);
        return false;
    }

    // Either assign an exact match or look for unique unstrict match.
    E *elem = R::getThis()->findExisting(args);

    if (!elem) {
        elem = R::getThis()->findUnstrict(args);

        if (!elem) {
            ch->pecho("Значение '%s' не найдено.", args.c_str());
            return false;
        }

        list<E *> matches = R::getThis()->findAll(args);
        if (matches.size() > 1) {
            ch->pecho("Найдено несколько совпадений для '%s', уточни значение:", args.c_str());
            for (auto &m: matches) {
                ch->pecho("    {y{hc%s %s{x", cmd, m->getName().c_str());
            }
            return false;
        }
    }

    field.assign(elem->getIndex());
    ch->pecho("Новое значение: {g%s{x", field.getName().c_str());
    return true;    
}

#endif
