/* $Id$
 *
 * ruffina, 2004
 */
#include "affectwrapper.h"

#include "affect.h"
#include "skill.h"
#include "skillmanager.h"
#include "liquid.h"
#include "wearlocation.h"

#include "nativeext.h"
#include "regcontainer.h"
#include "reglist.h"
#include "wrappermanager.h"
#include "subr.h"
#include "handler.h"
#include "merc.h"

#include "schedulerwrapper.h"
#include "def.h"

using namespace std;
NMI_INIT(AffectWrapper, "аффект");

AffectWrapper::AffectWrapper( const RegisterList &args )
{
    RegisterList::const_iterator i;

    i = args.begin( );
    
    if (i != args.end( ))
	type.assign( *skillManager->findExisting( i->toString( ).c_str( ) ) );
    else
	return;
    if (++i != args.end( ))
	level = i->toNumber( );
    else
	return;
    if (++i != args.end( ))
	duration = i->toNumber( );
    else
	return;
    if (++i != args.end( ))
	location = i->toNumber( );
    else
	return;
    if (++i != args.end( ))
	modifier = i->toNumber( );
    else
	return;
    if (++i != args.end( ))
	where = i->toNumber( );
    else 
	return;
    if (++i != args.end( ))
	bitvector = i->toNumber( );
}

void AffectWrapper::toAffect( Affect & af ) 
{
    af.type = type;
    af.where = where;
    af.location = location;
    af.duration = duration;
    af.modifier = modifier;
    af.bitvector = bitvector;
    af.level = level;
    
    if (!global.empty( )) {
	af.global.setRegistry( global.getRegistry( ) );
	af.global.set( global );
    }
}

void AffectWrapper::fromAffect( const Affect & af ) 
{
    type = af.type;
    where = af.where;
    location = af.location;
    duration = af.duration;
    modifier = af.modifier;
    bitvector = af.bitvector;
    level = af.level;

    if (!af.global.empty( )) {
	global.setRegistry( af.global.getRegistry( ) );
	global.set( af.global );
    }
}

NMI_GET( AffectWrapper, type, "название скила, которым этот аффект вешается, или none" ) 
{ 
    int sn = type;

    if (sn < 0)
	return Register( "none" );
    else
	return Register( type->getName( ) ); 
} 

NMI_SET( AffectWrapper, type, "название скила, которым этот аффект вешается, или none" ) 
{ 
    const DLString & name = arg.toString( );

    if (name == "none") 
	type.setName( name.c_str( ) );
    else {
	Skill * skill = skillManager->findExisting( name );

	if (!skill)
	    throw Scripting::IllegalArgumentException( );
	
	type.assign( *skill );
    }
}


#define GS(x, api) \
NMI_GET( AffectWrapper, x, api ) \
{ \
    return Register( x.getValue( ) ); \
} \
NMI_SET( AffectWrapper, x, api ) \
{ \
    x.setValue( arg.toNumber( ) ); \
}

GS(where, "поле, у которого аффект изменяет биты (таблица .tables.affwhere_flags.)")
GS(bitvector, "какие биты добавятся полю, указанному в where")
GS(location, "поле, на которое аффект воздействует численно (таблица .tables.apply_flags)")
GS(modifier, "на сколько изменится поле, указанное в location")
GS(duration, "длительность, -1 для вечных аффектов")
GS(level, "уровень аффекта")

NMI_SET( AffectWrapper, global, "" ) 
{
    if (where == TO_LOCATIONS) {
	global.setRegistry( wearlocationManager );
	global.fromString( arg.toString( ) );
    } else if (where == TO_LIQUIDS) {
	global.setRegistry( liquidManager );
	global.fromString( arg.toString( ) );
    }
}

NMI_GET( AffectWrapper, global, "" ) 
{
    return global.toString( );
}

Scripting::Register AffectWrapper::wrap( const Affect &af )
{
    AffectWrapper::Pointer aw( NEW );

    aw->fromAffect( af );
    
    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler( aw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( AffectWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<AffectWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

