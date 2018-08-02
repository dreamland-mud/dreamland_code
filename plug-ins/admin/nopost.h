/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          nopost.h  -  description
                             -------------------
    begin                : Thu Oct 18 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef NOPOST_H
#define NOPOST_H

#include "xmlattributeticker.h"
#include "xmlattributeplugin.h"
#include "playerattributes.h"


/**
 * @author Igor S. Petrenko
 */
class XMLAttributeNoPost : public XMLAttributeOnlineTicker, public RemortAttribute {
XML_OBJECT;
public:
	typedef ::Pointer<XMLAttributeNoPost> Pointer;

	XMLAttributeNoPost( );

	virtual void start( PCMemoryInterface * ) const;
	virtual void end( PCMemoryInterface * ) const;
};


#endif
