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

#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "xmlattributecodesource.h"
#include "wrappermanager.h"

#include "xmleditorinputhandler.h"

#include "comm.h"
#include "dl_ctype.h"
#include "def.h"

using namespace Scripting;
using namespace std;

bool has_fenia_security( PCharacter *pch );

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
	    << "     {Wlist{x         - показать список всех существующих codesources" << endl
	    << "     {Wread{x <номер> - прочитать cs по номеру из списка" << endl
	    << "     {Wcopy{x <номер> - скопировать cs из списка в буфер редактора" << endl
            << endl
            << "Редактирование:" << endl
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
	    sprintf( header, "[%5u] {g%8s{x: %-36s {D(%lu функц.){x\r\n", 
		    i->getId( ), 
		    i->author.c_str( ), 
		    i->name.c_str( ),
		    i->functions.size( ));
            buf << header;
	}
	page_to_char(buf.str().c_str(), ch);
	return;
    }
    
    if(cmd.strPrefix("copy")) {
	CodeSource::Manager::iterator i;
	
	try {
	    i = CodeSource::manager->find( args.toInt( ) );
	} catch( ... ) {
	    return;
	}

	if(i == CodeSource::manager->end( ) ) {
	    page_to_char( "no such CodeSource\r\n", ch );
	    return;
	}

	pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
	    ->regs[0].split(i->content);
	
	page_to_char("Codesource copyied to the editor buffer\r\n", ch);
	return;
    }
    
    if(cmd.strPrefix("read")) {
	CodeSource::Manager::iterator i;
	
	try {
	    i = CodeSource::manager->find( args.toInt( ) );
	} catch( ... ) {
	    return;
	}

	if(i == CodeSource::manager->end( ) ) {
	    page_to_char( "no such CodeSource\r\n", ch );
	    return;
	}
	
	char buf[MAX_STRING_LENGTH];
	sprintf( buf, "[%u] {g%s{x: %s\r\n", 
		    i->getId(), 
		    i->author.c_str( ),
		    i->name.c_str( ));
	page_to_char(buf, ch);
	
	ostringstream ostr;
	string::const_iterator c;
	for(c = i->content.begin( ); c != i->content.end( ); c++ ) 
	{
	    if(*c == '{')
		ostr << '{';
		
	    ostr << *c;
	}
	page_to_char(ostr.str( ).c_str( ), ch);
	return;
    }
    
    if(cmd.strPrefix("show")) {
	XMLAttributeCodeSource::Pointer csa = pch->getAttributes( 
			).findAttr<XMLAttributeCodeSource>( "codesource" );

	if(!csa) {
	    page_to_char( "you do not edit any CodeSource\r\n", ch );
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
	    page_to_char( "you do not edit any CodeSource\r\n", ch );
	    return;
	} 
	
	if(csa->content.empty( )) {
	    ch->send_to( "no more lines left\r\n" );
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
	    page_to_char( "you do not edit any CodeSource\r\n", ch );
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
	    page_to_char( "you do not edit any CodeSource\r\n", ch );
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
	    page_to_char( "you do not edit any CodeSource\r\n", ch );
	    return;
	} 
		
	ifstream ifs(args.c_str( ));
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
	    page_to_char( "you do not edit any CodeSource\r\n", ch );
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
	    page_to_char( "you do not edit any CodeSource\r\n", ch );
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
            CodeSource::Manager::iterator i;

            try {
                i = CodeSource::manager->find( args.toInt( ) );
            } catch( ... ) {
                page_to_char( "no such CodeSource\r\n", ch );
                return;
            }

            if(i == CodeSource::manager->end( ) ) {
                page_to_char( "no such CodeSource\r\n", ch );
                return;
            }

            parms.resize(2);
            parms[0] = i->name;
            parms[1] = i->content;
        }

        ch->desc->writeWSCommand("cs_edit", parms);
	return;
    }
    
    ch->println("Неверная подкоманда, используйте {Wcodesource help{x для справки.");
}


