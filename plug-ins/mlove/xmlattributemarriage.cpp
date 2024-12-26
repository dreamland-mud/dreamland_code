/* $Id: xmlattributemarriage.cpp,v 1.1.2.4.22.1 2007/09/11 00:34:17 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "xmlattributemarriage.h"

#include "idcontainer.h"
#include "lex.h"
#include "reglist.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "msgformatter.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

XMLAttributeMarriage::XMLAttributeMarriage( )
{
}

bool XMLAttributeMarriage::handle( const WhoisArguments &args )
{
    DLString buf;

    if (!spouse.empty( )) {
        PCMemoryInterface *spousePCM = PCharacterManager::find( spouse );

        if (spousePCM) {
            if (wife)
                buf << "замужем за ";
            else 
                buf << "женат" << GET_SEX( args.pch, "", "о", "а" ) << " на ";
        } else {
            if (wife)
                buf << "вдова ";
            else
                buf << "вдовец ";
        }

        buf << "{W" << spouse << "{w";
    } 

    if (!history.empty( )) {
        if (!spouse.empty( ))
            buf << ", ";

        buf << "был" << GET_SEX( args.pch, " женат(замужем)", "о", "а замужем(жената)" ) 
            << " {W" << history.size( ) 
            << "{w раз" << GET_COUNT( history.size( ), "", "а", "" );
    }

    if (!buf.empty( )) {
        args.lines.push_back( buf );
        return true;
    }

    return false;
}

Register XMLAttributeMarriage::toRegister() const
{
    Register attrReg = Register::handler<IdContainer>();
    IdContainer *attrContainer = attrReg.toHandler().getDynamicPointer<IdContainer>();

    attrContainer->setField(IdRef("spouse"), spouse);
    attrContainer->setField(IdRef("wife"), wife.getValue());

    Register historyReg = Register::handler<RegList>();
    RegList *historyList = historyReg.toHandler().getDynamicPointer<RegList>();

    for (auto &h: history)
        historyList->push_back(Register(h));

    attrContainer->setField(IdRef("history"), historyReg);

    return attrReg;
}



