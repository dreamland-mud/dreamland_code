/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBMANIP_H
#define WEBMANIP_H

#include <map>
#include <sstream>
#include "oneallocate.h"
#include "dlstring.h"
#include "plugin.h"

class Object;
class Character;
class PCharacter;
struct ManipCommandArgs;
struct extra_descr_data;

class WebManipCommand : public virtual Plugin {
public:    
	typedef ::Pointer<WebManipCommand> Pointer;

        virtual ~WebManipCommand( );

        virtual void initialization( );
        virtual void destruction( );
        virtual const DLString &getName( ) const = 0;
        virtual bool run( ostringstream &buf, const ManipCommandArgs &args ) = 0;
};


class WebManipManager : public OneAllocate, public virtual Plugin {
public:
	typedef ::Pointer<WebManipManager> Pointer;
    typedef std::map<DLString, WebManipCommand::Pointer> WebManipMap;

	WebManipManager( );
	virtual ~WebManipManager( );

        void registrate( WebManipCommand::Pointer );
        void unregistrate( WebManipCommand::Pointer );

        void decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *, const DLString &pocket, int combined ) const;
        void decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character * ) const;
        void decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const;
        void decorateExtraDescr( ostringstream &buf, const char *desc, extra_descr_data *ed, Character *ch ) const;
        void decorateCharacter( ostringstream &buf, const DLString &descr, Character *victim, Character *ch ) const;

protected:        
        virtual void initialization( );
        virtual void destruction( );
        bool run( ostringstream &buf, const DLString &command, const ManipCommandArgs &args ) const;

        WebManipMap manips;
};    

extern WebManipManager *webManipManager;

struct ManipCommandArgs {
	ManipCommandArgs(Character *target) {
		this->target = target;
	}
	Character *target;
};

struct ItemManipArgs : public ManipCommandArgs {
	ItemManipArgs(Character *target, Object *item, const DLString &descr, const DLString &pocket, int combined)
		: ManipCommandArgs(target)
	{
		this->item = item;
		this->descr = descr;
		this->pocket = pocket;
		this->combined = combined;
	}
	Object *item;
	DLString descr;
	DLString pocket;
	int combined;
};

struct ShopItemManipArgs : public ManipCommandArgs {
	ShopItemManipArgs(Character *target, Object *item, const DLString &descr)
		: ManipCommandArgs(target)
	{
		this->item = item;
		this->descr = descr;
	}

	Object *item;
	DLString descr;
};

struct PocketManipArgs : public ManipCommandArgs {
	PocketManipArgs(Character *target, const DLString &pocket, Object *container)
		: ManipCommandArgs(target)
	{
		this->pocket = pocket;
		this->container = container;
	}

	DLString pocket;
	Object *container;
};

struct ExtraDescrManipArgs : public ManipCommandArgs {
    ExtraDescrManipArgs(Character *target, const char *desc, extra_descr_data *ed)
            : ManipCommandArgs(target)
    {
        this->desc = desc;
        this->ed = ed;
    }

    const char *desc;
    extra_descr_data *ed;
};

struct PlayerManipArgs : public ManipCommandArgs {
	PlayerManipArgs(Character *target, PCharacter *victim, const DLString &descr)
		: ManipCommandArgs(target)
	{
		this->victim = victim;
		this->descr = descr;
	}

    PCharacter *victim;
	DLString descr;
};


#endif
