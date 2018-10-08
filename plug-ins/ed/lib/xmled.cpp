/* $Id$
 *
 * ruffina, 2004
 */

#include <iostream>
#include <sstream>

#include "xmled.h"
#include "boolean.h"

using namespace std;

/*------------------------------------------------------------------
 * to xml
 *------------------------------------------------------------------*/
int
XMLEditor::ToXMLState::line2id(const Line *i)
{
    l2i_t::iterator it;

    it = l2i.find(i);
    
    if(it == l2i.end( )) {
        l2i[i] = ++lastid;
        core.push_back(i);
        return lastid;
    }

    return it->second;
}

void 
XMLEditor::ToXMLState::iposToXML( XMLNode::Pointer &parent, const Line * i )
{
    parent->setType( XMLNode::XML_LEAF );

    if(!i)
        return;

    parent->insertAttribute("id", line2id(i));
}

void 
XMLEditor::ToXMLState::iposToXML( XMLNode::Pointer &parent, const DLString &name, const Line * i )
{
    XMLNode::Pointer node( NEW, name );
    iposToXML(node, i);
    parent->appendChild( node );
}
/*
void
XMLEditor::ToXMLState::registersToXML(XMLNode::Pointer& parent)
{
    XMLNode::Pointer node( NEW, "registers" );
    node->setType( XMLNode::XML_NODE );
    
    map<char, list_t>::const_iterator i;
    for(i = ed.registers.begin(); i != ed.registers.end(); i++) {
        XMLNode::Pointer nreg( NEW, "register" );
        iposToXML(nreg, i->second.end( ).data);
        nreg->insertAttribute("name", i->first);
        node->appendChild( nreg );
    }
    parent->appendChild( node );
}
*/
void
XMLEditor::ToXMLState::marksToXML(XMLNode::Pointer& parent)
{
    XMLNode::Pointer node( NEW, "marks" );
    node->setType( XMLNode::XML_NODE );
    
    marks_t::const_iterator i;
    for(i = ed.marks.begin(); i != ed.marks.end(); i++) {
        XMLNode::Pointer nmark( NEW, "mark" );
        iposToXML(nmark, i->second.data);
        nmark->insertAttribute("name", i->first);
        node->appendChild( nmark );
    }
    parent->appendChild( node );
}

void
XMLEditor::ToXMLState::undoToXML(XMLNode::Pointer& parent, const Undo &u)
{
    parent->setType( XMLNode::XML_NODE );
    parent->insertAttribute("add", Boolean(u.add).toString( ));
    
    iposToXML(parent, "head", u.head.data);
    iposToXML(parent, "tail", u.tail.data);
}


void
XMLEditor::ToXMLState::undoListToXML(XMLNode::Pointer& parent, const list<Undo> &u)
{
    XMLNode::Pointer node( NEW, "changes" );
    node->setType( XMLNode::XML_NODE );
    
    list<Undo>::const_iterator i;
    for(i = u.begin(); i != u.end(); i++) {
        XMLNode::Pointer nact( NEW, "change" );
        undoToXML(nact, *i);
        node->appendChild( nact);
    }
    parent->appendChild( node );
}

void
XMLEditor::ToXMLState::undoStackToXML(XMLNode::Pointer& parent)
{
    XMLNode::Pointer node( NEW, "undostack" );
    node->setType( XMLNode::XML_NODE );

    list<CmdUndo>::const_iterator i;
    for(i = ed.undostack.begin(); i != ed.undostack.end(); i++) {
        XMLNode::Pointer nundo( NEW, "undo" );
        nundo->setType( XMLNode::XML_NODE );
        undoListToXML(nundo, i->changes);
        iposToXML(nundo, "currentline", i->currentline.data);
        node->appendChild( nundo );
    }
    parent->appendChild( node );
}

void
XMLEditor::ToXMLState::lineToXML( XMLNode::Pointer& parent, const Line * i)
{
    XMLNode::Pointer node( NEW, "line" );
    node->setType( XMLNode::XML_LEAF );
    
    node->insertAttribute("id", line2id(i));
    node->insertAttribute("prev", line2id(i->prev));
    node->insertAttribute("next", line2id(i->next));
    /*safe place to store leading/trailing spaces*/
    node->insertAttribute("text", i->str);

    parent->appendChild( node );
}

void
XMLEditor::ToXMLState::coreToXML(XMLNode::Pointer& parent)
{
    XMLNode::Pointer node( NEW, "core" );
    node->setType( XMLNode::XML_NODE );

    core_t::iterator i;
    for(i = core.begin(); i != core.end(); i++) 
        lineToXML(node, *i);

    parent->appendChild( node );
}

void
XMLEditor::ToXMLState::substpatToXML( XMLNode::Pointer &parent )
{
    XMLNode::Pointer node;
    
    node.construct( );
    node->setType( XMLNode::XML_NODE );
    node->setName( "lastsubst" );
    node->insertAttribute("pat", ed.lastsubst.pat);
    node->insertAttribute("rep", ed.lastsubst.rep);
    parent->appendChild( node );
    node.clear( );
    
    node.construct( );
    node->setType( XMLNode::XML_NODE );
    node->setName( "lastpat" );
    node->insertAttribute("pat", ed.lastpat);
    parent->appendChild( node );
    node.clear( );
}

