/* $Id$
 *
 * ruffina, 2004
 */
#include "affectwrapper.h"

#include "skill.h"
#include "skillgroup.h"
#include "skillmanager.h"
#include "spelltarget.h"
#include "liquid.h"
#include "wearlocation.h"
#include "nativeext.h"
#include "regcontainer.h"
#include "reglist.h"
#include "wrappermanager.h"
#include "wrap_utils.h"
#include "subr.h"
#include "fight.h"
#include "handler.h"
#include "damageflags.h"
#include "merc.h"
#include "schedulerwrapper.h"
#include "def.h"

const FlagTable * affect_where_to_table(int where);

using namespace std;
NMI_INIT(AffectWrapper, "аффект");

AffectWrapper::AffectWrapper( const RegisterList &args )
{
    RegisterList::const_iterator i;

    i = args.begin( );
    
    if (i != args.end( )) {
        Skill *skill = skillManager->findExisting(i->toString());
        if (!skill)
            throw Scripting::Exception("Affect type not found.");
        target.type.assign(*skill);
    } else
        return;

    if (++i != args.end( ))
        target.level = i->toNumber( );
    else
        return;

    if (++i != args.end( ))
        target.duration = i->toNumber( );
    else
        return;

    if (++i != args.end())
        throw Scripting::TooManyArgumentsException();
}

AffectWrapper::AffectWrapper(Affect &af)
{
    af.copyTo(target);
}

NMI_GET( AffectWrapper, type, "название умения, которым этот аффект вешается, или none" ) 
{ 
    int sn = target.type;

    if (sn < 0)
        return Register( "none" );
    else
        return Register( target.type->getName( ) ); 
} 

NMI_SET( AffectWrapper, type, "название умения, которым этот аффект вешается, или none" ) 
{ 
    const DLString & name = arg.toString( );

    if (name == "none") 
        target.type.setName( name.c_str( ) );
    else {
        Skill * skill = skillManager->findExisting( name );

        if (!skill)
            throw Scripting::IllegalArgumentException( );
        
        target.type.assign( *skill );
    }
}

NMI_INVOKE(AffectWrapper, apply, "(ch): применить действие аффекта на ch, не вешая его")
{
    Character *ch = args2character(args);

    affect_modify(ch, &target, true);
    return Register();	
}

NMI_SET(AffectWrapper, source, "(ch|obj|room): запомнить ch, obj или room как источник этого аффекта")
{
    SpellTarget::Pointer src = arg2target(arg);

    if (src->type == SpellTarget::ROOM)
        target.sources.add(src->room);
    else if (src->type == SpellTarget::OBJECT)
        target.sources.add(src->obj);
    else if (src->type == SpellTarget::CHAR)
        target.sources.add(src->victim);
    else 
        throw Scripting::IllegalArgumentException();
}

NMI_GET(AffectWrapper, source, "вернуть наиболее актуальный источник этого аффекта: player или mob")
{
    Character *ch = target.sources.getOwner();
    if (ch == NULL)
        return Register();
    else
        return ::wrap(ch);
}

NMI_INVOKE(AffectWrapper, damage, "(vict,dam,skillName[,damtype,damflags]): нанести vict повреждения в размере dam умением skillName и типом damtype (таблица .tables.damage_table)" )
{
    Character *victim = argnum2character(args, 1);
    int dam = argnum2number(args, 2);
    Skill *skill = argnum2skill(args, 3);
    int dam_type = args.size() > 3 ? argnum2flag(args, 4, damage_table) : DAM_NONE;
    bitstring_t damflags = args.size() > 4 ? argnum2flag(args, 5, damage_flags) : 0L;

    return damage(&target, victim, dam, skill->getIndex( ), dam_type, true, damflags);
}


#define GS(x, api) \
NMI_GET( AffectWrapper, x, api ) \
{ \
    return Register( (int)target.x.getValue( ) ); \
} \
NMI_SET( AffectWrapper, x, api ) \
{ \
    target.x.setValue( arg.toNumber( ) ); \
}

GS(location, "поле, на которое аффект воздействует численно (таблица .tables.apply_flags)")
GS(modifier, "на сколько изменится поле, указанное в location")
GS(duration, "длительность, -1 для вечных аффектов")
GS(level, "уровень аффекта")

NMI_INVOKE(AffectWrapper, bitvector, "(table,flags): какие флаги и по какой таблице устанавливает аффект; table - таблица или название, flags - биты или строка с их названиями")
{
    const FlagTable *table = argnum2table(args, 1);
    int flag = argnum2flag(args, 2, *table);

    target.bitvector.setTable(table);
    target.bitvector.setValue(flag);
    return Register();
}

NMI_GET(AffectWrapper, bitvector, "численное значение флагов, которые установит аффект")
{
    return Register((int)target.bitvector.getValue());
}

NMI_SET(AffectWrapper, where, "на какую таблицу применен bitvector или на что воздействует global (.tables.affwhere_flags)")
{
    int where = arg.toNumber();

    switch (where) {
        case TO_LOCATIONS:
            target.global.setRegistry( wearlocationManager );
            break;
        case TO_LIQUIDS:
            target.global.setRegistry( liquidManager );
            break;
        case TO_SKILL_GROUPS:
            target.global.setRegistry( skillGroupManager );
            break;
        case TO_SKILLS:
            target.global.setRegistry( skillManager );
            break;
        default:
            throw Scripting::Exception("Accepted values for 'where' field are: wearlocations, liquids, skills, skill_groups");
    }
}

NMI_SET( AffectWrapper, global, "список значений для where=locations (слоты экипировки), liquids (жидкости), skills, skill groups" ) 
{
    const GlobalRegistryBase *registry = target.global.getRegistry();
    if (!registry)
        throw Scripting::Exception("Affect 'global' is assigned before 'where' is set.");

    target.global.fromString(arg.toString());
}

NMI_GET( AffectWrapper, global, "список значений для where=locations (слоты экипировки), liquids (жидкости), skills, skill groups" ) 
{
    return target.global.toString( );
}

Scripting::Register AffectWrapper::wrap( const Affect &af )
{
    AffectWrapper::Pointer aw( NEW, af );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler( aw );

    return Scripting::Register( sobj );
}

NMI_INVOKE( AffectWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<AffectWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

