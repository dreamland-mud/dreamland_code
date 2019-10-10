/* $Id: ed.h,v 1.1.2.2.6.2 2014-09-19 11:50:41 rufina Exp $
 *
 * ruffina, 2004
 */
/* $Id: ed.h,v 1.1.2.2.6.2 2014-09-19 11:50:41 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __ED_H__
#define __ED_H__

#ifndef yyFlexLexer
#define yyFlexLexer edFlexLexer
// MOC_SKIP_BEGIN
#include <FlexLexer.h>
// MOC_SKIP_END
#endif

#if 0
class yyFlexLexer
{
}
#endif

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <string> 

using namespace std;

struct Editor : public yyFlexLexer {
    struct Line {
        Line() : prev(0), next(0) { count++; }
        Line(const string &s) : str(s.c_str()), prev(0), next(0) { count++; }
        ~Line() { count--; }

        operator const string () const {
            return str;
        }

        string str;
        Line *prev, *next;

        static int count;
    };
    
    
    inline static void reque(Line *pred, Line *succ) {
        pred->next = succ;
        succ->prev = pred;
    }
    inline static void insque(Line *elem, Line *pred) {
        reque(elem, pred->next);
        reque(pred, elem);
    }
    inline static void remque(Line *elem) {
        reque(elem->prev, elem->next);
    }

    struct List {
        template <typename Line>
        struct _iterator {
            typedef Line &reference;
            typedef Line *pointer;
            typedef Line value_type;
            typedef bidirectional_iterator_tag iterator_category;
            typedef ptrdiff_t difference_type;
            typedef size_t size_type;

            Line *data;

            _iterator() : data(0) { }
            _iterator(Line *l) : data(l) { }

            Line &operator *() {
                return *data;
            }
            Line *operator ->() {
                return data;
            }
            
            _iterator &operator ++ () {
                data = data->next;
                return *this;
            }
            _iterator &operator -- () {
                data = data->prev;
                return *this;
            }
            _iterator operator ++ (int) {
                _iterator rc = *this;
                data = data->next;
                return rc;
            }
            _iterator operator -- (int) {
                _iterator rc = *this;
                data = data->prev;
                return rc;
            }

            bool operator == (const _iterator &r) {
                return data == r.data;
            }
            bool operator != (const _iterator &r) {
                return data != r.data;
            }

            operator bool () const {
                return valid();
            }

            bool valid() const {
                return data != 0;
            }
        };
        typedef _iterator<Line> iterator;
        typedef _iterator<const Line> const_iterator;

        Line *data;

        List(const List &l) {
            data = new Line;
            data->next = data->prev = data;
            assign(l.begin(), l.end());
        }
        List() {
            data = new Line;
            data->next = data->prev = data;
        }
        ~List() {
            clear();
            delete data;
        }
        iterator begin() {
            return data->next;
        }
        iterator end() {
            return data;
        }
        const_iterator begin() const {
            return data->next;
        }
        const_iterator end() const {
            return data;
        }

        size_t size() {
            return distance(begin(), end());
        }
        bool empty() {
            return begin() == end();
        }

        void erase(iterator i) {
            remque(i.data);
            /*list element can't be marked, so delete is safe*/
            delete i.data;
        }
        iterator erase(iterator start, iterator stop) {
            while(start != stop)
                erase(start++);
            return stop;
        }

        void clear() {
            erase(begin(), end());
        }

        iterator insert(iterator pos, const Line &d) {
            Line *n = new Line;

            n->str = d.str;

            reque(pos->prev, n);
            reque(n, pos.data);

            return n;
        }

        template <typename T1>
        void insert(iterator pos, T1 start, T1 stop) {
            for(;start != stop; start++)
                insert(pos, *start);
        }
        
        template <typename T1>
        void assign(T1 start, T1 stop) {
            clear();
            insert(end(), start, stop);
        }

        const List &operator = (const List &r) {
            assign(r.begin(), r.end());
            return *this;
        }
        
        void fromstream(istream &is);
        void tostream(ostream &os);
    };

    typedef Editor::List list_t;
    typedef list_t::iterator ipos_t;
    typedef list_t::const_iterator cipos_t;
    typedef map<char, ipos_t> marks_t;
    
    struct reg_t : public list<string> {
        void split(const std::string &);
        string dump( ) const;
    };
    
    Editor();
    virtual ~Editor();

    void appendBuffer(const string &b);
    void setBuffer(const string &b);
    void eval(const string &s);
    
#define SF_NONE            0
#define SF_PRINT    1
#define SF_GLOBAL   2
    struct subst_args {
        string pat, rep;
    };

    struct SVal {
        string str;
        int number;
        char character;
        ipos_t ipos;
        pair<ipos_t, ipos_t> ipair;
        subst_args sargs;
        int sflags;
    };

    ipos_t currentline;
    list_t lines;
    marks_t marks;
private:
    int parse( );
    int lex( SVal * );

public:
    int linenum(ipos_t i);
    int checkswap(ipos_t i, ipos_t j);
    void safeadvance(ipos_t &ipos, int off);
    ipos_t lookup(const string &resrc, bool forward);
    void yyerror(const char *msg);
    void unmark(ipos_t i);
    void clear( );
    
    int do_print(ipos_t l, ipos_t r);
    int do_number(ipos_t l, ipos_t r);
    int do_subst(ipos_t l, ipos_t r, int flags);
    int do_del(ipos_t l, ipos_t r, char reg);
    int do_append(const string &);
    int do_shell(ipos_t l, ipos_t r, const string &);
    int do_yank(ipos_t l, ipos_t r, char reg);
    int do_put(ipos_t l, char reg);
    int do_join(ipos_t l, ipos_t r);
    int do_undo();

    subst_args lastsubst;
    string lastpat;

    struct Undo {
        bool add;
        ipos_t head, tail;
    };

    struct CmdUndo {
        list<Undo> changes;
        ipos_t currentline;
    };

    list<CmdUndo> undostack;
    list<Undo> undo;

    void undo_add(ipos_t h, ipos_t t);
    void undo_del(ipos_t h, ipos_t t);
    void release_undo(Undo &u);
    void clear_changes( list<Undo> &us );
    void clear_undo();

    list_t appendlines;
    ipos_t append_at;

    virtual void print(const string &) = 0;
    virtual void error(const string &) = 0;
    virtual string shell(const string &, const string &) = 0;
    virtual void done() = 0;
    virtual void help() = 0;
    virtual reg_t &registerAt(char) = 0;
};


#endif /* __ED_H__ */
