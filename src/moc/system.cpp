/* $Id: system.cpp,v 1.10.2.2.24.1 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, 2004
 */
/*
 * NoFate, 2001
 */
// system.cpp: implementation of the MOCSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "system.h"
#include "mocfunctional.h"

int System::in_class = 0;
int System::in_header = 0;
int System::templateOpen = 0;
int System::typedefOpen = 0;
bool System::staticVariable = false;
string System::currentFile = "";
System::ClassType System::currentClass;
System::VariableXMLType System::currentVariable;
System::VectorStringType System::includeFiles;
bool System::root = false;
string System::inputFile;
string System::outputFile = "moc_xml.cpp";
System::ListClassType mocClassList;
System::StackIncludeType System::staticInclude;
System::MapClassType System::classMap;
System::ListClassType System::mocClassList;
System::ListStringType System::includeList;
System::ListStringType System::usingNamespaceList;
System::ListStringType System::namespaceStack;
System::VectorStringType System::includePathVector;
System::VectorStringType System::headersVector;
System::StatementStackType System::statementStack;
System::MapType System::namespaceMap;



void System::appendIncludePath( const string& include )
{
	includePathVector = toList<VectorStringType>( include, ':' );
}
