/* $Id: xmlref.cpp,v 1.1.4.2.6.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "logstream.h"
#include "xmlref.h"

/*----------------------------------------------------------------------
 * XMLRefBase
 *----------------------------------------------------------------------*/
XMLRefBase::refid_t  XMLRefBase::lastRefId = 0;
const DLString XMLRefBase::ATTRIBUTE_REFID = "refid";
XMLRefBase::refmap_t XMLRefBase::refmap;

XMLRefBase::refid_t
XMLRefBase::nextReferenceId( )
{
    while(refmap.find(++lastRefId) != refmap.end())
	;

    return lastRefId;
}

/*----------------------------------------------------------------------
 * XMLRefVariable
 *----------------------------------------------------------------------*/
XMLRefVariable::XMLRefVariable() : refid(0)
{
    setReferenceId(nextReferenceId( ));
}

XMLRefVariable::~XMLRefVariable()
{
    setReferenceId(0);
}

XMLRefBase::refid_t
XMLRefVariable::getReferenceId() const
{
    return refid;
}

void
XMLRefVariable::setReferenceId(refid_t id)
{
    if(id == refid)
	return;

    if(id)
	refmap[id] = this;

    if(refid) {
	refmap_t::iterator i = refmap.find(refid);

	if(i == refmap.end( ))
	    LogStream::sendError()
		<< "refmap: key " << refid 
		<< " not found for erase. duplicate erase?" << endl;
	else
	    refmap.erase(i);
    }

    refid = id;
}

