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
class GlobalRegistryBase;
class StringList;
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

    /** True only inside handle(), i.e. while input from the editor's own
     *  descriptor is being processed. owner is set there and nowhere else. */
    bool isHandling( ) const { return owner != 0; }

    /** returns corresponding area pointer for mob/room/obj vnum */
    static AreaIndexData *get_vnum_area( int );

    /** Op journal entry for a subcommand that changed something. Typed in
     *  game or run by OlcOps, every edit lands in the same journal. */
    void journal( PCharacter *, const DLString &cmd, const DLString &args );

    /** What this editor edits, for the journal: "room", 3001. Editors that
     *  don't override it are journaled as "olc", 0. */
    virtual DLString getJournalKind( ) const;
    virtual int getJournalVnum( ) const;

    /** Names the edited entity inside web menu commands ("room:3001",
     *  "skill:fireball"), so a menu clicked from scrollback can't edit
     *  whatever is open now. Empty: menus are not bound. */
    virtual DLString getMenuKey( );
    /** The key of the editor drawing right now, empty outside one: web_list_menu
     *  binds every menu it draws to it. Set by Command::run for each subcommand and
     *  by each editor's show, so a child editor shown from a parent's subcommand,
     *  or an editor opened by name, binds to itself. */
    static DLString currentMenuKey;
    /** The editor drawing right now draws for a web client: web_list_menu
     *  draws the full field widget only then. Set with currentMenuKey by a scope
     *  that knows the viewer; false outside one, which draws the telnet form. */
    static bool currentWeb;

    /** Set by journal(): the last subcommand changed the edited entity. */
    bool lastChanged;
    /** Set by journal(), cleared when 'done' journals: edits not yet committed. */
    bool pendingDone;
    /** Journal profile and source: "immortal"/"editor" in game, OlcOps sets its own. */
    DLString journalProfile, journalSource;

protected:
    virtual void statePrompt( Descriptor *d ) = 0;
    virtual void changed( PCharacter * ) = 0;
    virtual void commit( ) = 0;

    bool sedit(DLString &);
    bool sedit(XMLString &);
    bool xmledit(XMLDocument::Pointer &xml);
    bool xmleditWeb(XMLDocument::Pointer &xml, const DLString &saveCommand);
    bool xmleditPaste(XMLDocument::Pointer &xml);

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
    /** quiet: no warning when the props key is not a behavior name (callers
     *  that own a fixed non-behavior key, like oedit 'msg'). */
    bool editProps(GlobalBitvector &behaviors, Json::Value &props, const DLString &arguments, bool quiet = false);
    
    Descriptor *owner;
    XML_VARIABLE XMLBoolean inSedit;
    XML_VARIABLE OLCStringEditor strEditor;
    XML_VARIABLE XMLString lastCmd, lastArgs;
};

/** Sets OLCState::currentMenuKey for the lifetime of one subcommand, and
 *  OLCState::currentWeb from the viewer. */
class OLCMenuKeyScope {
public:
    OLCMenuKeyScope( const DLString &key, Character *viewer );
    ~OLCMenuKeyScope( );
private:
    DLString saved;
    bool savedWeb;
};

/** A menu key part: spaces become '_', and nothing that could break the menu tag
 *  or the client's '$' substitution. */
extern DLString olc_menu_key_part( const DLString &part );
/** A web menu id bound to the entity being drawn ('olcpick <key> <cmd>'), or cmd
 *  as is outside an editor. Hand-built menus use it, web_list_menu does it itself. */
extern DLString olc_menu_id( const DLString &cmd );

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

            // Handlers tokenize args in place: keep a copy for the journal.
            DLString argsCopy = args;

            // Menus drawn by this subcommand are bound to the edited entity.
            OLCMenuKeyScope menuScope( state->getMenuKey( ), pch );

            try {
                if (((*state)->*method)( pch, args )) {
                    state->changed( pch );
                    // 'done' edits nothing itself, but it is the moment a telnet
                    // editor's copy reaches the prototype: journal it once per
                    // edit run, so the area pusher sees the save it triggers.
                    if (getName( ) != "done")
                        state->journal( pch, getName( ), argsCopy );
                    else if (state->pendingDone) {
                        state->journal( pch, getName( ), "" );
                        state->pendingDone = false;
                    }
                }
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

        // An exact name wins over a prefix: otherwise 'tier' reaches 'tierstyle'
        // and 'act' reaches 'actdel', whichever was registered later.
        for (cmd = Chain::begin( ); cmd; cmd = cmd->getNext( ))
            if ((name ^ cmd->getKey( )) || (!cmd->getVal().rname.empty() && (name ^ cmd->getVal().rname)))
                return typename Command::Pointer( NEW, 
                                         (T *)this, 
                                         cmd->getKey( ), 
                                         cmd->getVal( ).method );

        for (cmd = Chain::begin( ); cmd; cmd = cmd->getNext( ))
            if (name.strPrefix( cmd->getKey( ) ) || name.strPrefix(cmd->getVal().rname))
                return typename Command::Pointer( NEW, 
                                         (T *)this, 
                                         cmd->getKey( ), 
                                         cmd->getVal( ).method );
        
        return CommandBase::Pointer( );
    }
};    

struct OLCMenuFrame;
/** A clickable OLC link (a web_cmd, an edit button) in the field look. A web
 *  viewer gets '{y[{W' link '{y]{x'; anyone else gets frame.open + link +
 *  frame.close, the link as telnet printed it before. */
