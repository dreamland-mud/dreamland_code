/* $Id: xmlattributes.cpp,v 1.5.2.11.6.3 2008/02/24 17:11:47 rufina Exp $
 *
 * ruffina, 2003
 * based on XMLAttributes by NoFate, 2001
 */

#include "class.h"
#include "logstream.h"
#include "xmlattributes.h"
#include "fenia/exceptions.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "lex.h"

using namespace Scripting;

void XMLAttributes::eraseAttribute( const DLString &name )
{
    iterator ipos = find( name );
    
    if (ipos != end( )) {
        XPAttribute attr = ipos->second;
        erase( ipos );
        attr->destroy( );
    }
}
   
void XMLAttributes::addAttribute( XMLAttribute::Pointer attr, const DLString &attrString )
{
    XPAttribute pAttr;
    pAttr.setPointer( attr.getPointer( ) );
    insert( std::make_pair( attrString, pAttr ) );
}

bool XMLAttributes::isAvailable( const DLString& name ) const
{
    return find( name ) != end( );
}

bool XMLAttributes::nodeFromXML( const XMLNode::Pointer & child )
{
    try {
        if (!XMLMapBase<XPAttribute>::nodeFromXML( child ))
            if (!XMLVariableContainer::nodeFromXML( child ))
                LogStream::sendWarning( ) 
                    << "Unparsed node <" <<  child->getName( ) << ">" << endl;
    } 
    catch (const ExceptionBadType& e) {
        LogStream::sendWarning( ) << e.what( ) << endl;
    }
    
    return true;
}

Register XMLAttributes::toRegister() const
{
    Register attrsReg = Register::handler<RegContainer>();
    RegContainer *attrsContainer = attrsReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto &a: *this) {
        const DLString &attrName = a.first;
        Register attrReg = a.second->toRegister();

        if (attrReg.type == Register::NONE) 
            // Still need to put something for attrs that don't handle toRegister() call yet.
            // Putting a null reg will just erase this entry from RegContainer.
            attrsContainer->setField(attrName, DLString::emptyString);
        else
            attrsContainer->setField(attrName, attrReg);
    }

    return attrsReg;
}