/* $Id: feniastringext.cpp,v 1.1.2.8.6.14 2009/10/11 21:41:08 rufina Exp $
 *
 * ruffina, 2004
 */
#include <string.h>

#include "register-impl.h"
#include "reglist.h"
#include "morphology.h"
#include "char.h"
#include "dl_strings.h"
#include "dl_ctype.h"
#include "stringlist.h"
#include "format.h"
#include "mudtags.h"
#include "regexp.h"
#include "nativeext.h"
#include "fenia/exceptions.h"
#include "wrap_utils.h"
#include "feniastring.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "reglist.h"
#include "def.h"

namespace Scripting {

NMI_INVOKE(FeniaString, size, "(): длина строки")
{
    return (int)size();
}

NMI_INVOKE(FeniaString, capitalize, "(): перевести 1й символ в верхний регистр, остальные в нижний")
{
    DLString str = *this;
    str.capitalize();
    return Register( str );
}

NMI_INVOKE(FeniaString, trim, "(): обрезать лишние пробелы по бокам")
{
    DLString str = *this;
    str.stripWhiteSpace();
    return Register( str );
}

NMI_INVOKE(FeniaString, noun, "(): попытаться найти одно существительное в строке")
{
    return Register(Syntax::noun(*this));
}

NMI_INVOKE(FeniaString, quote, "(): вернуть строку в кавычках, если в ней есть пробелы")
{
    return Register(quote());
}

NMI_INVOKE(FeniaString, strPrefix, "(str): true если эта строка - префикс str")
{
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );
    
    if (empty( ))
        return false;
    return strPrefix( args.front( ).toString( ) );
}

NMI_INVOKE(FeniaString, ruscase, "(case): строка в указанном падеже")
{
    int cse = 1;
    
    if (!args.empty())
        cse = args.front().toNumber( );

    return Register( ruscase( cse + '0' ) );
}

NMI_INVOKE(FeniaString, getOneArgument, "(): возвращает первое слово или первую фразу в кавычках")
{
    DLString str = *this;
    return str.getOneArgument( );
}

NMI_INVOKE(FeniaString, cutOneArgument, "(): возвращает строку без первого слова (или без первой фразы в кавычках)")
{
    DLString str = *this;
    str.getOneArgument( );
    return str;
}

NMI_INVOKE(FeniaString, arguments, "(): разбивает строку на список аргументов (слова или фразы в кавычках)") 
{
    RegList::Pointer arguments(NEW);
    DLString str = *this;

    while (!str.empty()) {
        arguments->push_back(
            Register(str.getOneArgument()));
    }

    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(arguments);
    return Register(sobj);
}


NMI_INVOKE(FeniaString, upperFirstChar, "(): возвращает ту же строку, но с большой буквы")
{
    DLString str = *this;
    str.upperFirstCharacter();
    return Register( str );
}

NMI_INVOKE(FeniaString, matchAndReplace, "(pattern, str): заменяет в данной строке все вхождения $1..$9 на соответствующие совпадения") 
{
    if (args.size() != 2)
        throw NotEnoughArgumentsException();

    DLString pattern = args.front().toString();
    DLString str = args.back().toString();
    DLString result = *this;
    
    try {
        RegExp re( pattern.c_str() );
        RegExp::MatchVector mv;
        
        mv = re.subexpr( str.c_str() );
        if (mv.size() == 0)
            return result;

        for (int i = mv.size() - 1; i >= 0; i--) {
            DLString num;
            num << "$" << i + 1;
            result.replaces(num, mv[i]);            
        }
        
    } catch( const std::exception &e ) {
        throw Exception(e.what());
    }

    return result;
}

NMI_INVOKE(FeniaString, matchGroups, "(pattern): возвращает список (List) из всех групп шаблона") 
{
    if (args.size() != 1)
        throw NotEnoughArgumentsException();

    DLString pattern = args.front().toString();
    RegList::Pointer groups(NEW);
    
    try {
        RegExp re( pattern.c_str() );
        RegExp::MatchVector mv;
        
        mv = re.subexpr( this->c_str() );

        for (int i = mv.size() - 1; i >= 0; i--) {
            groups->push_back( Register( mv[i] ) );
        }
        
    } catch( const std::exception &e ) {
        throw Exception(e.what());
    }

    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(groups);
    return Register(sobj);
}

NMI_INVOKE(FeniaString, match, "(regex): true если строка соответствует этому регулярному выражению")
{
    if (args.empty())
        throw NotEnoughArgumentsException();
    
    try {
        RegExp re( args.front().toString().c_str() );

        return re.match( c_str() );
    } catch( const std::exception &e ) {
        throw Exception(e.what());
    }
}

NMI_INVOKE(FeniaString, matchCase, "(regex): true если строка соответствует этому регулярному выражению с учетом регистра")
{
    if (args.empty())
        throw NotEnoughArgumentsException();
        
    try {
        RegExp re( args.front().toString().c_str(), true );

        return re.match( c_str() );
    } catch( const std::exception &e ) {
        throw IllegalArgumentException();
    }
}

