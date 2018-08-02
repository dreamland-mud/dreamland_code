/* $Id: test.cpp,v 1.1.2.2 2006/01/03 09:50:53 rufina Exp $
 *
 * ruffina, 2004
 */
/* $Id: test.cpp,v 1.1.2.2 2006/01/03 09:50:53 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */
#include <unistd.h>
#include <fstream>
#include <iostream>

using namespace std;
    
#undef yyFlexLexer
#define yyFlexLexer edFlexLexer
#include <FlexLexer.h>

#include "xmled.h"
#include "xmldocument.h"

extern int yydebug;

struct Test : public XMLEditor {
    Test() { eof = false; }

    bool eof;
    
    virtual void print(const string &str) {
	cout << "cout: " << str << endl;
    }
    virtual void error(const string &str) {
	cerr << "cerr: " << str << endl;
    }
    virtual void done() {
	eof = true;
    }
    virtual string shell(const string &cmd, const string &str) {
	cout << "cmd: " << cmd << endl;

	FILE *f = popen(cmd.c_str(), "r+");
	char buf[1024];
	
	if(!f)
	    perror("popen");

	fwrite(str.c_str(), str.size(), 1, f);
	write(1, str.c_str(), str.size());
	buf[fread(buf, 1, sizeof(buf)-1, f)] = 0;
	cout << "buf: " << buf << endl;

	return buf;
    }

    virtual reg_t &registerAt(char r) {
	cout << "get reg " << (int)r << " size: " << regs[r].size( ) << endl;
	return regs[r];
    }

    XMLEditorRegisters regs;
};

int
main(int argc, char **argv)
{
//    yydebug = 1;
    {
	Test ed;

	{
	    ifstream iss("state.xml");
	    if(iss) {
		XMLDocument::Pointer doc(NEW);
		doc->load( iss );
		ed.fromXML( doc->getFirstNode( ) );
	    } else
		cerr << "state.xml not loaded" << endl;

	    iss.close( );
	}
	
	{
	    ifstream iss("regs.xml");
	    if(iss) {
		XMLDocument::Pointer doc(NEW);
		doc->load( iss );
		ed.regs.fromXML( doc->getFirstNode( ) );
	    } else
		cerr << "regs.xml not loaded" << endl;

	    iss.close( );
	}
	
	if(argc > 1) {
	    ifstream is(argv[1]);
	    ed.lines.fromstream(is);
	    
	    ed.currentline = ed.lines.end();
	    ed.currentline--;
	    cout << "Total " << ed.lines.size() << " lines" << endl;
	}
	
	char buf[512];
	while(!ed.eof && cin.getline(buf, sizeof(buf))) {
	    if(!*buf)
		ed.eval(" ");
	    else
		ed.eval(buf);
	}

	{
	    ofstream oss("state.xml");
	    if(oss) {
		XMLDocument::Pointer doc(NEW);
		
		XMLNode::Pointer root(NEW, "root");
		ed.toXML(root);
		doc->appendChild( root );
		
		doc->save( oss );
	    } else
		cerr << "state.xml not saved" << endl;

	    oss.close( );
	}
	
	{
	    ofstream oss("regs.xml");
	    if(oss) {
		XMLDocument::Pointer doc(NEW);
		XMLNode::Pointer regs(NEW, "regs");
		ed.regs.toXML(regs);
		doc->appendChild( regs );
		doc->save( oss );
	    } else
		cerr << "regs.xml not saved" << endl;

	    oss.close( );
	}

    }

    cout << "Lines used: " << Editor::Line::count << endl;
}

