/* $Id: dbio.h,v 1.13.2.2.28.5 2010-09-01 08:21:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// dbio.h: interface for the DBIO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBIO_H__C4E32C3C_D3DB_4E2E_AF2C_CA9169950F71__INCLUDED_)
#define AFX_DBIO_H__C4E32C3C_D3DB_4E2E_AF2C_CA9169950F71__INCLUDED_

#include "dlstring.h"
#include "dlobject.h"
#include "dldirectory.h"
#include "exceptiondbio.h"
#include "exceptiondbioeof.h"


/**
 * @author Igor S. Petrenko
 * @short Класс для работы с каталогами
 */
class DBIO : public virtual DLObject
{
public:
	/**
	 * @author Igor S. Petrenko
	 */
	class DBNode
	{
	public:
		inline DBNode( )
		{
		}

		inline DBNode( const DLString& key, const DLString& xml )
			: key( key ), xml( xml )
		{
		}

		inline const DLString& getKey( ) const
		{
			return key;
		}

		inline const DLString& getXML( ) const
		{
			return xml;
		}

	private:
		DLString key;
		DLString xml;
	};

public:
	
	static const DLString EXT_XML;
	
	DBIO( const DLString & tableName );
	DBIO( const DLString & tablePath, const DLString & tableName );
	DBIO( const DLDirectory &tableDir, const DLString & tableName );
	virtual ~DBIO( );

	void open( ) throw( ExceptionDBIO );
	void open( const DLString &tableName ) throw( ExceptionDBIO );

	DBNode nextXML( ) throw( ExceptionDBIO, ExceptionDBIOEOF );

	void insert( const DBNode & ) throw( ExceptionDBIO );
	void insert( const DLString&, const DLString& ) throw( ExceptionDBIO );
	void safeInsert( const DBNode & ) throw( ExceptionDBIO );
	void safeInsert( const DLString&, const DLString& ) throw( ExceptionDBIO );
	DBNode select( const DLString& ) throw( ExceptionDBIO );
	void remove( const DLString& ) throw( ExceptionDBIO );
	void renameID( const DLString& oldKey, const DLString& newKey ) throw( ExceptionDBIO );
        DLFile getEntryAsFile( const DLString &key );

private:
	DLDirectory table;
};



#endif // !defined(AFX_DBIO_H__C4E32C3C_D3DB_4E2E_AF2C_CA9169950F71__INCLUDED_)
