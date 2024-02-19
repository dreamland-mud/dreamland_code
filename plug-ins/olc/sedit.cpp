/* $Id$
 *
 * ruffina, 2004
 */

#include "colour.h"
#include "sedit.h"
#include "interp.h"
#include "dlstring.h"
#include "pcharacter.h"

#include "olcstate.h"

OLCStringEditor::OLCStringEditor(OLCState &s) : olc(s)
{
}

void
OLCStringEditor::done( )
{
    olc.seditDone( );
}

Descriptor *
OLCStringEditor::getOwner( )
{
    /*XXX - throw somth?*/
    return olc.owner;
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

void OLCStringEditor::help()
{
    Character *ch = getOwner( )->character;
    if (ch)
        ch->send_to(ED_SYNTAX);
}