extern DLString olc_link(Character *ch, const DLString &link, const OLCMenuFrame &frame);
/** olc_link around web_cmd(ch, cmd, seeFmt). */
extern DLString olc_cmd(Character *ch, const DLString &cmd, const DLString &seeFmt, const OLCMenuFrame &frame);
/** Spaces a list header needs after a column of OLC_FRAME_NONE links: the web frame adds two columns. */
extern DLString olc_head_pad(Character *ch);
/** An OLC edit button: web_edit_button in the field look. Empty for telnet and
 *  for players without OLC security, as web_edit_button is. */
extern DLString olc_edit_button(Character *ch, const DLString &editor, const DLString &args);
/** olc_edit_button, or nothing when showWeb is off ('show noweb'). */
extern DLString olc_edit_button(bool showWeb, Character *ch, const DLString &editor, const DLString &args);
/** Every name in a flag table, space-separated. */
extern DLString flag_table_all_names(const FlagTable &table);

/** What a menu field looks like around its value on telnet: whatever the caller
 *  printed before the field widget, so telnet output stays as it was. */
struct OLCMenuFrame {
    const char *open;
    const char *close;
};
/** '[{W' value '{x]', the usual OLC field. */
extern const OLCMenuFrame OLC_FRAME_BRACKETS;
/** The bare value: the caller prints its own colour around it. */
extern const OLCMenuFrame OLC_FRAME_NONE;
/** '[' value ']' with no colour of its own (oedit). */
extern const OLCMenuFrame OLC_FRAME_PLAIN;

/** The OLC field widget, every menu below ends here. A web viewer gets
 *  '{y[{W' value ' {Y<triangle>{y]{x' with the value and the triangle inside the
 *  drop-down toggle. Anyone else gets frame.open + the menu + frame.close: the
 *  field exactly as telnet printed it before the widget, the menu tags invisible
 *  there. An empty label shows as 'none' on the web only. */
extern DLString olc_menu_widget(bool web, const StringList &commands, const DLString &id, const DLString &label,
                                const OLCMenuFrame &frame);

/** A field value as a web drop-down of every flag in the table: picking one sends
 *  '<cmd> <flag>'. showWeb=false ('show noweb') prints the framed label, no menu. */
extern DLString web_flag_menu(bool showWeb, const FlagTable &table, const DLString &cmd, const DLString &label,
                              const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);
/** web_flag_menu for a flagBitsEdit field: an enumerated table's zero row ('null')
 *  is left out, since toggling it sets bit 0 and restricts to nothing. */
extern DLString web_flag_bits_menu(bool showWeb, const FlagTable &table, const DLString &cmd, const DLString &label,
                                   const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);
/** A field value as a web drop-down of fixed values: picking one sends '<cmd> <value>'. */
extern DLString web_list_menu(bool showWeb, const StringList &values, const DLString &cmd, const DLString &label,
                              const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);
/** A field value as a web drop-down of every valid registry element (races, liquids,
 *  clans...): picking one sends '<cmd> <name>'. A non-empty clearWord is offered last
 *  (liquids: 'none'). */
extern DLString web_registry_menu(bool showWeb, GlobalRegistryBase *registry, const DLString &cmd, const DLString &label,
                                  const DLString &clearWord = DLString::emptyString,
                                  const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);
/** A set edited by globalBitvectorEdit as a web drop-down: a member sends '<cmd> -name',
 *  any other element '<cmd> +name'. */
extern DLString web_bitvector_menu(bool showWeb, const GlobalBitvector &field, const DLString &cmd, const DLString &label,
                                   const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);
/** Full names of every mob special function (spec_...), then clearWord if given. */
extern StringList spec_function_names(const DLString &clearWord = DLString::emptyString);
/** A material field (fight/material.json names) as a web drop-down: '<cmd> <name>'. */
extern DLString web_material_menu(bool showWeb, const DLString &cmd, const DLString &label,
                                  const DLString &clearWord = DLString::emptyString,
                                  const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);
/** A genderEdit field as an m/f/n/p web drop-down. */
extern DLString web_gender_menu(bool showWeb, const DLString &cmd, const DLString &label,
                                const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);
/** A boolEdit field as a y/n web drop-down. */
extern DLString web_bool_menu(bool showWeb, const DLString &cmd, const DLString &label,
                              const OLCMenuFrame &frame = OLC_FRAME_BRACKETS);

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
                   "{W%1$s +значение{x, {W%1$s -значение{x -- добавить или убрать одно значение\r\n"
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

    // '+name' adds and '-name' removes one element, the rest stays: the web menus
    // send these. The name may contain spaces ('+storm giant').
    if (args.at(0) == '+' || args.at(0) == '-') {
        bool fSet = args.at(0) == '+';
        DLString name = args.substr(1);
        name.stripWhiteSpace();

        // findExisting also returns placeholders of retired names.
        E *elem = registry->findExisting(name);
        if (elem && !elem->isValid())
            elem = 0;
        if (!elem)
            elem = registry->findUnstrict(name);

        if (!elem) {
            ch->pecho("Значение '%s' не найдено.", name.c_str());
            return false;
        }

        if (fSet)
            field.set(elem->getIndex());
        else
            field.remove(elem->getIndex());

        ch->pecho("Новое значение: {g%s{x", field.toString().c_str());
        return true;
    }

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