NMI_INVOKE(FeniaString, isName, "(arg): является ли слова из arg одними из полных слов в строке")
{
    if (args.empty())
        throw NotEnoughArgumentsException();
        
    return isName( args.front().toString().c_str() );
}

NMI_INVOKE(FeniaString, is_name, "(arg): являются ли слова из arg подсловами в строке")
{
    if (args.empty())
        throw NotEnoughArgumentsException();
        
    const DLString &a = args.front().toString();
    char l[size()+1], r[a.size()+1];

    ::copy(begin(), end(), l);
    l[size()] = 0;

    ::copy(a.begin(), a.end(), r);
    r[a.size()] = 0;
    
    return is_name( l, r );
}

NMI_INVOKE(FeniaString, substr, "(offset[,length]): возвращает подстроку начинающуюся с offset длины length или до конца")
{
    if (args.empty())
        throw NotEnoughArgumentsException();
        
    try {
        RegisterList::const_iterator iter;
        size_type i1, i2 = std::string::npos;

        iter = args.begin();
        i1 = iter->toNumber();

        if (++iter != args.end())
            i2 = iter->toNumber();
        
        return substr( i1, i2 );
    } catch( const std::exception &e ) {
        throw IllegalArgumentException();
    }
}

NMI_INVOKE(FeniaString, stripColour, "(): удаляет все символы цвета")
{
    DLString rc;
    
    for(iterator i = begin(); i != end();) {
        char c = *i++;
        
        if(c == '{') {
            if(*i++ == '{')
                rc += '{';
        } else
            rc += c;
    }
    
    return rc;
}

NMI_INVOKE(FeniaString, stripTags, "(): удаляет все специальные теги и цвета")
{
    ostringstream buf;
    mudtags_convert(c_str(), buf, TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR);
    return buf.str();
}

NMI_INVOKE(FeniaString, contains, "(words): true если эта строка содержит одно из слов из строки words")
{
    char strbuf[size( )];
    char *p_strbuf = strbuf;
    char *s_tok;
    
    if (args.empty())
        throw NotEnoughArgumentsException();
        
    const char *arg = args.front( ).toString( ).c_str( );
    strcpy(strbuf, c_str( ));

    while (( s_tok = strtok( p_strbuf, " " ) ) != NULL) {
        p_strbuf = NULL;

        if (::is_name( s_tok, arg ))
            return true;
    }
    
    return false;
}

NMI_INVOKE(FeniaString, split, "(sep): возвращает List из подстрок, разбитых по разделителю sep")
{
    string delim;
    RegList::Pointer list(NEW);
    
    if (args.empty() || args.front().toString().empty())
        delim = "\n";
    else
        delim = args.front().toString();
       
    StringList tokens;
    tokens.split(*this, delim);
    for (auto &token: tokens)
        list->push_back(Register(token));

    Object *obj = &Object::manager->allocate();
    obj->setHandler(list);

    return Register(obj);
}

NMI_INVOKE(FeniaString, at, "(i): возвращает символ в позиции i")
{
    int i = args2number(args);
    if (i < 0 || (unsigned) i >= this->size())
        throw Scripting::IndexOutOfBoundsException();

    DLString result;
    result.assign(this->at(i));
    return result;
}

NMI_INVOKE(FeniaString, api, "(): печатает этот api")
{
    ostringstream buf;
    
    traitsAPI<FeniaString>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE(FeniaString, format, "(width): превращает строку в абзац заданной ширины")
{
    int width;
    ostringstream os;
    istringstream is(c_str( ));
    
    if (args.empty())
        throw NotEnoughArgumentsException();

    width = args.front().toNumber();

    Formatter fmt(os, is);
    fmt.format(0, width);
    return os.str( );
}


NMI_INVOKE(FeniaString, replace, "(s1,s2): заменяет все вхождения первой подстроки на вторую")
{
    DLString aStr, bStr;
    RegisterList::const_iterator iter;
    
    if (args.size() < 2)
        throw NotEnoughArgumentsException();

    iter = args.begin();
    aStr = iter->toString();
    bStr = (++iter)->toString();

    DLString r = *this;
    r.replaces( aStr, bStr );
    return r;
}

NMI_INVOKE(FeniaString, isRussian, "(): возвращает true если строка не пуста и содержит только русские буквы")
{
    if (empty( ))
        return false;

    for (iterator i = begin(); i != end(); i++) 
        if (!dl_isrusalpha( *i ) && !dl_isspace( *i ))
            return false;

    return true;
}

NMI_INVOKE(FeniaString, toLower, "(): переводит всю строку в нижний регистр")
{
    DLString s = *this;
    for( DLString::size_type pos = 0; pos < s.length( ); pos++ )
    {
            char& ch = s.at( pos );
            ch = Char::lower( ch );
    }
    return s;
}

NMI_INVOKE(FeniaString, toUpper, "(): переводит всю строку в верхний регистр")
{
    DLString s = *this;
    for( DLString::size_type pos = 0; pos < s.length( ); pos++ )
    {
            char& ch = s.at( pos );
            ch = Char::upper( ch );
    }
    return s;
}

}

