/* $Id$
 *
 * ruffina, 2004
 */

#include <sstream>

#include "xmlmisc.h"
#include "autoflags.h"
#include "affectflags.h"
#include "logstream.h"
#include "affect.h"
#include "room.h"
#include "merc.h"
#include "mercdb.h"

using namespace std;

GSN(none);

/*********************************************************************
 * XMLOptionalString
 *********************************************************************/
XMLOptionalString::XMLOptionalString( ) : _set(false)
{
}

void
XMLOptionalString::set(bool b)
{
    _set = b;
}

bool
XMLOptionalString::set( ) const
{
    return _set;
}

void 
XMLOptionalString::fromXML(const XMLNode::Pointer &parent) throw(ExceptionBadType)
{
    set(true);
    XMLString::fromXML(parent);
}

bool
XMLOptionalString::toXML(XMLNode::Pointer &parent) const
{
    if(!set( ))
        return false;
    
    return XMLString::toXML(parent);
}

/*********************************************************************
 * XMLArmor
 *********************************************************************/
XMLArmor::XMLArmor( )
{
    pierce = 0;
    bash = 0;
    slash = 0;
    exotic = 0;
}

void
XMLArmor::init(int p, int b, int s, int e)
{
    pierce = p;
    bash = b;
    slash = s;
    exotic = e;
}

void 
XMLArmor::fromXML(const XMLNode::Pointer &parent) throw(ExceptionBadType)
{
    pierce = parent->getAttribute("pierce").toInt( );
    bash = parent->getAttribute("bash").toInt( );
    slash = parent->getAttribute("slash").toInt( );
    exotic = parent->getAttribute("exotic").toInt( );
}

bool
XMLArmor::toXML(XMLNode::Pointer &parent) const
{
    parent->setType(XMLNode::XML_LEAF);
    parent->insertAttribute("pierce", pierce);
    parent->insertAttribute("bash", bash);
    parent->insertAttribute("slash", slash);
    parent->insertAttribute("exotic", exotic);
    return true;
}

/*********************************************************************
 * XMLFlagsDiff
 *********************************************************************/
XMLFlagsDiff::XMLFlagsDiff(const FlagTable *t)
{
    table = t;
    add = del = 0;
}

bitstring_t
XMLFlagsDiff::get(bitstring_t base)
{
    return add | (base & ~del);
}

void
XMLFlagsDiff::setRealBase(bitstring_t r, bitstring_t b)
{
    setAddDel(r & ~b, ~r & b);
}

void
XMLFlagsDiff::setAddDel(bitstring_t a, bitstring_t d)
{
    add = a;
    del = d;
}


void
XMLFlagsDiff::fromXML(const XMLNode::Pointer &parent) throw(ExceptionBadType)
{
    add = table->bitstring(parent->getAttribute("add"));
    del = table->bitstring(parent->getAttribute("del"));

    if(add == NO_FLAG)
        add = 0;

    if(del == NO_FLAG)
        del = 0;
}

bool
XMLFlagsDiff::toXML(XMLNode::Pointer &parent) const
{
    if(add == 0 && del == 0)
        return false;

    parent->setType(XMLNode::XML_LEAF);

    if(add)
        parent->insertAttribute("add", table->names(add));
    
    if(del)
        parent->insertAttribute("del", table->names(del));
    
    return true;
}

/*********************************************************************
 * XMLDice
 *********************************************************************/
void
XMLDice::set(int n = 0, int t = 0, int b = 0)
{
    number = n;
    type = t;
    bonus = b;
}

void 
XMLDice::fromXML(const XMLNode::Pointer &parent) throw(ExceptionBadType)
{
    XMLNode::Pointer node = parent->getFirstNode( );

    number = type = bonus = 0;

    if(!node)
        return;

    if(sscanf(node->getCData( ).c_str( ), " %d d %d + %d ", 
            &number, &type, &bonus) != 3)
        throw ExceptionBadType("XMLDice", "illegal format");
}

bool
XMLDice::toXML(XMLNode::Pointer &parent) const
{
    XMLNode::Pointer node( NEW );
    ostringstream os;

    node->setType( XMLNode::XML_TEXT );
    os << number << "d" << type << "+" << bonus;
    node->setCData( os.str( ) );
    parent->appendChild( node );
    return true;
}

/*********************************************************************
 * XMLExtraDescr
 *********************************************************************/
bool
XMLExtraDescr::toXML(XMLNode::Pointer &parent) const
{
    if(!XMLString::toXML(parent))
        return false;

    parent->insertAttribute("keyword", keyword);
    return true;
}

void 
XMLExtraDescr::fromXML(const XMLNode::Pointer &parent) throw(ExceptionBadType)
{
    keyword = parent->getAttribute("keyword");
    XMLString::fromXML(parent);
}

/*********************************************************************
 * XMLApply
 *********************************************************************/
XMLApply::XMLApply( ) : where(APPLY_NONE)
{
}

bool
XMLApply::toXML(XMLNode::Pointer &parent) const
{
    if(where == APPLY_NONE)
        return false;

    if(!XMLIntegerNoEmpty::toXML(parent))
        return false;

    parent->insertAttribute("to", apply_flags.name(where));
    return true;
}

