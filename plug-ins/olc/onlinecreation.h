/* $Id$
 *
 * ruffina, 2004
 */

#ifndef _ONLINECREATION_H_
#define _ONLINECREATION_H_

#include "commandplugin.h"
#include "defaultcommand.h"
#include "pointer.h"
#include "so.h"

class PCharacter;

typedef void do_fn_t (PCharacter *, char *);


struct cmd_info;


class OnlineCreation : public CommandPlugin, public DefaultCommand
{
public:
	typedef ::Pointer<OnlineCreation> Pointer;

public:
	OnlineCreation(struct cmd_info *);
	virtual ~OnlineCreation( );
	
	inline virtual const DLString& getName( ) const
	{
		return ocName;
	}
	
	static inline void registerPlugin( SO::PluginList& ppl )
	{
	    OnlineCreation *oc;
	    for(oc = ocList; oc; oc = oc->next)
		ppl.push_back( static_cast<Plugin*>( oc ) );
	}
	
	virtual void run( Character* ch, const DLString& args );
	virtual bool available( Character * ) const;
        virtual CommandLoader * getLoader( ) const;

private:
	DLString ocName;
	OnlineCreation *next;
	do_fn_t *go;
	static OnlineCreation *ocList;
};

struct cmd_info {
    const char *name; 
    do_fn_t *go; 
    const char *shortName;
    int position; 
    int level;
    int log;
    int extra;
};

#define CMD(x, prio, rus, pos, lev, log, desc)		\
do_fn_t __do_ ## x;					\
static struct cmd_info __cmdinfo_ ## x = {			\
#x, &__do_ ## x, rus, pos, lev, log,  0	\
};								\
OnlineCreation::Pointer __cmd_ ## x (NEW, &__cmdinfo_ ## x);	\
void __do_ ## x (PCharacter *ch, char *argument)

#endif /* _ONLINECREATION_H_ */
