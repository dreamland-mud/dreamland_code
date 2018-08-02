/* $Id: ref-tree.h,v 1.1.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: ref-tree.h,v 1.1.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __REF_TREE_H__
#define __REF_TREE_H__

#include "nodes.h"
#include "scope.h"
#include "lex.h"
#include "xmlregister.h"
#include "codesource.h"

namespace Scripting {

/* References */

class DefaultRef : public ReferenceNode {
public:
    typedef ::Pointer<DefaultRef> Pointer;
    
    DefaultRef();
    DefaultRef(Lex::id_t i);
    virtual ~DefaultRef();

    virtual Reference evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
    
    Lex::id_t key;
};

class HashRef : public ReferenceNode {
public:
    typedef ::Pointer<HashRef> Pointer;
    
    HashRef();
    HashRef(ExpNode::Pointer e, ExpNode::Pointer k);
    virtual ~HashRef();

    virtual Reference evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
    
    ExpNode::Pointer exp;
    ExpNode::Pointer key;
};

class MemberRef : public ReferenceNode {
public:
    typedef ::Pointer<MemberRef> Pointer;
    
    MemberRef();
    MemberRef(ExpNode::Pointer e, Lex::id_t k);
    virtual ~MemberRef();

    virtual Reference evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
    
    ExpNode::Pointer exp;
    Register key;
};

class RootRef : public ReferenceNode {
public:
    typedef ::Pointer<RootRef> Pointer;
    
    RootRef();
    RootRef(Lex::id_t k);
    virtual ~RootRef();

    virtual Reference evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
    
    Register key;
};

} 

#endif /* __REF_TREE_H__ */