void 
XMLApply::fromXML(const XMLNode::Pointer &parent) throw(ExceptionBadType)
{
    where = apply_flags.value(parent->getAttribute("to"));
    XMLIntegerNoEmpty::fromXML(parent);
}

/*********************************************************************
 * XMLAffect
 *********************************************************************/
void
XMLAffect::init(Affect *pAf)
{
    switch(pAf->where) {
    case TO_AFFECTS:
        bits.setTable(&affect_flags);
        break;
    case TO_IMMUNE:
        bits.setTable(&imm_flags);
        break;
    case TO_RESIST:
        bits.setTable(&res_flags);
        break;
    case TO_VULN:
        bits.setTable(&vuln_flags);
        break;
    case TO_DETECTS:
        bits.setTable(&detect_flags);
        break;
    default:
        LogStream::sendError() << "incorrect pAf->where in area file" << endl;
    case TO_OBJECT:
        bits.setTable(NULL);
    }
    bits.setValue(pAf->bitvector);
    apply.where = pAf->location;
    apply.setValue(pAf->modifier);
}

Affect *
XMLAffect::compat()
{
    Affect *paf = dallocate( Affect );
    
    paf->type.assign(gsn_none);
    paf->where = TO_OBJECT;
    paf->duration = -1;
    
    if(bits.getTable( ) == NULL) 
        ;
    else if(bits.getTable( ) == &affect_flags)
        paf->where = TO_AFFECTS;
    else if(bits.getTable( ) == &imm_flags)
        paf->where = TO_IMMUNE;
    else if(bits.getTable( ) == &res_flags)
        paf->where = TO_RESIST;
    else if(bits.getTable( ) == &vuln_flags)
        paf->where = TO_VULN;
    else if(bits.getTable( ) == &detect_flags)
        paf->where = TO_DETECTS;
    else
        LogStream::sendError() << "incorrect pAf->where in area file" << endl;

    paf->bitvector = bits.getValue( );
    
    paf->location = apply.where;
    paf->modifier = apply.getValue( );

    return paf;
}

/*********************************************************************
 * exits
 *********************************************************************/
XMLExitBase::XMLExitBase() : flags(0, &exit_flags)
{
}


void 
XMLExitDir::init(const exit_data *ex)
{
    keyword.setValue(ex->keyword);
    if (ex->short_descr)
        short_descr.setValue(ex->short_descr);
    description.setValue(ex->description);
    flags.setValue(ex->exit_info_default);
    //if(ex->key > 0)
        key.setValue(ex->key);
    target.setValue(ex->u1.to_room ? ex->u1.to_room->vnum : -1);
}

exit_data *
XMLExitDir::compat( )
{
    EXIT_DATA *pexit = (EXIT_DATA*)alloc_perm(sizeof(EXIT_DATA));
    
    pexit->keyword = str_dup(keyword.getValue( ).c_str( ));
    pexit->short_descr = str_dup(short_descr.c_str());
    pexit->description = str_dup(description.getValue( ).c_str( ));
    pexit->exit_info_default = pexit->exit_info = flags.getValue( );
    pexit->key = key.getValue( );
    pexit->u1.vnum = target.getValue( );
    pexit->level = 0;
    
    return pexit;
}

XMLExtraExit::XMLExtraExit( ) : max_size_pass(SIZE_GARGANTUAN, &size_table)
{
}

void 
XMLExtraExit::init(const extra_exit_data *ex)
{
    description.setValue(ex->description);
    flags.setValue(ex->exit_info_default);
    //if(ex->key > 0)
        key.setValue(ex->key);
    target.setValue(ex->u1.to_room ? ex->u1.to_room->vnum : -1);

    short_desc_from.setValue(ex->short_desc_from);
    short_desc_to.setValue(ex->short_desc_to);
    room_description.setValue(ex->room_description);
    max_size_pass.setValue(ex->max_size_pass);
    moving_from.setValue(ex->moving_from);
    moving_mode_from.setValue(ex->moving_mode_from);
    moving_to.setValue(ex->moving_to);
    moving_mode_to.setValue(ex->moving_mode_to);
}

extra_exit_data *
XMLExtraExit::compat( )
{
    EXTRA_EXIT_DATA *peexit = (EXTRA_EXIT_DATA*)alloc_perm(sizeof(EXTRA_EXIT_DATA));
    
    peexit->description = str_dup(description.getValue( ).c_str( ));
    peexit->exit_info_default = peexit->exit_info = flags.getValue( );
    peexit->key = key.getValue( );
    peexit->u1.vnum = target.getValue( );
    peexit->level = 0;

    peexit->short_desc_from = str_dup( short_desc_from.getValue( ).c_str( ));
    peexit->short_desc_to = str_dup( short_desc_to.getValue( ).c_str( ));
    peexit->room_description = str_dup( room_description.getValue( ).c_str( ));
    peexit->max_size_pass = max_size_pass.getValue( );

    peexit->moving_from = moving_from.getValue( );
    peexit->moving_mode_from = moving_mode_from.getValue( );
    peexit->moving_to = moving_to.getValue( );
    peexit->moving_mode_to = moving_mode_to.getValue( );
    
    return peexit;
}

