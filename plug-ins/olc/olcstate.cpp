/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "json_utils.h"
#include "xmljsonvalue.h"
#include "olcstate.h"
#include "olc.h"
#include "sedit.h"
#include "pcharacter.h"
#include "behavior.h"
#include "security.h"
#include "interp.h"
#include "arg_utils.h"
#include "websocketrpc.h"


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

int OLCCommand::properOrder( Character * ) const
{
    return RC_ORDER_ERROR;
}

int OLCCommand::dispatch( const InterpretArguments &iargs )
{
    return RC_DISPATCH_OK;
}

int OLCCommand::dispatchOrder( const InterpretArguments &iargs )
{
    return RC_DISPATCH_NOT_HERE;
}

void OLCCommand::entryPoint( Character *ch, const DLString &cArguments )
{
    // Main method called from command interpreter
    
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

bool OLCState::can_edit( Character *ch, AreaIndexData *pArea )
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
AreaIndexData *OLCState::get_vnum_area(int vnum)
{
    for(auto &pArea: areaIndexes)
        if (vnum >= pArea->min_vnum && vnum <= pArea->max_vnum)
            return pArea;
        
    return 0;
}

static const char * SEDIT_SYNTAX = ""
"{gВход во встроенный редактор. {y{hcq{hx{g - выход, {y{hch{hx{g - справка.{x\r\n";

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

        if (!doc->getFirstNode())
            throw Exception("empty root node. Use 'behavior clear' instead.");

        xml = doc;
        return true;
    } catch (const exception &e) {
        owner->send((DLString("XML parse error: ") + e.what( ) + "\r\n").c_str( ));
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
    
    cmd->entryPoint(pch, lastArgs.getValue( ).c_str( ));

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

bool OLCState::enumerationArrayEdit(EnumerationArray &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;
    DLString valueName = args.getOneArgument();
    const FlagTable *table = field.getTable();
    int index = table->value(valueName);
    Integer value;

    
    if (index == NO_FLAG || !Integer::tryParse(value, args)) {
        ptc(ch, "Формат: %s <param name> <число>\r\n", cmd);
        ptc(ch, "        %s <param name> 0\r\n", cmd);
        return false;
    }

    field[index] = value;
    ptc(ch, "Поле %s установлено в %d.\r\n", table->fields[index].name, value);
    return true;
}

bool OLCState::flagBitsEdit(Flags &field)
{
    return flagBitsEdit(*field.getTable(), field);
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
    DLString tabname = FlagTableRegistry::getName(&table);

    if (args.empty()) {
        ptc(ch, "Использование:\r\n{y%s{x флаги - установить или снять указанные флаги\r\n{y{hc? %s{x - показать таблицу флагов\r\n",
            cmd, tabname.c_str());
        return false;
    }

    bitstring_t value = table.bitstring(args);
    if (value == NO_FLAG) {
        ptc(ch, "Не найдено ни одного флага по строке '%s'. Используй {y{hc? %s{x для таблицы всех флагов.\r\n",
             args.c_str(), tabname.c_str()); 
        return false;
    }

    field ^= value;
    ptc(ch, "Новое значение поля {g%s{x:\r\n%s\r\n", cmd, table.names(field).c_str());
    return true;
}

bool OLCState::flagValueEdit(Enumeration &field)
{
    return flagValueEdit(*field.getTable(), field);
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
    DLString tabname = FlagTableRegistry::getName(&table);

    if (args.empty()) {
        ptc(ch, "Использование:\r\n{y%s{x значение - установить значение\r\n{y{hc? %s{x - показать таблицу возможных значений\r\n",
            cmd, tabname.c_str());
        return false;
    }

    int value = table.value(args);
    if (value == NO_FLAG) {
        ptc(ch, "Значение '%s' не найдено. Используй {y{hc? %s{x для таблицы возможных значений.\r\n",
           cmd, tabname.c_str());
        return false;
    }

    field = value;
    ptc(ch, "Новое значение поля {g%s{x: %s (%s)\r\n", cmd, table.name(field).c_str(), table.message(field).c_str());
    return true;
}

bool OLCState::genderEdit(XMLString &field)
{
    PCharacter *ch = owner->character->getPC();
    DLString args = lastArgs;

    if (args.empty()) {
        stc("Syntax:  gender m|f|n|p\n\r", ch);
        return false;
    }

    field = Grammar::MultiGender(args.getOneArgument().c_str()).toString();

    ptc(ch, "Grammatical gender set to '%s'.\n\r", field.c_str());
    return true;
}

bool OLCState::genderEdit(Grammar::MultiGender &field)
{
    PCharacter *ch = owner->character->getPC();
    DLString args = lastArgs;

    if (args.empty()) {
        stc("Syntax:  gender m|f|n|p\n\r", ch);
        return false;
    }

    field = Grammar::MultiGender(args.getOneArgument().c_str());

    ptc(ch, "Grammatical gender set to '%s'.\n\r", field.toString());
    return true;
}

bool OLCState::numberEdit(int minValue, int maxValue, int &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;

    if (args.empty()) {
        ptc(ch, "Использование:\r\n{y%s{x число - установить значение в диапазоне от %d до %d\r\n",
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
        ptc(ch, "Использование:\r\n{y%s{x число - установить значение в диапазоне от %ld до %ld\r\n",
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

bool OLCState::rangeEdit(int minValue, int maxValue, int &field1, int &field2)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;
    DLString arg1 = args.getOneArgument();
    DLString arg2 = args.getOneArgument();
    Integer value1, value2;

    if (!Integer::tryParse(value1, arg1) || !Integer::tryParse(value2, arg2)) {
        ptc(ch, "Использование:\r\n{y%s{x число1 число2 - установить два значения. каждое в диапазоне от %d до %d\r\n",
             cmd, minValue, maxValue);        
        return false;
    }

    if (maxValue < value1 || maxValue < value2) {
        ptc(ch, "Значения должны быть не больше %d.\r\n", maxValue);
        return false;
    }

    if (minValue > value1 || minValue > value2) {
        ptc(ch, "Значения должны быть не меньше %d.\r\n", minValue);
        return false;
    }

    if (value1 > value2) {
        stc("Первое число должно быть меньше или равно второму.\r\n", ch);
        return false;
    }

    field1 = value1;
    field2 = value2;
    ptc(ch, "Новый диапазон %s: %d .. %d\r\n", cmd, field1, field2);
    return true;
}

bool OLCState::boolEdit(bool &field)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;
    DLString arg = args.getOneArgument();

    if (arg_is_yes(arg))
        field = true;
    else if (arg_is_no(arg))
        field = false;
    else {
        ptc(ch, "Использование:\r\n{y{hc%s y{x и {y{hc%s n{x - установить значение переключателя\r\n", cmd, cmd);
        return false;
    }

    ptc(ch, "Новое значение поля {g%s{x: %s\r\n", cmd, field ? "yes" : "no");
    return true;
}

bool OLCState::diceEdit(int *field)
{
    static char syntax[] = "Использование:\r\n{y%s{w число_бросков {Wd{w число_граней {W+{w бонус\r\n";
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

bool OLCState::enumerationArrayWebEdit(EnumerationArray &values)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;
    DLString arg = args.getOneArgument();
    const FlagTable *table = values.getTable();

    if (arg.empty()) {
        // Launch web editor.
        editorWeb(show_enum_array_web(values), lastCmd +" paste");
        return false;
    }

    if (arg_is_help(arg)) {
        // Show usage.
        stc("Использование:\r\n", ch);
        ptc(ch, "    %s - запустить веб-редактор массива значений\r\n", cmd);
        ptc(ch, "    %s paste - установить значения из буфера веб-редактора\r\n", cmd);
        ptc(ch, "    %s <string> - установить значения из строки\r\n", cmd);
        return false;
    }

    DLString newValue;
    if (arg_is_paste(arg)) {
        // Grab value from the editor buffer.
        editorPaste(newValue, ED_NO_NEWLINE);
    } else {
        // Grab value from command argument.
        newValue = lastArgs;
    }

    // Split values string by comma, receiving name+value pairs
    StringList pairs;
    pairs.split(newValue, ",");
    for (auto &pair: pairs) {
        DLString name = pair.getOneArgument();
        DLString value = pair.getOneArgument();        

        // Do a strict search inside a flag table for provided flag name.
        int index = table->index(name, true);
        if (index == NO_FLAG) {
            ptc(ch, "Вхождение %s не найдено в таблице %s.\r\n", 
                name.c_str(), values.getTableName().c_str());
            return false;
        }

        // Convert provided flag value to an integer.
        Integer v;
        if (!Integer::tryParse(v, value)) {
            ptc(ch, "Неверное числовое значение %s для %s.\r\n",
                  value.c_str(), name.c_str());
            return false;
        }

        values[index] = v;
    }

    return true;
}

bool OLCState::stringListEdit(XMLStringList &values)
{
    PCharacter *ch = owner->character->getPC();
    const char *cmd = lastCmd.c_str();
    DLString args = lastArgs;
    DLString arg = args.getOneArgument();

    if (arg.empty()) {
        // Launch web editor.
        editorWeb(values.toList().toString(), lastCmd +" paste");
        return false;
    }

    if (arg_is_help(arg)) {
        // Show usage.
        stc("Использование:\r\n", ch);
        ptc(ch, "    %s - запустить веб-редактор синонимов\r\n", cmd);
        ptc(ch, "    %s paste - установить синонимы из буфера веб-редактора\r\n", cmd);
        ptc(ch, "    %s <string> - установить синонимы из строки\r\n", cmd);
        return false;
    }

    DLString newValue;
    if (arg_is_paste(arg)) {
        // Grab value from the editor buffer.
        editorPaste(newValue, ED_NO_NEWLINE);
    } else {
        // Grab value from command argument.
        newValue = lastArgs;
    }

    // Try to assign new aliases from a space-separated string.
    StringList newAliases(newValue);
    if (newAliases.empty()) {
        values.clear();
        stc("Все синонимы очищены.\r\n", ch);
    } else {
        values.clear();
        for (auto &a: newAliases)
            values.push_back(a);
        ptc(ch, "Установлены новые синонимы: %s\r\n", values.toList().toString().c_str());
    }

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

    if (IS_SET(flags, ED_UPPERCASE))
        original.toUpper();

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

bool OLCState::editor(const char *argument, RussianString &russian, editor_flags flags)
{
    DLString original = russian.getFullForm();
    
    if (!editor(argument, original, flags))
        return false;
    
    russian.setFullForm(original.c_str());
    return true;
}

bool OLCState::editor(const char *argument, XMLStringList &values, editor_flags flags)
{
    DLString original = values.toList().join("\n");

    if (!editor(argument, original, flags))
        return false;

    StringList newLines;
    newLines.split(original, "\n");
    values.clear();
    for (auto &line: newLines)
        values.push_back(line);

    return true;
}

bool OLCState::editor(const char *argument, XMLJsonValue &value, editor_flags flags)
{
    DLString text = JsonUtils::toString(value);
    PCharacter *ch = owner->character->getPC();

    if (!editor(argument, text, flags))
        return false;

    ostringstream errbuf;

    if (!JsonUtils::validate(text, errbuf)) {
        ch->pecho("Ошибка парсинга JSON:");
        ch->pecho(errbuf.str());
        ch->pecho("Отредактируйте текст еще раз ({y{hc%s web{x).", lastCmd.c_str());
        return false;
    }

    JsonUtils::fromString(text, value);

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
    else if (IS_SET(flags, ED_JSON))
        interpret_raw(ch, "webedit", "json");
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

    if (arg_oneof_strict(command, "pastedone")) {
        bool rc = editorPaste(original, flags);
        if (rc)
            handle(ch->desc, "done");
        return rc;
    }

    if (arg_is_paste(command))
        return editorPaste(original, flags);

    if (arg_is_web(command)) {
        DLString pasteCmd = IS_SET(flags, ED_CALL_DONE) ? "pastedone" : "paste";
        return editorWeb(original, lastCmd + " " + pasteCmd, flags);
    }

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

    findCommand(ch, cmd)->entryPoint(ch, "");
    return false;
}

bool OLCState::editBehaviors(GlobalBitvector &behaviors, Json::Value &props)
{
    // Remember old behavior.
    std::set<int> oldBehaviors = behaviors.toSet();
    
    bool rc = globalBitvectorEdit<Behavior>(behaviors);

    if (!rc)
        return false;

    // For all entries that used to be available but no longer there,
    // clean the entry in the props map.
    for (auto b: oldBehaviors) {
        if (!behaviors.isSet(b)) {
            Behavior *bhv = behaviorManager->find(b);
            props.removeMember(bhv->getName().c_str());
        }
    }

    // For all entries that are new, create a value in the props map
    // and copy the defaults.
    std::set<int> newBehaviors = behaviors.toSet();
    for (auto b: newBehaviors) {
        if (oldBehaviors.count(b) == 0) {
            Behavior *bhv = behaviorManager->find(b);
            props[bhv->getName()] = bhv->props;
        }
    }

    return true;
}

bool OLCState::editProps(GlobalBitvector &behaviors, Json::Value &props, const DLString &arguments)
{
    DLString args = arguments;
    DLString bhvName = args.getOneArgument();
    DLString propName = args.getOneArgument();
    DLString propValue = args;
    Character *ch = owner->character;

    if (bhvName.empty() || propName.empty() || propValue.empty()) {
        ptc(ch, "Использование: prop <имя поведения> <свойство> <значение>\r\n");
        return false;
    }

    Behavior *bhv = behaviorManager->findExisting(bhvName);
    if (!bhv) {
        ptc(ch, "Поведение '%s' не существует, смотри {y{hc? behaviors{x для списка.\r\n", bhvName.c_str());
        return false;
    }

    if (!behaviors.isSet(bhv->getIndex())) {
        ptc(ch, "Поведение '%s' не установлено на этом предмете.\r\n", bhvName.c_str());
        return false;
    }

    if (!props[bhvName].isMember(propName)) {
        ptc(ch, "У поведения '%s' нету свойства под названием '%s'.\r\n", bhvName.c_str(), propName.c_str());
        return false;
    }

    Json::Value &target = props[bhvName][propName];

    if (target.isNull())
        target = propValue;
    else if (target.isNumeric() || target.isBool()) {
        if (!propValue.isNumber()) {
            ptc(ch, "Свойство '%s' должно быть числом, а не строкой.\r\n", propName.c_str());
            return false;
        }

        target = propValue.toInt();
    } else {
        target = propValue;
    }

    ptc(ch, "Свойству %s.%s установлено значение %s.\r\n", bhvName.c_str(), propName.c_str(), propValue.c_str());
    return true;
}

DLString web_edit_button(bool showWeb, Character *ch, const DLString &editor, const DLString &args)
{
    if (showWeb)
        return web_edit_button(ch, editor, args);
    else
        return DLString::emptyString;
}
