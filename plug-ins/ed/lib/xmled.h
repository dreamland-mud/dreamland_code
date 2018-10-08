/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __XMLED_H__
#define __XMLED_H__

#include "ed.h"
#include "xmlcontainer.h"

class XMLEditor : public Editor, public virtual XMLContainer {
public:
    struct ToXMLState {
        ToXMLState(const Editor &e) : ed(e), lastid(0) { }

        void substpatToXML( XMLNode::Pointer &parent );
        void coreToXML(XMLNode::Pointer& parent);
        void lineToXML( XMLNode::Pointer& parent, const Line * i);
        void undoStackToXML(XMLNode::Pointer& parent);
        void undoListToXML(XMLNode::Pointer& parent, const list<Undo> &u);
        void undoToXML(XMLNode::Pointer& parent, const Undo &u);
        void marksToXML(XMLNode::Pointer& parent);
        void registersToXML(XMLNode::Pointer& parent);
        void iposToXML( XMLNode::Pointer &parent, const DLString &name, const Line * i );
        void iposToXML( XMLNode::Pointer &parent, const Line * i );
        int line2id(const Line * i);

        bool toXML( XMLNode::Pointer & );

    private:
        typedef list<const Line *> core_t;
        typedef map<const Line *, int> l2i_t;

        const Editor &ed;
        int lastid;
        core_t core;
        l2i_t l2i;
    };
    
    struct FromXMLState {
        FromXMLState(Editor &e) : ed(e) { }

        Line * id2line( int i );
        void lineFromXML( const XMLNode::Pointer &parent );
        void coreFromXML( const XMLNode::Pointer &parent );
        void listFromXML( const XMLNode::Pointer &parent, List &l );
        void iposFromXML( const XMLNode::Pointer &parent, ipos_t &i );
        void registerFromXML( const XMLNode::Pointer &parent );
        void registersFromXML( const XMLNode::Pointer &parent );
        void markFromXML( const XMLNode::Pointer &parent );
        void marksFromXML( const XMLNode::Pointer &parent );
        void undoFromXML( const XMLNode::Pointer &parent, Undo &u );
        void changesFromXML( const XMLNode::Pointer &parent, list<Undo> &u );
        void undoCmdFromXML( const XMLNode::Pointer &parent, CmdUndo &u );
        void undoStackFromXML( const XMLNode::Pointer &parent );
        void fromXML( const XMLNode::Pointer &parent );

    private:
        typedef map<int, Line *> i2l_t;

        Editor &ed;
        i2l_t i2l;
    };
    
    virtual bool toXML(XMLNode::Pointer &p) const;
    virtual bool nodeFromXML( const XMLNode::Pointer & );
};

struct XMLEditorRegisters : public map<char, Editor::reg_t>, 
                            public virtual XMLContainer 
{
    typedef Editor::reg_t reg_t;
    virtual bool toXML(XMLNode::Pointer &p) const;
    virtual bool nodeFromXML( const XMLNode::Pointer & );
};


#endif
