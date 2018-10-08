/* $Id: system.h,v 1.16.2.4 2005/08/26 11:23:53 rufina Exp $
 * 
 * ruffina, 2004
 * based on System by NoFate
 */
#ifndef __MOC_SYSTEM_H__
#define __MOC_SYSTEM_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <ostream>
#include <iostream>

using namespace std;

class System
{
public:
        typedef std::vector<string> VectorStringType;
        typedef        std::list<string> ListStringType;
        typedef std::map<string,int> MapType;

        struct VariableXMLType
        {
                string name;
                string        type;
                string        className;
                bool is_Static;
                bool is_Pointer;
                
                inline VariableXMLType( );

                inline void setName( const string& );
                inline const string& getName( ) const;

                inline void setType( const string& );
                inline const string& getType( ) const;
        
                inline void setClassName( const string& );
                inline const string& getClassName( ) const;

                inline void setStatic( bool );
                inline bool isStatic( ) const;
                
                inline void setPointer( );
                inline void unsetPointer( );
                inline bool isPointer( ) const;
        };

        struct NodeType {
                bool has_Variable;
                VariableXMLType variable;

                inline NodeType( );

                inline void setVariable( );
                inline bool hasVariable( ) const;
        };

        struct ClassType
        {
                typedef std::map<string, NodeType>        MapNodeType;

                string name;
                string file;
                System::ListStringType ancestors;
                MapNodeType nodes;
                bool xmlObject;

                inline ClassType( );

                inline bool isXML_OBJECT( ) const;
                inline void setXML_OBJECT( );

                inline void setFile( const string& );
                inline const string& getFile( ) const;

                inline void setName( const string& );
                inline const string& getName( ) const;

                inline void addAncestor( const string& );

                inline void addVariable( const VariableXMLType& );

                inline void destroy( );

                inline ClassType& operator = ( const ClassType& );

        };

        struct MocType
        {
                string str;

                inline void setString( const char* );
                inline void setString( const string& str );
                inline const string& getString( ) const;
        };

        struct IncludeType
        {
                int st;
                void* bufferState;
                string fileName;
                ClassType cl;
                int line;

                inline IncludeType( const ClassType&, const string&, int, int line, void* );

                inline int getST( ) const;
                inline void* getBuffer( );
                inline const string& getFileName( ) const;
                inline ClassType& getClassType( );
        };

public:
        typedef        std::list<ClassType>        ListClassType;
        typedef std::map<string,ClassType> MapClassType;
        typedef std::stack<IncludeType> StackIncludeType;


public:

        System( );

        inline static int getClassLevel( );
        inline static void inClass( );
        inline static void outClass( );

        inline static int getHeaderLevel( );
        inline static void inHeader( );
        inline static void outHeader( );
        inline static void clearHeader( );

        inline static bool isStaticVariable( );
        inline static void setStaticVariable( );
        inline static void unsetStaticVariable( );

        inline static void setCurrentFile( const string& );
        inline static const string& getCurrentFile( );

        inline static ClassType& getCurrentClass( );
        inline static void setCurrentClass( const ClassType& );

        inline static VariableXMLType& getCurrentVariable( );

        inline static void openTemplate( );
        inline static void closeTemplate( );
        inline static int getTemplate( );

        inline static void openTypedef( );
        inline static void closeTypedef( );
        inline static int getTypedef( );

        inline static VectorStringType& getIncludeFiles( );

        inline static string lower( string );

        inline static bool isRoot( );
        inline static void setRoot( );

        static void appendIncludePath( const string& );
        inline static VectorStringType& getIncludePath( );

        inline static void setInputFile( const string& );
        inline static const string& getInputFile( );
        inline static bool isInputFile( );

        inline static void setOutputFile( const string& );
        inline static const string& getOutputFile( );

        static ListClassType& getMocClassList( );

        static MapClassType& getClassMap( );

        static StackIncludeType& getStackInclude( );
        static ListStringType& getUsingNamespaceList( );
        static ListStringType& getNamespaceStack( );
        static ListStringType& getIncludeList( );

        static VectorStringType& getHeadersVector( );
        
        inline static MapType& getNamespaceMap( );
        inline static void namespaceOpen( );
        inline static void namespaceClose( );
        inline static bool isNamespace( );

        // generate.cpp
        static void findAllNodes( ClassType, ClassType::MapNodeType* );
        static void generateMoc( );
        static void generateClass( ostream& output, const ClassType& mc, VectorStringType& ifiles );
        static void generateHeader( VectorStringType& ifiles, VariableXMLType & var );

