/* $Id: pointer.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          pointer.h  -  description
                             -------------------
    begin                : Thu May 31 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef POINTER_H
#define POINTER_H

/**
 * @author Igor S. Petrenko
 * @short Используется для Pointer для вызова new
 */
static const struct NEWStruct {} NEW = {};

/**
 * @short Работа с указателями ( работает с link/unlink класса DLObject )
 * @author Igor S. Petrenko
 */
template<typename T>
class Pointer
{
public:
	typedef Pointer<T> SelfType;

public:
	/** Пустой указатель */
	inline Pointer( ) : pointer( 0 )
	{
	}
	
	/** Как параметр Pointer этого типа */
	inline Pointer( const SelfType& cpointer )
	{
		setPointerWithoutSelf( cpointer.pointer );
	}
	
	/** Как параметр, Pointer другого типа */
	template<typename NoT>
	inline Pointer( NoT cpointer )
	{
		setPointerWithoutSelf( cpointer.getPointer( ) );
	}
	
	/** Копия класса в this ( link ) */
	inline Pointer( T* pointer )
	{
		setPointerWithoutSelf( pointer );
	}

	/** Копия класса в this ( link ) */
	inline Pointer( const T* pointer )
	{
		setPointerWithoutSelf( const_cast<T*>( pointer ) );
	}
	
	/** Конструктор по умолчанию, без аргументов */
	inline Pointer( const NEWStruct& )
	{
		setPointerWithoutSelf( new T( ) );
	}

	/** Конструктор по умолчанию, 1-н аргумент */
	template<typename Arg1>
	inline Pointer( const NEWStruct&, Arg1 arg1 )
	{
		setPointerWithoutSelf( new T( arg1 ) );
	}
	
	/** Конструктор по умолчанию, 2-ва аргумента */
	template<typename Arg1, typename Arg2>
	inline Pointer( const NEWStruct&, Arg1 arg1, Arg2 arg2 )
	{
		setPointerWithoutSelf( new T( arg1, arg2 ) );
	}
	
	/** Конструктор по умолчанию, 3-и аргумента */
	template<typename Arg1, typename Arg2, typename Arg3>
	inline Pointer( const NEWStruct&, Arg1 arg1, Arg2 arg2, Arg3 arg3 )
	{
		setPointerWithoutSelf( new T( arg1, arg2, arg3 ) );
	}

	/** Конструктор по умолчанию, 4-е аргумента */
	template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	inline Pointer( const NEWStruct&, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4 )
	{
		setPointerWithoutSelf( new T( arg1, arg2, arg3, arg4 ) );
	}
	
	/** если указатель сущ., то unlink */
	inline ~Pointer( )
	{
		if( pointer != 0 ) pointer->unlink( );
	}

	/** @return Получить обьект указателя */
	inline T* getPointer( )
	{
		return pointer;
	}
	
	/** @return Получить const обьект указателя */
	inline const T* getPointer( ) const
	{
		return pointer;
	}

	/** @return Получить обьект указателя и привести через static_cast к типу P*/
	template<typename T1>
	inline T1* getStaticPointer( )
	{
		return static_cast<T1*>( pointer );
	}

	template<typename T1>
	inline T1* getConstPointer( ) const
	{
		return const_cast<T1*>( static_cast<T1*>( pointer ) );
	}
	
	/** @return Получить обьект указателя и привести через dynamic_cast к типу T1*/
	template<typename T1>
	inline T1* getDynamicPointer( )
	{
		return dynamic_cast<T1*>( pointer );
	}
	
	/** Установить указатель */
	inline void setPointer( T* pointer )
	{
		if( this->pointer != 0 ) this->pointer->unlink( );
		setPointerWithoutSelf( pointer );
	}
	
	/** Присвоить класс */
	inline SelfType& operator = ( T* pointer )
	{
		setPointer( pointer );
		return *this;
	}
	
	inline SelfType& operator = ( const SelfType& cpointer )
	{
		setPointer( cpointer.pointer );
		return *this;
	}
	
	/**
	 * Присвоить указатель. Используется если NT наследует T
	 */
	template<typename NoT>
	inline SelfType& operator = ( NoT& cpointer )
	{
		setPointer( cpointer.getPointer( ) );
		return *this;
	}
	
	/** Доступ к классу через '*' */
	inline T* operator * ( ) 
	{
		return pointer;
	}
	
	/** Доступ к классу, как const,  через '*' */
	inline const T* operator * ( ) const
	{
		return pointer;
	}
	
	/** Доступ к классу через '->' */
	inline T* operator -> ( )
	{
		return pointer;
	}

	/** Доступ к классу, как const,  через '->' */
	inline const T* operator -> ( ) const
	{
		return pointer;
	}

	/** @return true - если указатели равны */
	inline bool operator == ( const SelfType& cpointer ) const
	{
		return pointer == cpointer.pointer;
	}
	
	/** @return true - если указатели равны */
	template<typename NoT>
	inline bool operator == ( const NoT& cpointer ) const
	{
		return pointer == cpointer.getPointer( );
	}

	/** @return true - если указатели не равны */
	inline bool operator != ( const SelfType& cpointer ) const
	{
		return pointer != cpointer.pointer;
	}
	
	/** @return true - если указатели не равны */
	template<typename NoT>
	inline bool operator != ( const NoT& cpointer ) const
	{
		return pointer != cpointer.getPointer( );
	}
	
	inline operator bool( ) const
	{
		return !isEmpty( );
	}
	
	/** @return true - если указатель пустой */
	inline bool isEmpty( ) const
	{
		return pointer == 0;
	}

	/** Обнуляет указатель ( unlink ) */
	inline void clear( )
	{
	    if (pointer != 0) {
		pointer->unlink( );
		pointer = 0;
	    }
	}
	
	inline void strip( )
	{
	    if (pointer != 0) {
		pointer->setLinkCount( pointer->getLinkCount( ) - 1 );
		pointer = 0;
	    }
	}

	/** Создать обьект */
	inline void construct( )
	{
		setPointer( new T( ) );
	}

private:
	/** Установить указатель, если сам класс isEmpty */
	inline void setPointerWithoutSelf( T* pointer )
	{
		if( pointer != 0 ) pointer->link( );
		this->pointer = pointer;
	}
		
private:
	T* pointer;
};



#endif
