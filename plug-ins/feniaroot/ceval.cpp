/* $Id: ceval.cpp,v 1.1.4.4.6.6 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include "profiler.h"
#include "phase.h"
#include "function.h"
#include "admincommand.h"
#include "rpccommandmanager.h"
#include "pcharacter.h"
#include "logstream.h"
#include "dreamland.h"
#include "commonattributes.h"
#include "comm.h"
#include "merc.h"

#include "characterwrapper.h"
#include "wrappersplugin.h"
#include "wrappermanager.h"
#include "def.h"

using Scripting::CodeSource;


bool has_fenia_security( PCharacter *pch )
{
    if (pch->get_trust( ) >= 110)
	return true;
    
    if (dreamland->hasOption( DL_BUILDPLOT )) {
	XMLIntegerAttribute::Pointer secAttr
		= pch->getAttributes( ).findAttr<XMLIntegerAttribute>( "feniasecurity" );
	
	if (secAttr && secAttr->getValue( ) >= 110)
	    return true;
    }

    try {
	static Scripting::IdRef FENIA_SECURITY_ID("feniaSecurity");
	Register thiz = WrapperManager::getThis( )->getWrapper( pch );
	
	if ((*thiz[FENIA_SECURITY_ID]).toNumber() >= 110)
	    return true;
    } catch (::Exception e) {
    }

    return false;
}

CMDADM( eval )
{
    PCharacter *pch = ch->getPC( );

    if (!pch) 
	return;
    
    if (!has_fenia_security( pch )) {
	ch->println("Ты не ботаешь по фене.");
	return;
    }

    if (constArguments.empty( )) {
	ch->println("Синтаксис: {Weval {x<expression>");
	return;
    }

    Register thiz = WrapperManager::getThis( )->getWrapper( ch );
    
    try {
	CodeSource &cs = CodeSource::manager->allocate();
	
	cs.author = pch->getName( );
	cs.name = "<eval command>";

	cs.content = constArguments;
	cs.eval(thiz);
    
    } catch (const ::Exception &e) {
	ch->send_to( e.what( ) );
    }
}

RPCRUN(cs_eval)
{
    if(args.size() < 2) {
        LogStream::sendError() << "cs_eval: not enough arguments: " << args.size() << endl;
        return;
    }

    DLString subject = args[0];
    DLString body = args[1];

    PCharacter *pch = ch->getPC();

    if(pch) {
        LogStream::sendNotice() 
                << "cs_eval: " << subject << endl
                << "----------------------------" << endl
                << body
                << "----------------------------" << endl;

        if (!has_fenia_security( pch )) {
            ch->println("Ты не ботаешь по фене.");
            return;
        }

        Register thiz = WrapperManager::getThis( )->getWrapper( ch );
        
        try {
            CodeSource &cs = CodeSource::manager->allocate();
            
            cs.author = pch->getName( );
            cs.name = subject;

            cs.content = body;
            cs.eval(thiz);
        
        } catch (const ::Exception &e) {
            ch->send_to( e.what( ) );
        }
    } else {
        LogStream::sendError() << "cs_eval: pch is required" << endl;
    }
}
