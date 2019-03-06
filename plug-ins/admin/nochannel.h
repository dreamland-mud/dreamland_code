/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          nochannel.h  -  description
                             -------------------
    begin                : Thu Sep 27 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/
/***************************************************************************
                          xmlattributenochannel.h  -  description
                             -------------------
    begin                : Fri Oct 5 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef NOCHANNEL_H
#define NOCHANNEL_H

#include "xmlattributeticker.h"
#include "xmlattributeplugin.h"
#include "playerattributes.h"

/**
 * @author Igor S. Petrenko
 * @short nochannels command, for those spammers
 * nochannel <player> <date>
 */

/**
 * @author Igor S. Petrenko
 */
class XMLAttributeNoChannel : public XMLAttributeOnlineTicker, public RemortAttribute
{
XML_OBJECT;
public:
        typedef ::Pointer<XMLAttributeNoChannel> Pointer;

        XMLAttributeNoChannel( );
        virtual ~XMLAttributeNoChannel( );
        
        virtual void start( PCMemoryInterface * ) const;
        virtual void end( PCMemoryInterface * ) const;
};



#endif
