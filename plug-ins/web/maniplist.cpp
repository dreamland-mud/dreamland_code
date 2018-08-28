#include <sstream>
#include "maniplist.h"
#include "commandmanager.h"

Manip::Manip( const DLString &cmdName, const DLString &args ) 
{
    this->cmdName = cmdName;
    cmd = commandManager->findExact( cmdName );
    if (cmd.isEmpty( ))
        throw Exception( cmdName + ": manip command not found" );
    this->args = args;
}

DLString Manip::toString( ) const 
{
    return cmd->getRussianName( ) + " " + args;
}

const DLString ManipList::TAG = "m";
const DLString ManipList::ATTR_CMD = "c";
const DLString ManipList::ATTR_LOCAL = "l";
const DLString ManipList::THIS = "$";

ManipList::~ManipList( )
{
}

DLString ManipList::toString( ) const 
{
    ostringstream buf;

    buf << "{Iw<" << TAG << " ";
    
    if (manips.size( ) > 0) {
        buf << ATTR_CMD << "=\"";
        for (list<Manip>::const_iterator m = manips.begin( );
             m != manips.end( );
             m++)
        {
            buf << m->toString( ) << ",";
        }

        buf << "\" ";
    }
   
    if (locals.size( ) > 0) {
        buf << ATTR_LOCAL << "=\"";
        for (list<Manip>::const_iterator m = locals.begin( );
             m != locals.end( );
             m++)
        {
            buf << m->toString( ) << ",";
        }
        buf << "\" ";
    }

    buf << "i=\"" << getID( ) << "\">{Ix"
        << descr << "{Iw</" << TAG << ">{Ix";
    return buf.str( );
}

