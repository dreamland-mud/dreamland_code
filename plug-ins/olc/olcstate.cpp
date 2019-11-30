/* $Id$
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "olcstate.h"
#include "olc.h"
#include "sedit.h"
#include "pcharacter.h"
#include "security.h"
#include "interp.h"
#include "arg_utils.h"
#include "websocketrpc.h"
#include "mercdb.h"

/*--------------------------------------------------------------------------
 * OLCCommand
 *-------------------------------------------------------------------------*/
OLCCommand::OLCCommand( const DLString &n ) : name( n )
{
}

const DLString& OLCCommand::getName( ) const
{
    return name;
}

short OLCCommand::getLog( ) const
{
    return LOG_ALWAYS;
}

bool OLCCommand::matches( const DLString &argument ) const
{
    // TODO match Russian names.
    return !argument.empty( ) && argument.strPrefix( name );
}

bool OLCCommand::properOrder( Character * )
{
    return false;
}

bool OLCCommand::dispatch( const InterpretArguments &iargs )
{
    return true;
}

bool OLCCommand::dispatchOrder( const InterpretArguments &iargs )
{
    return false;
}

void OLCCommand::run( Character *ch, const DLString &cArguments )
{
    char args[MAX_STRING_LENGTH];

    strcpy( args, cArguments.c_str( ) );
    run( ch->getPC( ), args );        
}

/*--------------------------------------------------------------------------
 * OLCInterpretLayer 
 *-------------------------------------------------------------------------*/
void
OLCInterpretLayer::putInto( )
{
    interp->put( this, CMDP_FIND, 5 );
}

bool 
OLCInterpretLayer::process( InterpretArguments &iargs )
{
    if (iargs.ch->is_npc( ))
        return true;
    
    OLCState::Pointer state = OLCState::getOLCState(iargs.d);
    if (!state) {
        return true;
    }
    
    if(iargs.cmdName.empty())
        iargs.cmdName = "show";

    if(iargs.cmdName == "?")
        iargs.cmdName = "olchelp";

    iargs.pCommand = state->findCommand( iargs.ch->getPC( ), iargs.cmdName );

    if (iargs.pCommand) 
        iargs.advance( );
    
    return true;
}

/*--------------------------------------------------------------------------
 * OLCState 
 *-------------------------------------------------------------------------*/
OLCState::OLCState() : inSedit(false), strEditor(*this)
{
}

OLCState::Pointer OLCState::getOLCState(Descriptor *d)
{
    OLCState::Pointer state;

    if (!d || d->handle_input.empty( ))
        return state;
    
    state = d->handle_input.front( ).getDynamicPointer<OLCState>( );
    return state;
}

int 
OLCState::handle(Descriptor *d, char *arg)
{
    int rc = 0;
    
    owner = d;
    if(inSedit.getValue( ))
        strEditor.eval(arg);
    else
        rc = InterpretHandler::handle(d, arg);
    owner = 0;

    return rc;
}

void OLCState::prompt( Descriptor *d )
{
    if(inSedit.getValue( ))
        strEditor.prompt(d);
    else
        statePrompt(d);
}

void OLCState::attach( PCharacter *ch ) 
{
    if (ch->desc)
        ch->desc->handle_input.push_front( this );
}

void OLCState::detach( PCharacter *ch ) 
{
    if (!ch->desc)
        return;

    handle_input_t::iterator i;
    handle_input_t &hi = ch->desc->handle_input;

    for(i = hi.begin(); i != hi.end(); i++)
        if(**i == this) {
            hi.erase(i);
            return;
        }
}


bool OLCState::can_edit( Character *ch, int vnum )
{
    if (!ch->is_npc( )) {
        XMLAttributeOLC::Pointer attr;
        int sec = ch->getPC( )->getSecurity();
        
        if (sec <= 0)
            return false;
        else if (sec > 9)
            return true;
            
        attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );
        
        if (attr) {
            XMLAttributeOLC::RangeList::iterator i;
            for (i = attr->vnums.begin( ); i != attr->vnums.end( ); i++) 
                if (i->minVnum <= vnum && vnum <= i->maxVnum)
                    return true;
        }
    }

    return false;
}

