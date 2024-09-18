
/* $Id: objectbehavior.h,v 1.1.2.6.6.3 2007/09/11 00:00:29 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef OBJECTBEHAVIOR_H
#define OBJECTBEHAVIOR_H

#include <sstream>

#include "xmlvariablecontainer.h"
#include "xmlpersistent.h"

class Object;
class Character;

class ObjectBehavior : public XMLVariableContainer {
XML_OBJECT
public:    
    typedef ::Pointer<ObjectBehavior> Pointer;
    
    ObjectBehavior( );
    virtual ~ObjectBehavior( );

    virtual void setObj( Object * );
    virtual void unsetObj( );
    Object * getObj( );

    static const DLString NODE_NAME;

    virtual void wear( Character *victim );
    virtual void equip( Character *victim );
    virtual void remove( Character *victim );
    virtual void get( Character *victim ); 
    virtual bool fetch( Character *victim, Object *item ); 
    virtual bool drop( Character *victim );
    virtual bool sac( Character *victim );
    virtual void entry(        );
    virtual void give( Character *from, Character *to );
    virtual void greet( Character *victim );
    virtual void fight( Character *victim );
    virtual bool death( Character *victim );
    virtual void speech( Character *victim, const char *speech );
    virtual void show( Character *victim, ostringstream &buf );
    virtual bool area( );
    virtual bool hourly();
    virtual bool extract( bool );
    virtual bool save( ); 
    virtual void delete_( Character * ); 
    virtual bool quit( Character *, bool ); 
    virtual bool examine( Character *victim );
    virtual bool use( Character *user, const char * );
    virtual bool command( Character *, const DLString &, const DLString & );
    virtual DLString extraDescription( Character *ch, const DLString & );
    virtual bool visible( const Character * );

    virtual bool isLevelAdaptive( ); 
    virtual bool mayFloat( ); 
    virtual bool canSteal( Character * ); 
    virtual bool canLock( Character * ); 
    virtual bool canConfiscate( ); 
    virtual bool canEquip( Character * );
    virtual bool canDress( Character *, Character * );

    virtual bool hasTrigger( const DLString & );

protected:
    Object *obj;

};

extern template class XMLStub<ObjectBehavior>;

#endif