bool
XMLEditor::ToXMLState::toXML( XMLNode::Pointer &node )
{
    lastid = 0;
    core.clear( );
    l2i.clear( );

    iposToXML(node, "lines", ed.lines.end( ).data);
    iposToXML(node, "appendlines", ed.appendlines.end( ).data);
    iposToXML(node, "currentline", ed.currentline.data);
    iposToXML(node, "append_at", ed.append_at.data);

//    registersToXML( node );
    marksToXML( node );
    undoStackToXML( node );
    undoListToXML( node, ed.undo );
    substpatToXML( node );

    /*this must be the last node*/
    coreToXML( node );

    return true;
}

bool
XMLEditor::toXML(XMLNode::Pointer &parent) const
{
    XMLNode::Pointer node( NEW, "state" );
    node->setType( XMLNode::XML_NODE );
    ToXMLState s(*this);
    s.toXML( node );
    parent->appendChild( node );
    return true;
}

/*------------------------------------------------------------------
 * from xml
 *------------------------------------------------------------------*/
Editor::Line *
XMLEditor::FromXMLState::id2line( int i )
{
    i2l_t::iterator it;

    it = i2l.find( i );

    if(it == i2l.end( )) 
        return i2l[i] = new Line;

    return it->second;
}

void
XMLEditor::FromXMLState::lineFromXML( const XMLNode::Pointer &parent )
{
    Line *me = id2line(parent->getAttribute("id").toInt( ));
    me->prev = id2line(parent->getAttribute("prev").toInt( ));
    me->next = id2line(parent->getAttribute("next").toInt( ));
    me->str = parent->getAttribute("text");
}

void
XMLEditor::FromXMLState::coreFromXML( const XMLNode::Pointer &parent )
{
    const XMLNode::NodeList &nl = parent->getNodeList( );

    XMLNode::NodeList::const_iterator it;
    for(it = nl.begin( ); it != nl.end( ); it++)
        lineFromXML(*it);
}

void
XMLEditor::FromXMLState::listFromXML( const XMLNode::Pointer &parent, List &l )
{
    Line *ln = id2line(parent->getAttribute("id").toInt( ));

    l.clear( );
    delete l.data;
    l.data = ln;
}

void
XMLEditor::FromXMLState::iposFromXML( const XMLNode::Pointer &parent, ipos_t &i )
{
    const DLString &id = parent->getAttribute("id");

    if(id.empty( ))
        i.data = 0;
    else
        i.data = id2line(id.toInt( ));
    
}

/*
void
XMLEditor::FromXMLState::registerFromXML( const XMLNode::Pointer &parent )
{
    listFromXML(parent, ed.registers[parent->getAttribute("name").toInt( )]);
}

void
XMLEditor::FromXMLState::registersFromXML( const XMLNode::Pointer &parent )
{
    const XMLNode::NodeList &nl = parent->getNodeList( );

    XMLNode::NodeList::const_iterator it;
    for(it = nl.begin( ); it != nl.end( ); it++)
        registerFromXML(*it);
}
*/
void
XMLEditor::FromXMLState::markFromXML( const XMLNode::Pointer &parent )
{
    iposFromXML(parent, ed.marks[parent->getAttribute("name").toInt( )]);
}

void
XMLEditor::FromXMLState::marksFromXML( const XMLNode::Pointer &parent )
{
    const XMLNode::NodeList &nl = parent->getNodeList( );

    XMLNode::NodeList::const_iterator it;
    for(it = nl.begin( ); it != nl.end( ); it++)
        markFromXML(*it);
}

void
XMLEditor::FromXMLState::undoFromXML( const XMLNode::Pointer &parent, Undo &u )
{
    /*
    if(parent->getType() != XMLNode::XML_NODE || parent->getName() != "change")
        throw XXX;
    */
    const XMLNode::NodeList &nl = parent->getNodeList( );

    XMLNode::NodeList::const_iterator it;
    for(it = nl.begin( ); it != nl.end( ); it++) {
        XMLNode::Pointer node = *it;
        /*
        if(node->getType() != XMLNode::XML_NODE)
            throw XXX;
        */
        if(node->getName() == "head")
            iposFromXML(node, u.head);
        else if(node->getName() == "tail")
            iposFromXML(node, u.tail);
        /*
        else
            throw XXX;
         */
    }
    
    u.add = parent->getAttribute("add").toBoolean( );
}

void
XMLEditor::FromXMLState::changesFromXML( const XMLNode::Pointer &parent, list<Undo> &u )
{
    /*
    if(parent->getType() != XMLNode::XML_NODE || parent->getName() != "changes")
        throw XXX;
    */
    const XMLNode::NodeList &nl = parent->getNodeList( );
    XMLNode::NodeList::const_iterator it;

    u.clear( );
    for(it = nl.begin( ); it != nl.end( ); it++) {
        Undo au;
        undoFromXML( *it, au );
        u.push_back( au );
    }
}

