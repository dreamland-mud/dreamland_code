/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBMANIP_H
#define WEBMANIP_H

#include <list>
#include <sstream>
#include "oneallocate.h"
#include "dlstring.h"
#include "plugin.h"

class Object;
class Character;

class WebManip : public virtual Plugin {
public:    
	typedef ::Pointer<WebManip> Pointer;

        virtual ~WebManip( );

        virtual void initialization( );
        virtual void destruction( );
        virtual bool decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *, const DLString &pocket, int combined ) const;
        virtual bool decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character * ) const;
        virtual bool decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const;
 
};



class WebManipManager : public OneAllocate, public virtual Plugin {
public:
	typedef ::Pointer<WebManipManager> Pointer;
        typedef std::list<WebManip::Pointer> WebManipList;

	WebManipManager( );
	virtual ~WebManipManager( );

        void registrate( WebManip::Pointer );
        void unregistrate( WebManip::Pointer );

        void decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *, const DLString &pocket, int combined ) const;
        void decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character * ) const;
        void decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const;

protected:        
        virtual void initialization( );
        virtual void destruction( );

        WebManipList manips;
};    

extern WebManipManager *webManipManager;
#endif