bool OLCState::can_edit( Character *ch, AREA_DATA *pArea )
{
    if (!ch->is_npc( )) {
        XMLAttributeOLC::Pointer attr;
        int a = pArea->min_vnum, b = pArea->max_vnum;
        int sec = ch->getPC( )->getSecurity();
        
        if (sec <= 0)
            return false;
        else if (sec > 9)
            return true;
            
        attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );

        if (attr) {
            XMLAttributeOLC::RangeList::iterator i;
            for (i = attr->vnums.begin( ); i != attr->vnums.end( ); i++) 
                if ((a <= i->minVnum && i->maxVnum <= b)
                    || (i->minVnum <= b && b <= i->maxVnum)
                    || (i->minVnum <= a && a <= i->maxVnum))
                {
                    return true;
                }
        }
                
    }

    return false;
}

/* returns corresponding area pointer for mob/room/obj vnum */
AREA_DATA *OLCState::get_vnum_area(int vnum)
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next)
        if (vnum >= pArea->min_vnum && vnum <= pArea->max_vnum)
            return pArea;
        
    return 0;
}

static const char * SEDIT_SYNTAX = ""
"{gВход во встроенный редактор. {y{hcq{hx{g - выход, {y{hch{hx{g - справка.\r\n";

bool
OLCState::sedit(DLString &original)
{
    if(inSedit.getValue( )) {
        ostringstream os;
        strEditor.lines.tostream(os);
        original = os.str( );
        inSedit.setValue( false );
        return true;
    } else {
        strEditor.clear( );
        strEditor.setBuffer( original );
        strEditor.clear_undo( );
        inSedit.setValue( true );

        if (owner && owner->character)
            owner->character->send_to(SEDIT_SYNTAX);

        return false;
    }
}

bool
OLCState::sedit(XMLString &original)
{
    DLString orig = original.getValue( );

    if(!sedit(orig))
        return false;
    
    original.setValue(orig);
    return true;
}

bool
OLCState::sedit(char *&original)
{
    DLString orig = original;
    
    if(!sedit(orig))
        return false;
    
    free_string(original);
    original = str_dup(orig.c_str( ));
    return true;
}

bool
OLCState::xmledit(XMLDocument::Pointer &xml)
{
    ostringstream os;
    if(xml)
        xml->save( os );
    
    DLString buf = os.str( );
    
    if(!sedit(buf))
        return false;

    try {
        XMLDocument::Pointer doc(NEW);
        istringstream is( buf );
        doc->load( is );
        xml = doc;
        return true;
    } catch(const exception &e ) {
        owner->send((DLString("xml parse error: ") + e.what( ) + "\r\n").c_str( ));
    }
    return false;
}

void
OLCState::seditDone( )
{
    if(!owner) {
        LogStream::sendError() << "olc: seditDone: no owner" << endl;
        return;
    }
    
    Character *ch = owner->character;
    if(!ch) {
        LogStream::sendError() << "olc: seditDone: no character" << endl;
        return;
    }

    PCharacter *pch = ch->getPC( );
    
    CommandBase::Pointer cmd;
    
    cmd = findCommand(pch, lastCmd.getValue( ).c_str( ));

    if(!cmd) {
        LogStream::sendError() << "olc: seditDone: command not found to repeat" << endl;
        return;
    }
    
    cmd->run(pch, lastArgs.getValue( ).c_str( ));

    if(inSedit.getValue( )) {
        LogStream::sendError() << "olc: seditDone: still in sedit after command repeat" << endl;
        inSedit.setValue( false );
        return;
    }
    strEditor.clear( );
}