        // moc.l
        static void setST( int );
        static int getST( );
        static void pushST( );
        static void popST( );
        static void setST_INITIAL( );

private:
        typedef std::stack<int>        StatementStackType;

private:
        static int in_class;
        static int in_header;
        static int templateOpen;
        static int typedefOpen;
        // Defined in moc.l
        static bool staticVariable;
        static string currentFile;
        static ClassType currentClass;
        static VariableXMLType currentVariable;
        static VectorStringType includeFiles;
        static bool root;
        static string inputFile;
        static string outputFile;
        static VectorStringType includePathVector;
        static ListClassType mocClassList;
        static MapClassType classMap;
        static StackIncludeType staticInclude;
        static ListStringType namespaceStack;
        static ListStringType usingNamespaceList;
        static ListStringType includeList;
        static VectorStringType headersVector;
        static int statement;
        static StatementStackType statementStack;
        static MapType namespaceMap;
};





// ------------- System::VariableXMLType-----------------------
 
System::VariableXMLType::VariableXMLType( )
                : is_Static( false ), is_Pointer( false )
{
}

void System::VariableXMLType::setName( const string& name )
{
        this->name = name;
}

const string& System::VariableXMLType::getName( ) const
{
        return name;
}

void System::VariableXMLType::setType( const string& type )
{
        this->type = type;
}

const string& System::VariableXMLType::getType( ) const
{
        return type;
}

void System::VariableXMLType::setClassName( const string& className )
{
        this->className = className;
}

const string& System::VariableXMLType::getClassName( ) const
{
        return className;
}

void System::VariableXMLType::setStatic( bool is_Static )
{
        this->is_Static = is_Static;
}

bool System::VariableXMLType::isStatic( ) const
{
        return is_Static;
}

void System::VariableXMLType::setPointer( )
{
        this->is_Pointer = true;
}

void System::VariableXMLType::unsetPointer( )
{
        this->is_Pointer = false;
}

bool System::VariableXMLType::isPointer( ) const
{
        return is_Pointer;
}

// ------------- System::NodeType-----------------------
System::NodeType::NodeType( )
            :  has_Variable( false )
{
}

void System::NodeType::setVariable( ) 
{
    has_Variable = true;
}

bool System::NodeType::hasVariable( ) const
{
    return has_Variable;
}
// ------------- System::ClassType -----------------------

System::ClassType::ClassType( ) : xmlObject( false )
{
}

bool System::ClassType::isXML_OBJECT( ) const
{
        return xmlObject;
}

void System::ClassType::setXML_OBJECT( )
{
        xmlObject = true;
}

void System::ClassType::setFile( const string& file )
{
        this->file = file;
}

const string& System::ClassType::getFile( ) const
{
        return file;
}

void System::ClassType::setName( const string& name )
{
        this->name = name;
}

const string& System::ClassType::getName( ) const
{
        return name;
}

void error(const string &text);

void System::ClassType::addAncestor( const string& name )
{
    System::MapClassType::iterator ca;
    ListStringType::iterator i;
    string nspace, fqcn;
    
    ca = System::getClassMap( ).find( (fqcn = name) );
    
    for(i = System::getNamespaceStack().begin(), nspace = "";
            i != System::getNamespaceStack().end() &&
            ca == System::getClassMap( ).end(); i++) 
    {
        nspace += *i + "::";
        ca = System::getClassMap( ).find( (fqcn = nspace + name) );
    }
    
    for(i = System::getUsingNamespaceList().begin();
            i != System::getUsingNamespaceList().end() &&
            ca == System::getClassMap( ).end(); i++) 
    {
        ca = System::getClassMap( ).find( (fqcn = *i + "::" + name) );
    }
    
    if(ca != System::getClassMap( ).end()) {
        ancestors.push_back( fqcn );
    } else {
        ancestors.push_back( name );
    }
}

void System::ClassType::addVariable( const VariableXMLType& variable )
{
        NodeType &node = nodes[variable.getName( )];
        
        node.variable = variable;
        node.setVariable( );
}

void System::ClassType::destroy( )
{
        ancestors.clear( );
        nodes.clear( );
        name = "";
        file = "";
        xmlObject = false;
}

System::ClassType& System::ClassType::operator = ( const ClassType& cl )
{
        name = cl.name;
        file = cl.file;
        ancestors = cl.ancestors;
        nodes = cl.nodes;
        xmlObject = cl.xmlObject;
        return *this;
}



