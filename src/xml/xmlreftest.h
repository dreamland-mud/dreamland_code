/* $Id: xmlreftest.h,v 1.1.4.1.6.1 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef __XMLREFTEST_H__
#define __XMLREFTEST_H__

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlpointer.h"

class XMLRefTestTarget : public XMLVariableContainer, public XMLRefVariable
{
XML_OBJECT
public:
    typedef ::Pointer<XMLRefTestTarget> Pointer;

    XML_VARIABLE XMLInteger var;
};


class XMLRefTestMaster : public XMLVariableContainer
{
XML_OBJECT
public:
    typedef ::Pointer<XMLRefTestMaster> Pointer;

    XML_VARIABLE XMLPointer<XMLRefTestTarget> ref;
};

class XMLRefTestSlave : public XMLVariableContainer
{
XML_OBJECT
public:
    typedef ::Pointer<XMLRefTestSlave> Pointer;

    XML_VARIABLE XMLReference<XMLRefTestTarget> ref;
};

#endif