void
XMLEditor::FromXMLState::undoCmdFromXML( const XMLNode::Pointer &parent, CmdUndo &u )
{
    /*
    if(parent->getType() != XMLNode::XML_NODE || parent->getName() != "undo")
        throw XXX;
    */
    const XMLNode::NodeList &nl = parent->getNodeList( );
    XMLNode::NodeList::const_iterator it;
    
    for(it = nl.begin( ); it != nl.end( ); it++) {
        XMLNode::Pointer node = *it;
        
        if(node->getName( ) == "changes") 
            changesFromXML(node, u.changes);
        else if(node->getName( ) == "currentline")
            iposFromXML(node, u.currentline);
    }
}

void
XMLEditor::FromXMLState::undoStackFromXML( const XMLNode::Pointer &parent )
{
    /*
    if(parent->getType() != XMLNode::XML_NODE || parent->getName() != "undostack")
        throw XXX;
    */
    const XMLNode::NodeList &nl = parent->getNodeList( );
    XMLNode::NodeList::const_iterator it;
    
    ed.undostack.clear( );
    for(it = nl.begin( ); it != nl.end( ); it++) {
        CmdUndo auc;
        undoCmdFromXML(*it, auc);
        ed.undostack.push_back( auc );
    }
}

void
XMLEditor::FromXMLState::fromXML( const XMLNode::Pointer &parent )
{
    ed.clear( );
    i2l.clear( );

    const XMLNode::NodeList &nl = parent->getNodeList( );

    XMLNode::NodeList::const_iterator it;
    for(it = nl.begin( ); it != nl.end( ); it++) {
        XMLNode::Pointer node = *it;
        /*
        if(node->getType() != XMLNode::XML_NODE)
            throw XXX;
        */
        if(node->getName() == "core")
            coreFromXML(node);
        else if(node->getName() == "changes")
            changesFromXML(node, ed.undo);
        else if(node->getName() == "undostack")
            undoStackFromXML(node);
        else if(node->getName() == "marks")
            marksFromXML(node);
//        else if(node->getName() == "registers")
//            registersFromXML(node);
        else if(node->getName() == "append_at")
            iposFromXML(node, ed.append_at);
        else if(node->getName() == "currentline")
            iposFromXML(node, ed.currentline);
        else if(node->getName() == "appendlines")
            listFromXML(node, ed.appendlines);
        else if(node->getName() == "lines")
            listFromXML(node, ed.lines);
        else if(node->getName() == "lastpat") 
            ed.lastpat = node->getAttribute("pat");
        else if(node->getName() == "lastsubst") {
            ed.lastsubst.pat = node->getAttribute("pat");
            ed.lastsubst.rep = node->getAttribute("rep");
        }
        /*
        else
            throw XXX;
         */
    }

    i2l_t::iterator mi;
    for(mi = i2l.begin( ); mi != i2l.end( ); mi++)
        if(!mi->second->prev || !mi->second->next)
            throw Exception("ed: broken state loaded");
}

bool
XMLEditor::nodeFromXML( const XMLNode::Pointer &parent )
{
    if(parent->getType() == XMLNode::XML_NODE 
            && parent->getName() == "state") 
    {
        FromXMLState s(*this);
        s.fromXML( parent );
        return true;
    }

    return false;
}

/*------------------------------------------------------------------
 * regs to/from xml
 *------------------------------------------------------------------*/
bool
XMLEditorRegisters::toXML(XMLNode::Pointer &parent) const
{
    const_iterator i;

    for(i = begin(); i != end(); i++) {
        XMLNode::Pointer nreg( NEW, "reg" );
        nreg->setType( XMLNode::XML_NODE );
        nreg->insertAttribute("name", i->first);

        ostringstream ostr;
        reg_t::const_iterator li;
        for(li = i->second.begin( ); li != i->second.end( ); li++)
            ostr << *li << endl;

        XMLNode::Pointer cdata( NEW, "'" + ostr.str( ) + "'" );
        cdata->setType( XMLNode::XML_TEXT );
        nreg->appendChild( cdata );

        parent->appendChild( nreg );
    }

    return true;
}

bool
XMLEditorRegisters::nodeFromXML( const XMLNode::Pointer &parent )
{
    if(parent->getType() != XMLNode::XML_NODE || parent->getName() != "reg") 
        return false;

    XMLNode::Pointer cdata = parent->getFirstNode( );

    if(!cdata)
        return false;

    DLString cd = cdata->getCData();

    if(cd.size( ) >= 2 && cd[0] == '\'' && cd[cd.length( ) - 1] == '\'')
        cd = cd.substr(1, cd.length() - 2);

    (*this)[parent->getAttribute("name").toInt( )].split(cd);
    
    return true;
}