// ------------- System::MocType -----------------------


void System::MocType::setString( const char* str )
{
        this->str = str;
}

void System::MocType::setString( const string& str )
{
        this->str = str;
}

const string& System::MocType::getString( ) const
{
        return str;
}



// ------------- System::IncludeType -----------------------

System::IncludeType::IncludeType(
        const ClassType& cl, const string& fileName, int st, int line, void* bufferState
)
                : st( st ), bufferState( bufferState ), fileName( fileName ), cl( cl ), line( line )
{
}

int System::IncludeType::getST( ) const
{
        return st;
}

void* System::IncludeType::getBuffer( )
{
        return bufferState;
}

const string& System::IncludeType::getFileName( ) const
{
        return fileName;
}

System::ClassType& System::IncludeType::getClassType( )
{
        return cl;
}




// ------------- System -----------------------


int System::getClassLevel( )
{
        return in_class;
}

void System::inClass( )
{
        in_class++;
}

void System::outClass( )
{
        in_class--;
}

int System::getHeaderLevel( )
{
        return in_header;
}

void System::inHeader( )
{
        in_header++;
}

inline void System::outHeader( )
{
        in_header--;
}

void System::clearHeader( )
{
        in_header = 0;
}



bool System::isStaticVariable( )
{
        return staticVariable;
}

void System::setStaticVariable( )
{
        staticVariable = true;
}

void System::unsetStaticVariable( )
{
        staticVariable = false;
}



void System::setCurrentFile( const string& currentFile )
{
        System::currentFile = currentFile;
}

const string& System::getCurrentFile( )
{
        return currentFile;
}



System::ClassType& System::getCurrentClass( )
{
        return currentClass;
}

void System::setCurrentClass( const ClassType& currentClass )
{
        System::currentClass = currentClass;
}



System::VariableXMLType& System::getCurrentVariable( )
{
        return currentVariable;
}



void System::openTemplate( )
{
        templateOpen++;
}

void System::closeTemplate( )
{
        templateOpen--;
}

int System::getTemplate( )
{
        return templateOpen;
}


void System::openTypedef( )
{
        typedefOpen++;
}

void System::closeTypedef( )
{
        typedefOpen--;
}

int System::getTypedef( )
{
        return typedefOpen;
}


System::VectorStringType& System::getIncludeFiles( )
{
        return includeFiles;
}



string System::lower( string str )
{
        for( string::size_type i = 0; i < str.length( ); i++ )
        {
                if( str[i] >= 'A' && str[i] <= 'Z' )
                {
                        str[i] += ( 'a' - 'A' );
                }
        }
        return str;
}

bool System::isRoot( )
{
        return root;
}

void System::setRoot( )
{
        root = true;
}

System::VectorStringType& System::getIncludePath( )
{
        return includePathVector;
}


void System::setInputFile( const string& inputFile )
{
        System::inputFile = inputFile;
}

const string& System::getInputFile( )
{
        return inputFile;
}

bool System::isInputFile( )
{
        return !inputFile.empty( );
}



void System::setOutputFile( const string& outputFile )
{
        System::outputFile = outputFile;
}

const string& System::getOutputFile( )
{
        return outputFile;
}



inline System::ListClassType& System::getMocClassList( )
{
        return mocClassList;
}


inline System::MapClassType& System::getClassMap( )
{
        return classMap;
}


inline System::StackIncludeType& System::getStackInclude( )
{
        return staticInclude;
}

inline System::ListStringType& 
System::getUsingNamespaceList( )
{
    return usingNamespaceList;
}

inline System::ListStringType& 
System::getNamespaceStack( )
{
    return namespaceStack;
}


inline System::ListStringType& System::getIncludeList( )
{
        return includeList;
}


inline System::VectorStringType& System::getHeadersVector( )
{
        return headersVector;
}

System::MapType& System::getNamespaceMap( )
{
        return namespaceMap;
}

void System::namespaceOpen( )
{
        MapType::iterator ipos = namespaceMap.find( getCurrentFile( ) );
        if( ipos != namespaceMap.end( ) )
        {
                ipos->second++;
        }
        else
        {
                namespaceMap[getCurrentFile( )] = 1;
        }
}

void System::namespaceClose( )
{
        MapType::iterator ipos = namespaceMap.find( getCurrentFile( ) );
        if( ipos != namespaceMap.end( ) )
        {
                ipos->second--;
        }
}

bool System::isNamespace( )
{
        return namespaceMap[getCurrentFile( )] != 0;
}
        
#endif 
