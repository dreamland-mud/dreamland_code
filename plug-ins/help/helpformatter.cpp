/* $Id$
 *
 * ruffina, 2004
 */
#include "helpformatter.h"
#include "character.h"

HelpFormatter::HelpFormatter( )
                : fParse( true ), text( 0 )
{
}

HelpFormatter::~HelpFormatter( )
{
}

void HelpFormatter::reset( )
{
    fParse = true;
}

void HelpFormatter::setup( Character * )
{
}

// *...*     ->  {y...{w                      (bold text)
// _..._     ->  {D<{w...{D>{w                (italic with <>)
// =...=     ->  {c...{w                      (header text)
// (eng,рус) ->  рус              (language choice)
// [...]     ->  {W...{w                      (reference)
// %KEYWORD%
void HelpFormatter::run( Character *ch, ostringstream &out )
{
    bool t_asterix = false;
    bool t_underline = false;
    bool t_equals = false;
    bool t_ref = false;
    bool t_bracket = false;
    
    reset( );

    if (!text)
        return;

    setup( ch );

    for (const char *p = text; *p; ++p) {
        if (*p == '%') {
            DLString kw;

            while (*++p && *p >= 'A' && *p <= 'Z')
                kw << *p;
            
            if (*p == '%') {
                if (handleKeyword( kw, out ))
                    continue;
            }

            out << "%" << kw << (*p ? *p : '\0');
            continue;
        }

        if (!fParse) {
            out << *p;
            continue;
        }
        
        switch (*p) {    
        case '*':
            t_asterix = !t_asterix;
            t_bracket = false;
            out << (t_asterix ? "{y" : "{w");
            break;

        case '_':
            t_underline = !t_underline;
            t_bracket = false;
            out << (t_underline ? "{D" : "{w");
            break;

        case '=':
            t_equals = !t_equals;
            t_bracket = false;
            out << (t_equals ? "{c" : "{w");
            break;

        case '[':
            if (!t_ref) {
                t_ref = true;
                t_bracket = false;
                out << "{W{hh";
            } else
                out << *p;
            break;
            
        case ']':
            if (t_ref) {
                t_ref = false;
                t_bracket = false;
                out << "{x";
            } else
                out << *p;
            break;
        
        case '(':
            if (t_ref || t_equals || t_underline || t_asterix) {
                out << "{lE";
                t_bracket = true;
            } else
                out << *p;
            break;

        case ',':
            if (t_bracket) 
                out << "{lR";
            else
                out << *p;
            break;

        case ')':
            if (t_bracket) {
                t_bracket = false;
                out << "{lx";
            } else
                out << *p;
            break;
        
        default:
            out << *p;
            break;
        }
    }
    
}

// %KEYWORD%:
// H, HELP     ->  {Wсправка{w
// SA, SEEALSO ->  См. также
// U, USAGE    ->  Использование
// FMT         ->  {wФормат:{w
// FFF         ->  {w       {w
// PAUSE       ->  stops help tag parsing
// RESUME      ->  resumes help tag parsing
// A           ->  * 
// CAST        ->  колдовать
// OBJ         ->  предмет
// CHAR        ->  персонаж
// PLR         ->  игрок
// MOB         ->  монстр
// VICT        ->  жертва
// DIR         ->  направление
// YES, NO     ->  да, нет
// ALL         ->  все
// SHOW        ->  показ
// ON          ->  вкл
// OFF         ->  выкл
bool HelpFormatter::handleKeyword( const DLString &kw, ostringstream &out )
{
    if (kw == "H" || kw == "HELP") {
        out << "{Wсправка{w";
        return true;
    }

    if (kw == "A") {
        out << "*";
        return true;
    }

    if (kw == "FMT") {
        out << "{wФормат:{w";
        return true;
    }

    if (kw == "FFF") {
        out << "{w       {w";
        return true;
    }

    if (kw == "U" || kw == "USAGE") {
        out << "Использование";
        return true;
    }

    if (kw == "SA" || kw == "SEEALSO") {
        out << "См. также";
        return true;
    }

    if (kw == "CAST") {
        out << "колдовать";
        return true;
    }

    if (kw == "OBJ") {
        out << "предмет";
        return true;
    }

    if (kw == "CHAR") {
        out << "персонаж";
        return true;
    }

    if (kw == "PLR") {
        out << "игрок";
        return true;
    }

    if (kw == "MOB") {
        out << "монстр";
        return true;
    }


    if (kw == "VICT") {
        out << "жертва";
        return true;
    }

    if (kw == "DIR") {
        out << "направление";
        return true;
    }

    if (kw == "YES") {
        out << "да";
        return true;
    }

    if (kw == "NO") {
        out << "нет";
        return true;
    }

    if (kw == "ALL") {
        out << "все";
        return true;
    }

    if (kw == "SHOW") {
        out << "показ";
        return true;
    }

    if (kw == "ON") {
        out << "вкл";
        return true;
    }

    if (kw == "OFF") {
        out << "выкл";
        return true;
    }

    if (kw == "PAUSE") {
        fParse = false;
        return true;
    }
    
    if (kw == "RESUME") {
        fParse = true;
        return true;
    }

    return false;
}

DefaultHelpFormatter::DefaultHelpFormatter( const char *text )
{
    this->text = text;
}

DefaultHelpFormatter::~DefaultHelpFormatter( )
{
}

