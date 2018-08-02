/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBITEMMANIP_H
#define WEBITEMMANIP_H

#include "webmanip.h"

class WebItemManip : public WebManip {
public:    
	typedef ::Pointer<WebItemManip> Pointer;

        virtual bool decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *, const DLString &pocket, int combined ) const;
        virtual bool decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character * ) const;
        virtual bool decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const;
};

#endif
