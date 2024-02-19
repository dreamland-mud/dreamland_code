/* $Id: codesource.cpp,v 1.1.2.6.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: codesource.cpp,v 1.1.2.6.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <sstream>

#include "function.h"
#include "codesource.h"
#include "register-impl.h"
#include "object.h"


#undef yyFlexLexer
#define yyFlexLexer feniaFlexLexer
#include <FlexLexer.h>
#include "feniaparser.h"
#include <string.h>

namespace Scripting {

Register
CodeSource::eval()
{
    return eval(Register( ));
}

Register
CodeSource::eval(Register thiz)
{
    manager->put(id, *this);
    evaled = true;

    istringstream is(content);
    return FeniaParser(is, *this).eval( thiz );
}

void
CodeSource::finalize( )
{
    if(evaled)
        manager->del( id );

    manager->erase( id );
}

/*-----------------------------------------------------------------------
 * CodeSource::Manager
 *-----------------------------------------------------------------------*/
CodeSource::Manager *CodeSource::manager = 0;

CodeSource::Manager::Manager( )
{
    manager = this;
}

CodeSource::Manager::~Manager( )
{
    manager = 0;
}

void
CodeSource::Manager::open( )
{
    DbContext::open( "fenia", "codesouces" );
}

void
CodeSource::Manager::close( )
{
    DbContext::close( );
}

void
CodeSource::Manager::seq(id_t id, Data &val)
{
    char *begin = (char *)val.get_data( );
    char *end = begin + val.get_size( );

    char *arr[3], *p;
    unsigned int len[3], i;

    p = begin;
    
    for(i=0; i < sizeof(arr)/sizeof(*arr); i++) {
        arr[i] = p;
        
        for(; p < end && *p; p++)
            ;
        
        if(*p)
            throw ::Exception( "Object::Manager::seq: record too small" );
        
        len[i] = p - arr[i];
        p++;
    }
    
    if(p != end)
        throw ::Exception( "Object::Manager::seq: record too long" );

    CodeSource &cs = at( id );
    cs.name.assign(arr[0], len[0]);
    cs.author.assign(arr[1], len[1]);
    cs.content.assign(arr[2], len[2]);
    cs.evaled = true;

    /*XXX - does it belongs here?*/
    istringstream is(cs.content);
    FeniaParser(is, cs).compile( );

    if(id > lastId)
        lastId = id;
}

void
CodeSource::Manager::put( id_t id, CodeSource &cs )
{
    int nl = cs.name.size( ) + 1;
    int al = cs.author.size( ) + 1;
    int cl = cs.content.size( ) + 1;
    char buf[nl + al + cl];
    Data val(buf, nl + al + cl);
    
    memcpy(buf, cs.name.c_str( ), nl);
    memcpy(buf + nl, cs.author.c_str( ), al);
    memcpy(buf + nl + al, cs.content.c_str( ), cl);

    DbContext::put(id, val);
}


/*-----------------------------------------------------------------------
 * CodeSourceRef
 *-----------------------------------------------------------------------*/
ostream &
operator << (ostream &os, const CodeSourceRef &csr)
{
    CodeSource::Pointer cs = csr.source;
    
    if(cs)
        os 
           << "cs #" << cs->getId() << " (" << cs->name << ")";
    else
        os << "<unknown>";

    os << " line " << csr.line;
    return os;
}

}

