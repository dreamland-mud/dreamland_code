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

#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "xmlattributecodesource.h"
#include "wrappermanager.h"

#include "xmleditorinputhandler.h"

#include "comm.h"
#include "act.h"
#include "dl_match.h"    
#include "dl_ctype.h"
#include "dl_strings.h"
#include "def.h"

using namespace Scripting;
using namespace std;

bool has_fenia_security( PCharacter *pch );

static bool cs_by_subj(PCharacter *ch, const DLString &arg, id_t &csid)
{
    CodeSource::Manager::const_iterator i;
    for (i = CodeSource::manager->begin( );i != CodeSource::manager->end( ); i++) {
        if (dl_match(arg.c_str(), i->name.c_str(), true)) {
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
        ch->println("Ты не ботаешь по фене.");
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
            << "     {Wsubj{x <текст> - указать тему cs; удобно тут же указывать версию" << endl
            << "     {Wbin{x <id>     - прочесть текст cs из https://pastebin.com/<id>" << endl
            << "     {Wpaste{x        - вставить текст cs из буфера редактора" << endl
            << "     {W+ {x<строка>   - добавить одну строку в текст cs" << endl
            << "     {W- {x           - удалить последнюю строку из текста cs" << endl
            << "     {Wshow{x         - показать редактируемый cs" << endl
            << "     {Wclear{x        - очистить редактируемый cs" << endl
            << endl
            << "Выполнение:" << endl
            << "     {Wpost{x         - выполнить cs, далее можно делать eval blablaInit() для объявленной там функции" << endl;

        if (ch->isCoder( ))
            buf << endl
            << "Только для кодеров: " << endl
            << "     {Wftp{x <имя_файла_без_расширения> - прочитать текст cs из файла на ftp. файл должен лежать в подкаталоге progs и иметь расширение .f++" << endl
            << "     {Wfile{x <файл>  - прочитать текст cs из файла в корневом каталоге проекта" << endl;
        ch->send_to( buf );
        return;
    }

    if(cmd.strPrefix("list")) {
        ostringstream buf;
        char header[MAX_STRING_LENGTH];
        CodeSource::Manager::iterator i;

        buf << "{YСценарии{x (всего " << CodeSource::manager->size() << "):" << endl;
        sprintf(header, "{W[%5s] %8s  %-36s %s{x\r\n", "Номер", "Автор:", "Название", "Используется функций");
        buf << header;

        for(i = CodeSource::manager->begin( );i != CodeSource::manager->end( ); i++) {
            if (args.empty() 
                || is_name(args.c_str(), i->name.c_str())
                || dl_match(args.c_str(), i->name.c_str(), true)) {
                sprintf( header, "[%5u] {g%8s{x: %-36s {D(%lu функц.){x\r\n", 
                        i->getId( ), 
                        i->author.c_str( ), 
                        i->name.c_str( ),
                        i->functions.size( ));
                buf << header;
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
            ch->println("Синтаксис: cs search <строка>");
            return;
        }

        buf << "Сценарии, содержащие '" << args << "':" << endl;
        
        for(i = CodeSource::manager->begin( );i != CodeSource::manager->end( ); i++) {
            ostringstream matchBuf;
            stringstream lines(i->content);
            DLString line;            
            int num = 0;

            while (std::getline(lines, line, '\n')) {
                num++;
                if (line.find(args) != string::npos) {
                    line.replaces(args, highlight + args + "{x");
                    matchBuf << dlprintf("{D%3d{x ", num) << line << "\r\n";
                }
            }

            DLString match = matchBuf.str();
            if (!match.empty()) {
                buf << dlprintf("[%5u] {g%8s{x: {G%-36s {D(%d функц.){x",
                                i->getId(), i->author.c_str(), i->name.c_str(), i->functions.size())
                    << endl
                    << match
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
        ch->printf( "[%u] {g%s{x: %s\r\n", 
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
    
    if(cmd.strPrefix("write")) {
        if (!pch->isCoder( )) {
            ch->send_to( "Check your privilege.\r\n");
            return;
        }

        id_t csid;
        if (!cs_by_number(pch, args, csid))
            return;

        CodeSource &cs = CodeSource::manager->at(csid);               
        DLString filecontent = cs.content;
        DLDirectory dir( dreamland->getTableDir( ), "fenia.local" );
        DLFileStream( dir, cs.name, ".f++" ).fromString( filecontent );
        ch->printf("Codesource %d is saved as  %s/%s.f++.\r\n",  
                     cs.getId(), dir.getAbsolutePath().c_str(), cs.name.c_str());
        return;
    }

    if(cmd.strPrefix("show")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->println("Ты не редактируешь сценарий.");
            return;
        } 
        
        char buf[MAX_STRING_LENGTH];
        sprintf( buf, "%s: %s\r\n", 
                        pch->getNameP( ), 
                        csa->name.getValue( ).c_str( ));
        page_to_char(buf, ch);
        
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

    if(cmd.strPrefix("subject")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).getAttr<XMLAttributeCodeSource>( "codesource" );
        
        csa->name = args;

        ch->send_to("Ok.\r\n");
        return;
    }
    
    if(cmd.strPrefix("clear")) {
        pch->getAttributes( ).eraseAttribute( "codesource" );
        ch->send_to("Ok.\r\n");
        return;
    }

    if(cmd[0] == '-') {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->println("Ты не редактируешь сценарий.");
            return;
        } 
        
        if(csa->content.empty( )) {
            ch->send_to( "No more lines left\r\n" );
            return;
        }

        csa->content.pop_back( );
        ch->send_to("Ok.\r\n");
        return;
    }
    
    if(cmd[0] == '+') {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->println("Ты не редактируешь сценарий.");
            return;
        } 
        
        csa->content.push_back( args );
        ch->send_to("Ok.\r\n");
        return;
    }
    
    if(cmd.strPrefix("paste")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->println("Ты не редактируешь сценарий.");
            return;
        } 
        
        const Editor::reg_t &reg = pch->getAttributes()
            .getAttr<XMLAttributeEditorState>("edstate")->regs[0];
        
        for(Editor::reg_t::const_iterator i = reg.begin(); i != reg.end(); i++)
            csa->content.push_back( DLString( *i ) );

        ch->send_to("Pasted from editor buffer.\r\n");
        return;
    }

    if(cmd.strPrefix("file")) {
        if (!pch->isCoder( )) {
            ch->send_to( "This is not for you.\r\n");
            return;
        }
        
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );
        
        if(!csa) {
            ch->println("Ты не редактируешь сценарий.");
            return;
        } 
                
        ifstream ifs(args.c_str( ));
        if(!ifs) {
            ch->send_to("Open error.\r\n");
            return;
        }
        
        ch->send_to("Reading from file.\r\n");
        
        while(!ifs.eof()) {
            char buf[MAX_STRING_LENGTH];

            ifs.getline(buf, sizeof(buf));

            csa->content.push_back( buf );
        }
        
        ifs.close();
        
        ch->send_to("Ok.\r\n");
        return;
    }

    if(cmd.strPrefix("bin")) {
        DLString binName = args.getOneArgument();
        
        if (binName.empty( )) {
            ch->println( "Укажи имя файла на pastebin.com." );
            return;
        }

        for (DLString::size_type b = 0; b < binName.size( ); b++)
            if (!dl_isalnum( binName.at( b ) )) {
                ch->println("Имя файла должно состоять из букв и цифр.");
                return;
            }
        
        if(::system(("/home/dltest/runtime/bin/fetchbin " + binName).c_str()) != 0) {
            ch->println("Ошибка при попытке загрузить файл с pastebin.com");
            return;
        }

        args = "bins/" + binName;
        cmd = "ftp";
    }

    if(cmd.strPrefix("ftp")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->println("Ты не редактируешь сценарий.");
            return;
        } 
        
        if (args.find('.') != DLString::npos) {
            ch->send_to("Dots and slashes not allowed in filename.\r\n");
            return;
        }
        
        DLString name = "/home/fenia/ftp_root/" + args + ".f++";

        ifstream ifs(name.c_str( ));
        if(!ifs) {
            ch->send_to("open error\r\n");
            return;
        }
        
        ch->send_to("Reading from file.\r\n");
        
        while(!ifs.eof()) {
            char buf[MAX_STRING_LENGTH];

            ifs.getline(buf, sizeof(buf));

            csa->content.push_back( buf );
        }
        
        ifs.close();
        
        ch->send_to("Ok.\r\n");
        return;
    }
    
    if(cmd.strPrefix("eval") || cmd.strPrefix("post")) {
        XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
                        ).findAttr<XMLAttributeCodeSource>( "codesource" );

        if(!csa) {
            ch->println("Ты не редактируешь сценарий.");
            return;
        } 
        
        CodeSource &cs = CodeSource::manager->allocate();
        
        cs.author = pch->getNameP( );
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
            ch->send_to("Ok.\r\n");
        } catch( ::Exception e ) {
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
    
    ch->println("Неверная подкоманда, используйте {Wcodesource help{x для справки.");
}