bool OLCState::mapEdit( Properties &map, DLString &args )
{
    DLString arg1 = args.getOneArgument();
    DLString arg2 = args;

    Character *ch = owner->character;
    if(!ch) {
        LogStream::sendError() << "olc: mapEdit: no character" << endl;
        return false;
    }

    if (arg1.empty( )) {
        stc("Syntax: property <name> <value>\n\r"
            "        property <name>\n\r"
            "        property <name> clear\n\r", ch);
        return false;
    }

    if (arg2 == "clear") {
        Properties::iterator p = map.find( arg1 );
        if (p == map.end( )) {
            ptc(ch, "Property '%s' not found.\n\r", arg1.c_str( ));
        } else {
            map.erase( p );
            stc("Property cleared.\n\r", ch);
        }

        return false;
    }

    DLString &property = map[arg1];

    if (arg2.empty( )) {
        if (sedit(property)) {
            stc("Property set.\n\r", ch);
            return true;
        } else
            return false;
    }

    map[arg1] = arg2;
    stc("Property set.\n\r", ch);
    return false;
}

bool OLCState::flagBitsEdit(const FlagTable &table, Flags &field)
{
    int value = field.getValue();
    bool rc = flagBitsEdit(table, value);
    if (rc)
        field.setValue(value);
    return rc;
}

bool OLCState::flagBitsEdit(const FlagTable &table, int &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ptc(ch, "Использование:\r\n{W%s{x флаги - установить или снять указанные флаги\r\n{W? %s{x - показать таблицу флагов\r\n",
            cmd, cmd);
        return false;
    }

    bitstring_t value = table.bitstring(args);
    if (value == NO_FLAG) {
        ptc(ch, "Не найдено ни одного флага по строке '%s'. Используй '? %s' для таблицы всех флагов.\r\n",
             args.c_str(), cmd); 
        return false;
    }

    field ^= value;
    ptc(ch, "Новое значение поля {g%s{x:\r\n%s\r\n", cmd, table.names(field).c_str());
    return true;
}

bool OLCState::flagValueEdit(const FlagTable &table, Flags &field)
{
    int value = field.getValue();
    bool rc = flagValueEdit(table, value);
    if (rc)
        field.setValue(value);
    return rc;
}

bool OLCState::flagValueEdit(const FlagTable &table, int &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ptc(ch, "Использование:\r\n{W%s{x значение - установить значение\r\n{W? %s{x - показать таблицу возможных значений\r\n",
            cmd, cmd);
        return false;
    }

    int value = table.value(args);
    if (value == NO_FLAG) {
        ptc(ch, "Значение '%s' не найдено. Используй '? %s' для таблицы возможных значений.\r\n",
           cmd, cmd);
        return false;
    }

    field = value;
    ptc(ch, "Новое значение поля {g%s{x: %s (%s)\r\n", cmd, table.name(field).c_str(), table.message(field).c_str());
    return true;
}

bool OLCState::numberEdit(int minValue, int maxValue, int &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ptc(ch, "Использование:\r\n{W%s{x число - установить значение в диапазоне от %d до %d\r\n",
            cmd, minValue, maxValue);
        return false;
    }
    
    int value = atoi(args.getOneArgument().c_str());
    if (value < minValue || value > maxValue) {
        ptc(ch, "Значение должно лежать в диапазоне от %d до %d.\r\n", minValue, maxValue);
        return false;
    }

    field = value;
    ptc(ch, "Новое значение поля {g%s{x: %d\r\n", cmd, field);
    return true;
}

bool OLCState::numberEdit(long minValue, long maxValue, long &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ptc(ch, "Использование:\r\n{W%s{x число - установить значение в диапазоне от %ld до %ld\r\n",
            cmd, minValue, maxValue);
        return false;
    }
    
    long value = atol(args.getOneArgument().c_str());
    if (value < minValue || value > maxValue) {
        ptc(ch, "Значение должно лежать в диапазоне от %ld до %ld.\r\n", minValue, maxValue);
        return false;
    }

    field = value;
    ptc(ch, "Новое значение поля {g%s{x: %ld\r\n", cmd, field);
    return true;
}

