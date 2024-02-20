/* $Id: ccodesource.cpp,v 1.1.2.12.6.9 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include <fstream>

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

    ch->pecho("Сценарий с темой '%s' не найден.", arg.c_str());
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
        ch->pecho("Сценарий под номером %d не найден.", num.getValue());
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
        ch->pecho("Ты не ботаешь по фене.");
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
    
    if (cmd.empty( ) || cmd.strPrefix( "help" )) {
        ostringstream buf;
        
        buf << "Синтаксис команды {Wcodesource{x:" << endl
            << "Чтение:" << endl
            << "     {Wlist{x [строка]      - показать список сценариев, всех или со строкой в названии" << endl
            << "     {Wread{x <номер>|<имя> - прочитать cs из списка по номеру или названию" << endl
            << "     {Wcopy{x <номер>|<имя> - скопировать cs из списка в буфер редактора" << endl
            << "     {Wsearch{x <строка>    - найти все сценарии, содержащие строку в коде"  << endl
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
            << "     {Wsave all{x             - сохранить все сценарии на диск" << endl;

        ch->send_to( buf );
        return;
    }

    if(cmd.strPrefix("list")) {
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

    if(cmd.strPrefix("search")) {
        ostringstream buf;
        CodeSource::Manager::iterator i;
        static DLString highlight("{R");

        if (args.empty()) {
            ch->pecho("Синтаксис: cs search <строка>");
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
    
    if(cmd.strPrefix("copy")) {
        id_t csid;
        if (!cs_by_number(pch, args, csid))
            return;

        const DLString &content = CodeSource::manager->at(csid).content;
        pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
            ->regs[0].split(content);
        
        ch->pecho("Сценарий %d скопирован в буфер редактора.", csid);
        return;
    }
    
    if(cmd.strPrefix("read")) {
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
    
    if(cmd.strPrefix("save")) {
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

    if(cmd.strPrefix("show")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->pecho("Ты не редактируешь сценарий.");
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
    
    if(cmd.strPrefix("clear")) {
        pch->getAttributes( ).eraseAttribute( "codesource" );
        ch->pecho("Ok.");
        return;
    }
    
    if(cmd.strPrefix("paste")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->pecho("Ты не редактируешь сценарий.");
            return;
        } 
        
        const Editor::reg_t &reg = pch->getAttributes()
            .getAttr<XMLAttributeEditorState>("edstate")->regs[0];
        
        for(Editor::reg_t::const_iterator i = reg.begin(); i != reg.end(); i++)
            csa->content.push_back( DLString( *i ) );

        ch->pecho("Pasted from editor buffer.");
        return;
    }

    if(cmd.strPrefix("load")) {
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

    if(cmd.strPrefix("eval") || cmd.strPrefix("post")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->pecho("Ты не редактируешь сценарий.");
            return;
        } 
        
        CodeSource &cs = CodeSource::manager->allocate();
        
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
    
    if(cmd.strPrefix("web")) {
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
    
    ch->pecho("Неверная подкоманда, используйте {Wcodesource help{x для справки.");
}


