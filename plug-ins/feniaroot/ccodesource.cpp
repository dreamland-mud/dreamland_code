/* $Id: ccodesource.cpp,v 1.1.2.12.6.9 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include <fstream>
#include <map>
#include <vector>

#include "logstream.h"
#include "exception.h"

#include "pcharacter.h"
#include "admincommand.h"
#include "commonattributes.h"
#include "dreamland.h"
#include "dlfileop.h"
#include "dlfilestream.h"
#include "dldirectory.h"

#include "codesourcerepo.h"
#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "fenia/object.h"
#include "xmlattributecodesource.h"
#include "wrappermanager.h"

#include "xmleditorinputhandler.h"

#include "comm.h"
#include "act.h"
#include "mudtags.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "dl_match.h"    
#include "dl_ctype.h"
#include "dl_strings.h"
#include "def.h"
#include "l10n.h"

using namespace Scripting;
using namespace std;

bool has_fenia_security( PCMemoryInterface *pch );
bool text_match_with_highlight(const DLString &text, const DLString &args, ostringstream &matchBuf);

static bool cs_by_subj(PCharacter *ch, const DLString &arg, id_t &csid)
{
    CodeSource::Manager::const_iterator i;
    for (i = CodeSource::manager->begin( );i != CodeSource::manager->end( ); i++) {
        if (i->name.find(arg) != DLString::npos
            || dl_match(arg.c_str(), i->name.c_str(), true)) 
    {
            csid = i->getId();
            return true;
        }
    }

    ch->pecho(_("Сценарий с темой '%s' не найден."), arg.c_str());
    return false;
}

static bool cs_by_number(PCharacter *ch, const DLString &arg, id_t &csid)
{
    CodeSource::Manager::iterator i;
    Integer num;

    if (!Integer::tryParse(num, arg)) {
        return cs_by_subj(ch, arg, csid);
    }

    i = CodeSource::manager->find(num);
    if (i == CodeSource::manager->end()) {
        ch->pecho(_("Сценарий под номером %d не найден."), num.getValue());
        return false;
    }

    csid = i->getId();
    return true;
}

CMDADM( codesource )
{
    PCharacter *pch = ch->getPC( );
    
    if (!pch)
        return;

    if (!has_fenia_security( pch )) {
        ch->pecho(_("Ты не ботаешь по фене."));
        return;
    }
    
    Register thiz = WrapperManager::getThis( )->getWrapper( ch );
    DLString args = constArguments, cmd;
    
    if (!args.empty( )) {
        if(dl_isalpha(args[0]))
            cmd = args.getOneArgument();
        else {
            cmd = args[0];
            args.erase(0, 1);
        }
    }
    
    if (cmd.empty( ) || arg_is_help(cmd)) {
        ostringstream buf;
        
        buf << "Синтаксис команды {Wcodesource{x:" << endl
            << "Чтение:" << endl
            << "     {Wlist{x [строка]      - показать список сценариев, всех или со строкой в названии" << endl
            << "     {Wread{x <номер>|<имя> - прочитать cs из списка по номеру или названию" << endl
            << "     {Wcopy{x <номер>|<имя> - скопировать cs из списка в буфер редактора" << endl
            << "     {Wsearch{x <строка>    - найти все сценарии, содержащие строку в коде"  << endl
            << "     {Wdups{x               - сценарии с дублирующимися именами (одно имя -> >1 cs)"  << endl
            << "     {Worphans{x            - счетчики неиспользуемых cs/объектов/функций (как boot fsck, read-only)"  << endl
            << "     {Wgc{x                 - dry-run: что схлопнула бы канонизация дублей при загрузке (read-only)"  << endl
            << endl
            << "Редактирование:" << endl
            << "     {Wweb{x [<номер>|<имя>] - редактировать новый или существующий сценарий в веб-редакторе" << endl
            << "     {Wpaste{x        - вставить текст cs из буфера редактора" << endl
            << "     {Wshow{x         - показать редактируемый cs" << endl
            << "     {Wclear{x        - очистить редактируемый cs" << endl
            << endl
            << "Выполнение:" << endl
            << "     {Wpost{x         - выполнить cs, далее можно делать eval blablaInit() для объявленной там функции" << endl;

        if (ch->isCoder( ))
            buf << endl
            << "Только для кодеров: " << endl
            << "     {Wload{x <файл>          - загрузить сценарий из файла в каталоге share/DL/fenia" << endl
            << "     {Wload all{x [<каталог>] - рекурсивно загрузить все сценарии из [под]каталога" << endl
            << "     {Wsave{x <номер>|<имя>   - сохранить сценарий на диск" << endl
            << "     {Wsave all{x             - сохранить все сценарии на диск" << endl
            << "     {Wdel{x <ном> force      - удалить cs: разорвать циклы, сборщик забирает недостижимый (сверься с findrefs)" << endl;

        ch->send_to( buf );
        return;
    }

    if(arg_is_list(cmd)) {
        ostringstream buf;
        CodeSource::Manager::iterator i;

        buf << "{YСценарии{x (всего " << CodeSource::manager->size() << "):" << endl;
        buf << fmt(0, "{W[%5s] %8s  %-36s %s{x\r\n", "Номер", "Автор:", "Название", "Используется функций");

        // Clicking on codesource ID will launch 'cs web <id>'.
        DLString lineFormat = "[" + web_cmd(ch, "cs web $1", "%5d") + "] {g%8s{x: %-36s {D(%d функц.){x\r\n";

        for(i = CodeSource::manager->begin( );i != CodeSource::manager->end( ); i++) {
            if (args.empty() 
                || i->name.find(args) != DLString::npos
                || is_name(args.c_str(), i->name.c_str())
                || dl_match(args.c_str(), i->name.c_str(), true)) {

                buf << fmt( 0, lineFormat.c_str(), 
                        i->getId(),
                        i->author.c_str( ), 
                        i->name.c_str( ),
                        i->functions.size( ));
            }
        }
        page_to_char(buf.str().c_str(), ch);
        return;
    }

    if(arg_is(cmd, "search")) {
        ostringstream buf;
        CodeSource::Manager::iterator i;
        static DLString highlight("{R");

        if (args.empty()) {
            ch->pecho(_("Синтаксис: cs search <строка>"));
            return;
        }

        DLString lineFormat = "[" + web_cmd(ch, "cs web $1", "%5d") + "] {g%8s{x: %-36s {D(%d функц.){x\r\n";

        buf << "Сценарии, содержащие '" << args << "':" << endl;
        
        for(i = CodeSource::manager->begin( );i != CodeSource::manager->end( ); i++) {
            ostringstream matchBuf;
            
            if (text_match_with_highlight(i->content, args, matchBuf)) {
                buf << fmt( 0, lineFormat.c_str(), 
                        i->getId(),
                        i->author.c_str( ), 
                        i->name.c_str( ),
                        i->functions.size( ))
                    << matchBuf.str()
                    << endl;
            }
        }
        page_to_char(buf.str().c_str(), ch);
        return;
    }

    if(arg_is_soft(cmd, "dups")) {
        // Group registered CodeSources by name; report names carrying more than
        // one live CodeSource. A name is meant to map to exactly one codesource,
        // so extra copies are "duplicate scenarios" held alive by references that
        // were never annulled. Transient engine-internal sources (empty name, or
        // "<...>" like "<eval command>") are excluded: they are one-off eval/force
        // fragments, not real duplicates. Output goes through pecho (no pager) so
        // it comes back through /api/force, unlike list/search's page_to_char.
        std::map<DLString, std::vector<id_t> > byName;
        for(CodeSource::Manager::iterator i = CodeSource::manager->begin( );
                i != CodeSource::manager->end( ); i++) {
            const DLString &nm = i->name;
            if (nm.empty( ) || nm[0] == '<')
                continue;
            byName[nm].push_back(i->getId( ));
        }

        int dupNames = 0, dupCopies = 0;
        ch->pecho("{YДубли сценариев по имени{x (name -> [cs ids]):");
        for(std::map<DLString, std::vector<id_t> >::iterator p = byName.begin( );
                p != byName.end( ); p++) {
            if (p->second.size( ) <= 1)
                continue;
            dupNames++;
            dupCopies += p->second.size( );
            ostringstream ids;
            for(size_t k = 0; k < p->second.size( ); k++) {
                if (k)
                    ids << ",";
                ids << p->second[k];
            }
            ch->pecho("  %-40s x%d : %s",
                    p->first.c_str( ), (int)p->second.size( ), ids.str( ).c_str( ));
        }
        ch->pecho("{YИтого{x: %d имен с дублями, %d лишних копий.",
                dupNames, dupCopies - dupNames);
        return;
    }

    if(arg_is_soft(cmd, "orphans")) {
        // Read-only mirror of the boot-time fenia fsck (ValidateTask): count
        // unreferenced CodeSources / objects / functions WITHOUT freeing any of
        // them. Freeing is what SIGSEGV'd four consecutive boots (see the comment
        // in validatetask.cpp); this only reports, so it is safe on live.
        int csOrphan = 0, objOrphan = 0, fnOrphan = 0;

        for(CodeSource::Manager::iterator i = CodeSource::manager->begin( );
                i != CodeSource::manager->end( ); i++) {
            if (i->refcnt == 0)
                csOrphan++;
            for(FunctionManager::iterator fi = i->functions.begin( );
                    fi != i->functions.end( ); fi++)
                if (fi->refcnt <= 0)
                    fnOrphan++;
        }

        // Scripting::Object must be qualified: a global game `class Object`
        // (items) is forward-declared via clan.h, so bare Object is ambiguous.
        for(Scripting::Object::Manager::iterator oi = Scripting::Object::manager->begin( );
                oi != Scripting::Object::manager->end( ); oi++)
            if (oi->refcnt <= 0 && oi->hasHandler( ))
                objOrphan++;

        ch->pecho("{YFenia orphans{x (unreferenced, reported not freed):");
        ch->pecho("  CodeSources:       %d", csOrphan);
        ch->pecho("  Objects(+handler): %d", objOrphan);
        ch->pecho("  Functions:         %d", fnOrphan);
        ch->pecho("  Всего сценариев в базе: %d", (int)CodeSource::manager->size( ));
        return;
    }

    if(arg_is_strict_soft(cmd, "gc")) {
        // Dry-run preview of the boot-time duplicate canonicalization (Fenia GC
        // phase, Trello #2857). Groups CodeSources by name; for every duplicated
        // name it picks a canonical copy (the most-referenced one -- the copy
        // closures mostly point at -- ties broken by lowest id for determinism)
        // and reports which other copies could be collapsed onto it. A copy is
        // collapsible only when its function SHAPE matches the canonical: the
        // same function count and identical argument-name lists position by
        // position (id order). Same-source recompiles match; a structurally
        // different stale copy is reported as skipped, never silently collapsed
        // -- collapsing a mismatched copy would rebind closures to the wrong
        // function. Reads only: this is the review artifact for the real
        // collapse, it changes nothing on disk or in memory.
        if (!ch->isCoder( )) {
            ch->pecho(_("Только для кодеров."));
            return;
        }

        std::map<DLString, std::vector<id_t> > byName;
        for(CodeSource::Manager::iterator i = CodeSource::manager->begin( );
                i != CodeSource::manager->end( ); i++) {
            const DLString &nm = i->name;
            if (nm.empty( ) || nm[0] == '<')
                continue;
            byName[nm].push_back(i->getId( ));
        }

        int dupNames = 0, collapsible = 0, mismatched = 0;
        ch->pecho("{YFenia GC dry-run{x (canonical <- collapse candidates, ничего не трогается):");

        for(std::map<DLString, std::vector<id_t> >::iterator p = byName.begin( );
                p != byName.end( ); p++) {
            if (p->second.size( ) <= 1)
                continue;
            dupNames++;

            id_t canonId = p->second[0];
            for(size_t k = 1; k < p->second.size( ); k++) {
                id_t cid = p->second[k];
                CodeSource &c = CodeSource::manager->at(cid);
                CodeSource &best = CodeSource::manager->at(canonId);
                if (c.refcnt > best.refcnt
                    || (c.refcnt == best.refcnt && cid < canonId))
                    canonId = cid;
            }

            CodeSource &canon = CodeSource::manager->at(canonId);
            ch->pecho("{C%s{x", p->first.c_str( ));
            ch->pecho("    canonical {W%d{x fns=%d refcnt=%d",
                    canonId, (int)canon.functions.size( ), canon.refcnt);

            for(size_t k = 0; k < p->second.size( ); k++) {
                id_t cid = p->second[k];
                if (cid == canonId)
                    continue;
                CodeSource &c = CodeSource::manager->at(cid);

                bool match = (c.functions.size( ) == canon.functions.size( ));
                if (match) {
                    FunctionManager::iterator fa = canon.functions.begin( );
                    FunctionManager::iterator fb = c.functions.begin( );
                    for( ; fa != canon.functions.end( ) && fb != c.functions.end( );
                            fa++, fb++) {
                        DLString aa = fa->argNames ? fa->argNames->toString( ) : DLString::emptyString;
                        DLString bb = fb->argNames ? fb->argNames->toString( ) : DLString::emptyString;
                        if (aa != bb) {
                            match = false;
                            break;
                        }
                    }
                }

                if (match) {
                    collapsible++;
                    ch->pecho("      collapse {G%d{x fns=%d refcnt=%d",
                            cid, (int)c.functions.size( ), c.refcnt);
                } else {
                    mismatched++;
                    ch->pecho("      {Rskip{x %d fns=%d refcnt=%d (different shape)",
                            cid, (int)c.functions.size( ), c.refcnt);
                }
            }
        }

        ch->pecho("{YИтого{x: %d имен, %d копий к схлопыванию, %d пропущено (разная сигнатура).",
                dupNames, collapsible, mismatched);
        return;
    }

    if(arg_is_strict(cmd, "del")) {
        // Reap one CodeSource by breaking its intra-cs reference cycles: drop
        // every function body, then let reference counting collect. A truly
        // unreachable island (a pre-P2b duplicate, or a cyclic zombie like sigils
        // #61639 / Setbat #41573 that refcount alone can never free) goes away
        // entirely, DB record included. A source still held from OUTSIDE survives
        // with dead bodies (invoke throws NullPointer, never a use-after-free) and
        // is restored intact on the next reboot -- so mis-reaping a still-reachable
        // source degrades gracefully, it does not crash. `force` is required
        // because this cannot itself prove unreachability (that is P4); the
        // operator asserts it with `findrefs <id>` first. Engine one-off names
        // ("<...>") are never deletable. See Trello #2857 (P3).
        if (!ch->isCoder( )) {
            ch->pecho(_("Только для кодеров."));
            return;
        }

        DLString idarg = args.getOneArgument( );
        DLString flag  = args.getOneArgument( );
        bool force = (flag == "force");

        Integer num;
        if (idarg.empty( ) || !Integer::tryParse(num, idarg)) {
            ch->pecho("Синтаксис: {Wcs del <числовой id> force{x. Сначала сверься с {Wfindrefs <id>{x.");
            return;
        }

        id_t csid;
        if (!cs_by_number(pch, idarg, csid))
            return;

        CodeSource &cs = CodeSource::manager->at(csid);

        if (cs.name.empty( ) || cs.name[0] == '<') {
            ch->pecho("Нельзя удалить служебный сценарий '%s'.", cs.name.c_str( ));
            return;
        }

        if (!force) {
            ch->pecho("Удаление cs %d (%s) необратимо и может задеть живые ссылки. "
                      "Проверь {Wfindrefs %d{x; если сценарий недостижим -- {Wcs del %d force{x.",
                      csid, cs.name.c_str( ), csid, csid);
            return;
        }

        // Snapshot before the point of no return; content stays recoverable from
        // the DB backup.
        LogStream::sendWarning( )
            << "cs del: force-reaping cs " << csid << " (" << cs.name << ") by "
            << pch->getNameC( ) << " -- author=" << cs.author
            << " refcnt=" << cs.refcnt << " functions=" << cs.functions.size( ) << endl;

        DLString name = cs.name;   // report after; `cs` may be gone by then

        // Pin the source across the whole sequence, so the last function's
        // collection can't collapse the CodeSource from INSIDE functions.erase()
        // -- that nested teardown ends with an rb_tree node_count write into the
        // just-freed cs chunk (inert on glibc, but real UB; review N2). With the
        // pin held, functions.erase() returns cleanly and the collapse fires at
        // top level from keep.clear() below, on an empty function map.
        CodeSource::Pointer keep(&cs);

        // Pin every function first (raw link), so nulling one body -- which fires
        // the ClosureExp dtors that unlink sibling functions -- cannot drop a
        // sibling to refcnt 0 and erase it from the map mid-loop. Then null the
        // bodies (cycles broken), then unpin: a function with no external
        // reference now collects, and ~ClosureExp never touches a freed sibling
        // because no body survives into the teardown (the numbered-function UAF,
        // review F3). Iterate a pointer snapshot, not the live map.
        std::vector<Function *> fns;
        FunctionManager::iterator fi;
        for(fi = cs.functions.begin( ); fi != cs.functions.end( ); fi++) {
            fns.push_back( &*fi );
            fi->link( );
        }
        for(size_t k = 0; k < fns.size( ); k++) {
            fns[k]->stmts = StmtNodeList::Pointer( );
            fns[k]->argNames = ArgNames::Pointer( );
        }
        for(size_t k = 0; k < fns.size( ); k++)
            fns[k]->unlink( );

        // Release the pin -- if every function collected, the cs collapses here,
        // at top level, with an empty function map. Must run BEFORE the survivor
        // check or a fully-collected source would misreport as survived.
        keep.clear( );

        if (CodeSource::manager->find(csid) == CodeSource::manager->end( ))
            ch->pecho("Сценарий %d (%s) удалён: цикл разорван, сборщик забрал его.",
                    csid, name.c_str( ));
        else
            ch->pecho("Сценарий %d (%s): циклы разорваны, но на него ещё есть ВНЕШНИЕ "
                    "ссылки -- не удалён (перезагрузка восстановит). Проверь {Wfindrefs %d{x.",
                    csid, name.c_str( ), csid);
        return;
    }

    if(arg_is_copy(cmd)) {
        id_t csid;
        if (!cs_by_number(pch, args, csid))
            return;

        const DLString &content = CodeSource::manager->at(csid).content;
        pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
            ->regs[0].split(content);
        
        ch->pecho(_("Сценарий %d скопирован в буфер редактора."), csid);
        return;
    }
    
    if(arg_is(cmd, "read")) {
        id_t csid;
        if (!cs_by_number(pch, args, csid))
            return;

        CodeSource &cs = CodeSource::manager->at(csid);        
        ch->pecho( "[%u] {g%s{x: %s", 
                    cs.getId(), 
                    cs.author.c_str( ),
                    cs.name.c_str( ));
        
        ostringstream ostr;
        string::const_iterator c;
        for(c = cs.content.begin( ); c != cs.content.end( ); c++ ) 
        {
            if(*c == '{')
                ostr << '{';
                
            ostr << *c;
        }
        page_to_char(ostr.str( ).c_str( ), ch);
        return;
    }
    
    if(arg_is(cmd, "save")) {
        if (!pch->isCoder( )) {
            ch->pecho("Check your privilege.");
            return;
        }

        if (args.empty()) {
            ch->pecho("Please specify codesource number, subj or 'all'.");
            return;
        }
        
        if (arg_is_all(args)) {
            CodeSourceRepo::getThis()->saveAll();
            return;
        }

        id_t csid;
        if (!cs_by_number(pch, args, csid))
            return;

        CodeSource &cs = CodeSource::manager->at(csid);               
        if (cs.name.empty()) {
            ch->pecho("This codesource has no subject, cannot save.");
            return;
        }

        DLString filecontent = cs.content;    
        try {
            DLDirectory dir( dreamland->getTableDir( ), "fenia.local" );
            DLFileStream( dir, cs.name, ".f++" ).fromString( filecontent );
            ch->pecho("Codesource %d is saved as  %s/%s.f++.",  
                        cs.getId(), dir.getAbsolutePath().c_str(), cs.name.c_str());
        } catch (const ::Exception &ex) {
            ch->pecho("Error saving codesource: %s", ex.what());
        }

        return;
    }

    if(arg_is_show(cmd)) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->pecho(_("Ты не редактируешь сценарий."));
            return;
        } 
        
        ch->pecho("%s: %s", 
                        pch->getNameC(), 
                        csa->name.getValue( ).c_str( ));
        
        XMLVectorBase<XMLString>::iterator i;
        
        ostringstream ostr;
        for(i = csa->content.begin( ); i != csa->content.end( ); i++) {
            string::const_iterator c;
            for(c = i->getValue( ).begin( ); c != i->getValue( ).end( ); c++ ) {
                if(*c == '{')
                    ostr << '{';
                ostr << *c;
            }
            ostr << endl;
        }
            
        page_to_char(ostr.str( ).c_str( ), ch);
            
        return;
    }
    
    if(arg_is_clear(cmd)) {
        pch->getAttributes( ).eraseAttribute( "codesource" );
        ch->pecho("Ok.");
        return;
    }
    
    if(arg_is_paste(cmd)) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->pecho(_("Ты не редактируешь сценарий."));
            return;
        } 
        
        const Editor::reg_t &reg = pch->getAttributes()
            .getAttr<XMLAttributeEditorState>("edstate")->regs[0];
        
        for(Editor::reg_t::const_iterator i = reg.begin(); i != reg.end(); i++)
            csa->content.push_back( DLString( *i ) );

        ch->pecho("Pasted from editor buffer.");
        return;
    }

    if(arg_is(cmd, "load")) {
        if (!pch->isCoder( )) {
            ch->pecho("This is not for you.");
            return;
        }

        DLString constArgs = args;
        DLString argOne = args.getOneArgument();
        if (arg_is_all(argOne)) {
            if (CodeSourceRepo::getThis()->readAll(args))
                ch->pecho("Recursively loaded all codesources from path %s.", args.c_str());
            else
                ch->pecho("Recursively loaded all codesources from path %s with some errors, check logs for details.", args.c_str());
            return;
        }

        if (CodeSourceRepo::getThis()->read(constArgs))
            ch->pecho("Codesource '%s' loaded from disk.", constArgs.c_str());
        else
            ch->pecho("Error loading codesource '%s', check logs for details.", constArgs.c_str());

        return;
    }

    if(arg_is(cmd, "eval") || arg_is(cmd, "post")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->pecho(_("Ты не редактируешь сценарий."));
            return;
        } 
        
        // Reuse the existing source of this name so a re-post replaces it in
        // place instead of minting a duplicate CodeSource. See P2b, Trello #2857.
        CodeSource *csReuse = CodeSource::manager->findByName(csa->name);
        CodeSource &cs = csReuse ? *csReuse : CodeSource::manager->allocate();

        cs.author = pch->getNameC();
        cs.name = csa->name;

        ostringstream sbuf;
        XMLVectorBase<XMLString>::iterator i;
        for(i = csa->content.begin( ); i != csa->content.end( ); i++)
            sbuf << i->getValue( ) << endl;
        
        cs.content = sbuf.str( );
        
        pch->getAttributes( ).eraseAttribute( "codesource" );
        
        if (dreamland->hasOption( DL_BUILDPLOT )) {
            LogStream::sendNotice( ) << "codesource: author=" << cs.author << " subj=" << cs.name << endl;
            LogStream::sendNotice( ) << sbuf.str( ) << endl;
        }
        
        try {
            cs.eval( thiz );
            ch->pecho("Ok.");
        } catch(const ::Exception& e ) {
            ostringstream ostr;
            ostr << "Evaluation exception: " << e.what() << endl;
            ch->send_to(ostr);
        }
        
        return;
    }
    
    if(arg_is(cmd, "web")) {
        std::vector<DLString> parms;
        
        if(!args.empty()) {
            id_t csid;
            if (!cs_by_number(pch, args, csid))
                return;

            CodeSource &cs = CodeSource::manager->at(csid);        
            parms.resize(2);
            parms[0] = cs.name;
            parms[1] = cs.content;
        }

        ch->desc->writeWSCommand("cs_edit", parms);
        return;
    }
    
    ch->pecho(_("Неверная подкоманда, используйте {Wcodesource help{x для справки."));
}