bool OLCState::diceEdit(int *field)
{
    static char syntax[] = "Использование:\r\n{W%s{w число_бросков {Wd{w число_граней {W+{w бонус\r\n";
    char buf[MAX_STRING_LENGTH], *num, *type, *bonus, *cp;
    
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ptc(ch, syntax, cmd);
        return false;
    }

    strcpy(buf, args.c_str());
    num = cp = buf;

    while (isdigit(*cp))
        ++cp;
    while (*cp != '\0' && !isdigit(*cp))
        *(cp++) = '\0';

    type = cp;

    while (isdigit(*cp))
        ++cp;
    while (*cp != '\0' && !isdigit(*cp))
        *(cp++) = '\0';

    bonus = cp;

    while (isdigit(*cp))
        ++cp;
    if (*cp != '\0')
        *cp = '\0';

    if ((!is_number(num) || atoi(num) < 1)
        || (!is_number(type) || atoi(type) < 1)
        || (!is_number(bonus) || atoi(bonus) < 0)) {
        ptc(ch, syntax, cmd);
        return false;
    }

    field[DICE_NUMBER] = atoi(num);
    field[DICE_TYPE] = atoi(type);
    field[DICE_BONUS] = atoi(bonus);
    int ave = field[DICE_BONUS] + (field[DICE_TYPE]+1)*field[DICE_NUMBER]/2;

    ptc(ch, "Полю {g%{x установлено значение {W%dd%d+%d{x, среднее {W%d{x.\r\n", 
        cmd, field[DICE_NUMBER], field[DICE_TYPE], field[DICE_BONUS], ave);
    return true;
}

bool OLCState::editorCopy(const char *field)
{
    DLString source = field ? field : DLString::emptyString;
    PCharacter *ch = owner->character->getPC();
    ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].split(source);
    ptc(ch, "Описание скопировано в буфер.\r\n");
    return false;
}

bool OLCState::editorCopy(const DLString &original)
{
    return editorCopy(original.c_str());
}

static void apply_flags(DLString &original, editor_flags flags)
{
    if (IS_SET(flags, ED_UPPER_FIRST_CHAR)) {
        original.upperFirstCharacter();
    }

    if (IS_SET(flags, ED_NO_NEWLINE)) {
        original.erase( 
            original.find_last_not_of('\r') + 1);
        original.erase( 
            original.find_last_not_of('\n') + 1);
    }

    if (IS_SET(flags, ED_ADD_NEWLINE)) {
        if (original.empty() || original.at(original.size() - 1) != '\n')
            original << "\n";
    }
}

bool OLCState::editorPaste(DLString &original, editor_flags flags)
{
    PCharacter *ch = owner->character->getPC();
    original = ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].dump( );
    apply_flags(original, flags);
    ptc(ch, "Описание вставлено из буфера.\r\n");
    return true;
}

bool OLCState::editorPaste(char *&field, editor_flags flags)
{
    DLString original = field ? field : DLString::emptyString;
    editorPaste(original, flags);

    if (field)
        free_string(field);

    field = str_dup(original.c_str());
    return true;
}

bool OLCState::editor(const char *argument, char *&field, editor_flags flags)
{
    DLString original = field;
    
    if (!editor(argument, original, flags))
        return false;
    
    free_string(field);
    field = str_dup(original.c_str());
    return true;
}

/**
 * Runs 'webedit' for the provided string, remembering invoked command ('desc', 'short' etc)
 * inside an attribute. '<desc> paste' will be done from the attribute's event handler once 
 * webedit has finished.
 */
bool OLCState::editorWeb(const DLString &original, const DLString &saveCommand, editor_flags flags)
{
    PCharacter *ch = owner->character->getPC();

    editorCopy(original);

    XMLAttributeOLC::Pointer attr = ch->getAttributes().getAttr<XMLAttributeOLC>("olc");
    attr->saveCommand.setValue(saveCommand);

    if (IS_SET(flags, ED_HELP_HINTS))
        interpret_raw(ch, "webedit", "help");
    else
        interpret_raw(ch, "webedit");
        
    return false;
}

