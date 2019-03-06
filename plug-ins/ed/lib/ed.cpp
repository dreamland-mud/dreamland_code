/* $Id: ed.cpp,v 1.1.2.4.6.1 2007/06/26 07:12:37 rufina Exp $
 *
 * ruffina, 2004
 */
/* $Id: ed.cpp,v 1.1.2.4.6.1 2007/06/26 07:12:37 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */
#include <sys/types.h>
#include <sstream>

using namespace std;
    
#include "src/regex/regex.h"
#include "ed.h"

int Editor::Line::count = 0;

void
Editor::reg_t::split(const std::string &s)
{
    clear( );

    char buf[1024];
    istringstream is(s);

    while( is.getline(buf, sizeof(buf)) )
        push_back(buf);
}

string
Editor::reg_t::dump( ) const
{
    ostringstream str;

    for(const_iterator i = begin(); i != end(); i++)
        str << *i << endl;

    return str.str( );
}

Editor::Editor() 
{
    currentline = lines.end( );
}

Editor::~Editor() 
{
    clear( );
}

int
Editor::linenum(ipos_t i)
{
    int n = 1;
    ipos_t j;
    
    for(j = lines.begin(); j != lines.end() ; j++, n++)
        if(j == i)
            return n;

    return 0;
}

int
Editor::checkswap(ipos_t i, ipos_t j)
{
    int l, r;

    l = linenum(i);
    r = linenum(j);

    if(!l || !r) {
        yyerror("address not in list");
        return -1;
    }

    if(l > r) {
        yyerror("start address greater then stop");
        return -1;
    }

    return 0;
}

void
Editor::safeadvance(ipos_t &ipos, int off)
{
    for(;ipos != lines.end() && off != 0;) {
        if(off > 0) {
            ipos++;
            off--;
        } else {
            ipos--;
            off++;
        }
    }
}

void
Editor::unmark(ipos_t i)
{
    marks_t::iterator it;
    list<char> keys;

    for(it = marks.begin( ); it != marks.end( ); it++)
        if(it->second == i)
            keys.push_back(it->first);
    
    list<char>::iterator ki;
    for(ki = keys.begin( ); ki != keys.end( ); ki++)
        marks.erase(*ki);
}

void
Editor::clear( )
{
    ipos_t begin = lines.begin( );
    ipos_t end = lines.end( );

    if(begin != end) {
        undo_del(begin, end->prev);
        reque(begin->prev, end.data);
    }

    marks.clear( );
    appendlines.clear( );

    clear_changes( undo );
    clear_undo( );

    currentline = lines.end( );
    append_at = ipos_t ( );
}

int
Editor::do_print(ipos_t l, ipos_t r)
{
    if(checkswap(l, r) < 0)
        return -1;

    currentline = r;
    ipos_t i;
    
    r++;
    int limit = 80;
    for(i = l; i != r; i++, limit--) {
        if(!limit) {
            yyerror("too much to print (>80 lines)");
            return 0;
        }
        print(i->str);
    }


    return 0;
}

int
Editor::do_number(ipos_t l, ipos_t r)
{
    if(checkswap(l, r) < 0)
        return -1;


    currentline = r;
    r++;
    ipos_t i;
    int n;
    int limit = 80;
    for(i = l, n = linenum(l); i != r; i++, n++, limit--) {
        if(!limit) {
            yyerror("too much to print (>80 lines)");
            return 0;
        }
        ostringstream ostr;
        ostr.width(3);
        ostr.flags(ios::left);
        ostr << n << ": " << i->str;
        print(ostr.str());
    }

    return 0;
}

