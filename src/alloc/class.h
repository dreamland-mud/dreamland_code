/* $Id: class.h,v 1.1.2.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          class.h  -  description
                             -------------------
    begin                : Mon Oct 1 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef CLASS_H
#define CLASS_H

#include "dlobject.h"
#include "exceptionclassnotfound.h"
#include "allocateclass.h"

class DLString;

class Class {
public:
    class ClassRegistrator : public virtual DLObject {
    public:
	typedef ::Pointer<ClassRegistrator> Pointer;

	virtual ~ClassRegistrator( );

	virtual AllocateClass::Pointer clone( ) const = 0;
	virtual AllocateClass * cloneRaw( ) const {
	    return NULL;
	}
    };

    template<typename C>
    class RegisterManyAllocateClass : public ClassRegistrator {
    public:
	typedef ::Pointer< RegisterManyAllocateClass<C> > Pointer;

	inline virtual AllocateClass::Pointer clone( ) const {
	    return ::Pointer<C> ( NEW );
	}
    };

    template<typename C>
    class RegisterOneAllocateClass : public ClassRegistrator {
    public:
	typedef ::Pointer< RegisterOneAllocateClass<C> > Pointer;

	inline virtual AllocateClass::Pointer clone( ) const {
	    return ::Pointer<C> ( C::getThis( ) );
	}
    };
	
public:
    template <typename T> static void regMocOne( ) {
	regClass( T::MOC_TYPE, 
		typename RegisterOneAllocateClass<T>::Pointer( NEW ) );
    }

    template <typename T> static void regMoc( ) {
	regClass( T::MOC_TYPE, 
		typename RegisterManyAllocateClass<T>::Pointer( NEW ) );
    }
    
    template <typename T> static void unregMoc( ) {
	unRegClass( T::MOC_TYPE );
    }
    
    template <typename T> static void regXMLVar( ) {
	regClass( T::TYPE, 
		typename RegisterManyAllocateClass<T>::Pointer( NEW ) );
    }
    
    template <typename T> static void unregXMLVar( ) {
	unRegClass( T::TYPE );
    }
    
    static void regClass( const DLString& name, ClassRegistrator::Pointer classPointer );
    static void unRegClass( const DLString& name );
    static AllocateClass::Pointer allocateClass( const DLString& name ) throw( ExceptionClassNotFound );
    static AllocateClass * allocateClassRaw( const DLString& name ) throw( ExceptionClassNotFound );
};


#endif
