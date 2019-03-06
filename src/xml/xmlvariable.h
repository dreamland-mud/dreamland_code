/* $Id: xmlvariable.h,v 1.9.2.1.28.2 2009/10/11 18:35:39 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// dlvariable.h: interface for the DLVariable class.
//
//////////////////////////////////////////////////////////////////////

#ifndef XMLVARIABLE_H
#define XMLVARIABLE_H

#include "exceptionbadtype.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short Предок всех XML переменных
 * переменные в XML идут в виде <str>10</str>, где str - имя переменной
 * для считывания таких переменных и создан класс XMLVariable. На вход
 * виртуальный метод fromXML получает строку приведенную выше и если
 * преобразование string -> тип переменной успешно, переменная получает
 * значение сохраненное в XML. Метод toXML производит обратную интерпритацию.
 */
class XMLVariable : public virtual DLObject
{
public:
    typedef ::Pointer<XMLVariable> Pointer;
    
    virtual ~XMLVariable( );

public:
    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType ) = 0;
    /** @param - пустой xmlnode передается по ссылке */
    virtual bool toXML( XMLNode::Pointer& node ) const = 0;
};


#endif