int
Editor::do_subst(ipos_t l, ipos_t r, int flags)
{
    if(checkswap(l, r) < 0)
        return -1;

    if(lastsubst.pat.empty()) {
        yyerror("no previous substitution");
        return -1;
    }
    regex_t re;
    
    int rc;
    
    rc = regcomp(&re, lastsubst.pat.c_str(), REG_EXTENDED);

    if(rc < 0) {
        char buf[1024];
        
        regerror(rc, &re, buf, sizeof(buf));

        yyerror(buf);
        return -1;
    }

    ipos_t i;
    
    rc = -1;
    
    const char *rep = lastsubst.rep.c_str();
    
    r++;
    for(i = l; i != r; i++) {
        string &line = i->str;
        regmatch_t matches[re.re_nsub + 1];
        string out;
        bool lastempty = true;
        
        const char *start = line.c_str();
        int slen = line.size();

        matches[0].rm_so = 0;
        matches[0].rm_eo = slen;
        
        if(regexec( &re, start, re.re_nsub + 1, matches, REG_STARTEND) != 0)
            continue;
            
        do {
            if(lastempty || matches[0].rm_so != matches[0].rm_eo) {
                int s = matches[0].rm_so;
                
                out.append(start, start + s);

                const char *src = rep;
                unsigned int no;
                char c;

                while( (c = *src++) != 0 ) {
                    switch(c) {
                    case '&':
                        no = 0;
                        break;
                    case '\\':
                        switch(*src) {
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9':
                            no = *src++ - '0';
                            break;
                        case 'r':
                            out += '\r';
                            src++;
                            continue;
                        default:
                            out += *src++;
                            continue;
                        }
                        break;
                    default:
                        out += c;
                        continue;
                    }

                    if (no > re.re_nsub || matches[no].rm_so < 0
                                        || matches[no].rm_eo < 0) 
                    {
                        yyerror("Wrong sub-expression number in pattern");
                        continue;
                    }

                    out.append(start + matches[no].rm_so, 
                               start + matches[no].rm_eo);
                }
            }

            if (matches[0].rm_so != matches[0].rm_eo) {
                start += matches[0].rm_eo;
                slen -= matches[0].rm_eo;
                lastempty = 0;
            } else {
                if(matches[0].rm_so < slen)
                    out += start[matches[0].rm_so];
                start += matches[0].rm_so + 1;
                slen -= matches[0].rm_so + 1;
                lastempty = 1;
            }
            
            if(!(flags & SF_GLOBAL))
                break;

            matches[0].rm_so = 0;
            matches[0].rm_eo = slen;
        } while(slen >= 0 && regexec( &re, start, 
               re.re_nsub + 1, matches, REG_STARTEND | REG_NOTBOL) == 0);
        
        if(slen > 0)
            out.append(start, start + slen);
        
        if(flags & SF_PRINT) {
            ostringstream ostr;
            ostr << "substitute: " << line << " -> " << out << endl;
            print(ostr.str());
        }

        /*delete matching line*/
        undo_del(i, i);
        reque(i->prev, i->next);
        
        /*insert substituted lines*/
        currentline = i->prev;
        i++;
        string::size_type sit;
        for(;;) {
            sit = out.find( '\r' );

            lines.insert(i, string(out, 0, sit));
            if(sit == out.npos)
                break;

            out.erase(0, sit + 1);
        }

        i--;
        undo_add(currentline->next, i);
        currentline = i;
    }

    return 0;
}

int
Editor::do_del(ipos_t l, ipos_t r, char reg)
{
    if(checkswap(l, r) < 0)
        return -1;
    
    r++;

    reg_t &rl = registerAt(reg);
    rl.assign(l, r);
    
    undo_del(l, r->prev);
    reque(l->prev, r.data);

    currentline = r;
    
    if(currentline == lines.end())
        currentline--;
    
    return 0;
}

int
Editor::do_yank(ipos_t l, ipos_t r, char reg)
{
    if(checkswap(l, r) < 0)
        return -1;
    
    currentline = r;
    r++;
    registerAt(reg).assign(l, r);

    
    return 0;
}

int
Editor::do_put(ipos_t i, char reg)
{
    const reg_t &r = registerAt(reg);
    
    if(r.empty( )) {
        yyerror("register empty");
        return -1;
    }
        
    currentline = i;
    i++;
    lines.insert(i, r.begin( ), r.end( ));
    undo_add(currentline->next, i->prev);

    advance(currentline, r.size( ));

    return 0;
}

int
Editor::do_append(const string &line)
{
    if(line == ".") {
        currentline = append_at;
        
        if(appendlines.empty())
            return 0;
        
        append_at++;
        lines.insert(append_at, appendlines.begin(), appendlines.end());
        append_at--;
        undo_add(currentline->next, append_at);

        currentline = append_at;

        appendlines.clear();

        append_at = list_t::iterator();
    } else
        appendlines.insert( appendlines.end(), line );
    
    return 0;
}

int 
Editor::do_shell(ipos_t l, ipos_t r, const string &cmd)
{
    int i, j;
    
    if(checkswap(l, r) < 0)
        return -1;

    r++;

    list_t alist;
    alist.assign(l, r);
    i = alist.size( );
    
    ostringstream os;
    alist.tostream(os);
    istringstream is(shell(cmd, os.str()));
    
    alist.clear();
    alist.fromstream(is);
    j = alist.size( );

    if(os.str( ) == is.str( )) {
        print("buffer not changed.");
        return 0;
    }
    
    undo_del(l, r->prev);
    reque(l->prev, r.data);

    currentline = r;
    if(alist.size() != 0) {
        currentline--;
        
        lines.insert(r, alist.begin(), alist.end());
        undo_add(currentline->next, r->prev);

        currentline = r;
        currentline--;
    }

    ostringstream report;
    report << i << " lines deleted, " << j << " lines inserted.";
    print(report.str( ));

    return 0;
}

int
Editor::do_join(ipos_t l, ipos_t r)
{
    if(checkswap(l, r) < 0)
        return -1;

    currentline = l;
    
    ipos_t i;
    
    string joined = l->str;

    undo_del(l, r);
    reque(l->prev, r->next);

    l++;
    r++;

    for(i = l; i != r; i++)
        joined += " " + i->str;
    
    currentline = lines.insert(r, joined);
    undo_add(currentline, currentline);

    return 0;
}

