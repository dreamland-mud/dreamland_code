/* $Id$
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "interp.h"
#include "mudtags.h"

#include "xmlpcstringeditor.h"
#include "format.h"

void 
XMLPCStringEditor::print(const std::string &s)
{
    getOwner( )->send((s + "\r\n").c_str( ));
}

void
XMLPCStringEditor::error(const std::string &s)
{
    getOwner( )->send(("error: " + s + "\r\n").c_str( ));
}

XMLPCStringEditor::reg_t &
XMLPCStringEditor::registerAt(char r)
{
    Character *ch = getOwner( )->character;
    if(!ch) {
        LogStream::sendError( ) << "XMLPCStringEditor::registerAt: no character" << endl;
        return defaultReg;
    }

    PCharacter *pch = ch->getPC( );
    if(!pch) {
        LogStream::sendError( ) << "XMLPCStringEditor::registerAt: no pcharacter" << endl;
        return defaultReg;
    }

    ::Pointer<XMLAttributeEditorState> er;
    er = pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate");

    return er->regs[r];
}

static const char * ED_SYNTAX = ""
"Полезные команды:\r\n"
"    {y{hcq{x - выход из редактора\r\n"
"    {y+1{x - следующая строка, {y-1{x - предыдущая строка, {y2{x - вторая строка\r\n"
"    {y{hc%p{x - показать весь текст, {y{hcp{x - показать текущую строку\r\n"
"    {y{hc%!show{x - показать в цвете\r\n"
"    {y%s/да/нет/g{x - заменить все вхождения, {ys/да/нет/g{x - заменить все в текущей строке\r\n"
"    {y%a{x и в конце {y.{x - добавить текст в конец\r\n"
"    {y%!format 0 79{x - формат по ширине 79 с отступом 0\r\n"
"    {yd{x - удалить текущую строку\r\n"
"    {y{hcu{x - отменить действие (undo)\r\n"
"Полное руководство {hlhttps://github.com/dreamland-mud/dreamland_code/wiki/ED{x.\r\n";

string 
XMLPCStringEditor::shell(const string &acmd, const string &text)
{
    Character *ch = getOwner( )->character;

    if(!ch)
        return "!no character!";
    
    DLString argumet(acmd), cmd;
    
    cmd = argumet.getOneArgument( );
        
    if(cmd.strPrefix("format") || cmd.strPrefix("justify")) {
        istringstream is(text);
        ostringstream os;
        int tab = 8, width = 70;
        DLString stab = argumet.getOneArgument( ), 
                 swidth = argumet.getOneArgument( );

        if(!stab.empty( ) && stab.isNumber( ))
            tab = stab.toInt( );

        if(!swidth.empty( ) && swidth.isNumber( ))
            width = swidth.toInt( );
        
        Formatter fmt(os, is);
        fmt.format(tab, width);

        return os.str( );
        
    } else if(cmd.strPrefix("show") || cmd.strPrefix("print")) {
        ostringstream os;
        mudtags_convert( text.c_str( ), os );
        getOwner( )->send(os.str( ).c_str( ));
    } else if (cmd.strPrefix("help")) {
        ch->send_to(ED_SYNTAX);    
    } else {
        interpret(ch, cmd.c_str( ));
    }

    return text;
}

void 
XMLPCStringEditor::prompt( Descriptor *d )
{
    if(append_at)
        d->send( "+ " );
    else
        d->send( ": " );
}

bool 
XMLAttributeEditorState::handle( const WebEditorSaveArguments &args )
{
    regs[0].split(args.text);
    return true;
}