bool OLCState::editor(const char *argument, DLString &original, editor_flags flags)
{
    PCharacter *ch = owner->character->getPC();
    DLString args = argument;
    DLString command = args.getOneArgument();
    const char *cmd = lastCmd.c_str();

    if (command.empty()) {
        if (!sedit(original)) 
            return false;
        
        apply_flags(original, flags);
        stc("Описание установлено.\n\r", ch);
        return true;
    }

    if (arg_is_copy(command)) 
        return editorCopy(original);

    if (arg_is_paste(command))
        return editorPaste(original, flags);

    if (arg_is_web(command))
        return editorWeb(original, lastCmd + " paste", flags);

    if (arg_is_help(command)) {
        stc("Команды редактора:\n\r", ch);
        ptc(ch, "%s        : войти во встроенный редактор описаний\n\r", cmd);
        ptc(ch, "%s web    : отредактировать описание в вебредакторе\n\r", cmd);
        ptc(ch, "%s copy   : скопировать описание в буфер\n\r", cmd);
        ptc(ch, "%s paste  : заменить описание на то, что в буфере\n\r", cmd);
        ptc(ch, "%s строка : заменить описание на строку\r\n", cmd);
        ptc(ch, "%s ?      : вывести эту подсказку\r\n", cmd);
        return false;
    }

    original = argument;
    apply_flags(original, flags);
    ptc(ch, "Описание установлено в строку %s\r\n", original.c_str());
    return true;
}

static EXTRA_DESCR_DATA *safe_extra_descr(PCharacter *ch, const char *keyword, EXTRA_DESCR_DATA *&list)
{
    EXTRA_DESCR_DATA *ed;

    for (ed = list; ed; ed = ed->next)
        if (is_name(keyword, ed->keyword))
            break;

    if (!ed) {
        ed = new_extra_descr();
        ed->keyword = str_dup(keyword);
        ed->description = str_empty;
        ed->next = list;
        list = ed;
        ptc(ch, "Создано новое экстра-описание [%s].\r\n", keyword);
    }

    return ed;
}

bool OLCState::extraDescrEdit(EXTRA_DESCR_DATA *&list)
{
    char buf[MAX_STRING_LENGTH];
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    char command[MAX_INPUT_LENGTH];
    char *keyword;

    strcpy(buf, lastArgs.c_str());
    keyword = one_argument(buf, command);

    if (!*command || !*keyword) {
        ptc(ch, "Синтаксис:\r\n%s set [keyword]    - войти во встроенный редактор экстра-описания\n\r", cmd);
        ptc(ch, "%s web [keyword]    - отредактировать экстра-описание в вебредакторе\n\r", cmd);        
        ptc(ch, "%s copy [keyword]   - скопировать экстра-описание в буфер\n\r", cmd);
        ptc(ch, "%s paste [keyword]  - установить экстра-описание из буфера\n\r", cmd);
        ptc(ch, "%s delete [keyword] - удалить экстра-описание\n\r", cmd);
        return false;
    }

    if (arg_is_copy(command)) {
        return editorCopy(
                    safe_extra_descr(ch, keyword, list)->description);
    }

    if (arg_is_paste(command)) {
        return editorPaste(
                    safe_extra_descr(ch, keyword, list)->description);
    }

    if (arg_is_web(command)) {
        return editorWeb(
                    safe_extra_descr(ch, keyword, list)->description,
                    lastCmd + " paste " + keyword);
    }

    if (is_name(command, "set")) {
        return sedit(
                    safe_extra_descr(ch, keyword, list)->description);
    }

    if (is_name(command, "delete")) {
        EXTRA_DESCR_DATA *ed;
        EXTRA_DESCR_DATA *ped = NULL;

        for (ed = list; ed; ed = ed->next) {
            if (is_name(keyword, ed->keyword))
                break;
            ped = ed;
        }
    
        if (!ed) {
            stc("Экстра-описание с таким ключом не найдено.\n\r", ch);
            return false;
        }

        if (!ped)
            list = ed->next;
        else
            ped->next = ed->next;

        free_extra_descr(ed);

        stc("Экстра-описание удалено.\n\r", ch);
        return true;
    }

    findCommand(ch, cmd)->run(ch, "");
    return false;
}

DLString web_edit_button(bool showWeb, Character *ch, const DLString &editor, const DLString &args)
{
    if (showWeb)
        return web_edit_button(ch, editor, args);
    else
        return DLString::emptyString;
}