void
Editor::release_undo(Undo &u)
{
    ipos_t i = u.head;
    
    for(;i != u.tail;) {
        unmark(i);
        delete (i++).data;
    }

    unmark(i);
    delete (i++).data;
}

int
Editor::do_undo()
{
    if(undostack.empty()) {
        yyerror("nothing to undo");
        return -1;
    }

    CmdUndo &cu = undostack.front();
    list<Undo> &us = cu.changes;

    for(; !us.empty(); us.pop_front()) {
        Undo &u = us.front();

        if(u.add) {
            reque(u.head->prev, u.tail->next);
            release_undo(u);
        } else {
            reque(u.head->prev, u.head.data);
            reque(u.tail.data, u.tail->next);
        }
    }

    currentline = cu.currentline;

    undostack.pop_front();

    return 0;
}

void 
Editor::undo_add(ipos_t h, ipos_t t)
{
    Undo u;
    
    u.add = true;
    u.head = h;
    u.tail = t;

    undo.push_front(u);
}

void 
Editor::undo_del(ipos_t h, ipos_t t)
{
    Undo u;
    
    u.add = false;
    u.head = h;
    u.tail = t;
    
    undo.push_front(u);
}

void 
Editor::List::fromstream(istream &is)
{
    char buf[1024];
    
    while( is.getline(buf, sizeof(buf)) )
        insert(end(), string(buf));
}

void
Editor::List::tostream(ostream &os)
{
    ipos_t i;

    for(i = begin();i != end(); i++)
        os << i->str << endl;
}


Editor::ipos_t
Editor::lookup(const string &resrc, bool forward)
{
    if(lines.empty()) {
        yyerror("buffer empty");
        return lines.end();
    }

    if(resrc.empty()) {
        yyerror("no previous pattern");
        return lines.end();
    }

    int rc;
    regex_t re;
    
    regfree(&re);

    rc = regcomp(&re, resrc.c_str(), REG_NOSUB | REG_EXTENDED);

    if(rc < 0) {
        char buf[1024];
        
        regerror(rc, &re, buf, sizeof(buf));

        yyerror(buf);
        return lines.end();
    }

    ipos_t i = currentline;
    
    do {
        if(forward)
            i++;
        else
            i--;
        
        if(i == lines.end()) {
            print("search wrapped");
            continue;
        }

        if(regexec( &re, i->str.c_str(), 0, 0, 0) == 0)
            return i;
        
    } while(i != currentline);

    return lines.end();
}


void 
Editor::clear_changes( list<Undo> &us )
{
    for(; !us.empty(); us.pop_front()) {
        Undo &u = us.front();

        if(!u.add) 
            release_undo( u );
    }
}

void 
Editor::clear_undo()
{
    for(;!undostack.empty(); undostack.pop_front())
        clear_changes(undostack.front( ).changes);
}

void
Editor::setBuffer(const string &b)
{
    if(!lines.empty())
        do_del(lines.begin(), lines.end()->prev, 0);

    appendBuffer(b);
}

void
Editor::appendBuffer(const string &b)
{
    char buf[512], *p;
    ipos_t i;

    if(b.empty())
        return;

    i = currentline;
    i++;
    string::const_iterator pos;
    
    for( p = buf, pos = b.begin( ); pos != b.end( ); pos++ ) {
        switch(*pos) {
            case '\r':
                break;
            case '\n':
                *p = 0;
                p = buf;
                lines.insert(i, string(p));
                break;
            default:
                if(p < buf + sizeof(buf) - 1)
                    *p++ = *pos;
                else
                    yyerror("input line too long");
        }
    }
    if(p != buf) {
        *p = 0;
        p = buf;
        lines.insert(i, string(p));
    }
    i--;

    if(currentline == i)
        return;

    undo_add(currentline->next, i);

    CmdUndo cu;

    cu.currentline = currentline;
    cu.changes = undo;

    undostack.push_front(cu);

    while(!undo.empty())
        undo.pop_front();
    
    currentline = i;
}

void
Editor::yyerror(const char *msg)
{
    error(msg);
}

void
Editor::eval(const string &s)
{
    istringstream input(s);
    char buf[512];

    while(input.getline(buf, sizeof(buf))) {
        string line(buf);
        ipos_t cline = currentline;
        
        if(append_at) {
            do_append(line);

        } else {
            istringstream is(line);
            switch_streams(&is, 0);
            if(parse())
                break;
        }

        if(append_at || undo.empty())
            continue;

        CmdUndo cu;

        cu.currentline = cline;
        cu.changes = undo;

        undostack.push_front(cu);

        while(!undo.empty())
            undo.pop_front();
    }
}

