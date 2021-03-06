/*------------------------------------------------------------------
// sqlite.cpp
//
// Copyright (c) 1997
// Robert Umbehant
// winglib@wheresjames.com
// http://www.wheresjames.com
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted for commercial and
// non-commercial purposes, provided that the following
// conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * The names of the developers or contributors may not be used to
//   endorse or promote products derived from this software without
//   specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
//   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------*/

#include "oexlib.h"

#if defined( OEX_ENABLE_SQLITE )

// Pull in sqlite
#include "../oss/sqlite/sqlite3.h"

// Assumptions
//oexSTATIC_ASSERT( sizeof( sqlite3* ) == sizeof( oexPVOID ) );

OEX_USING_NAMESPACE

CSQLite::CSQLite()
{_STT();
	m_sqobj = oexNULL;
	m_nRows = 0;
	m_bDebugMode = oexFALSE;
}

CSQLite::~CSQLite()
{_STT();
	Destroy();
}

int CSQLite::_Callback( void *pThis, int argc, char **argv, char **azColName )
{_STT();
	if ( !oexCHECK_PTR( pThis ) )
		return 0;

	if ( !oexCHECK_PTR( argv ) || !oexCHECK_PTR( azColName ) )
		return 0;

	// Do the data callback
	int nRet = ((CSQLite*)pThis )->OnCallback( argc, argv, azColName );

	// Count a row
	((CSQLite*)pThis )->m_nRows++;

	return nRet;
}

int CSQLite::OnCallback( int argc, char **argv, char **azColName )
{_STT();
	// What row are we on
	CPropertyBag &row = m_pbResult[ m_nRows ];

	for ( int i = 0; i < argc; i++ )
		if ( oexCHECK_PTR( azColName[ i ] ) )
			row[ oexMbToStr( azColName[ i ] ) ] = oexCHECK_PTR( argv[ i ] )
													? oexMbToStr( argv[ i ] )
													: oexT( "" );
		else
			row[ oexT( "*" ) ] = oexCHECK_PTR( argv[ i ] )
												? oexMbToStr( argv[ i ] )
												: oexT( "" );

	return 0;
}

void CSQLite::Destroy()
{_STT();
	// Lose results
	Clear();

	// Close any open database
	if ( m_sqobj )
	{	sqlite3_close( (sqlite3*)m_sqobj );
		m_sqobj = oexNULL;
	} // end if
}

void CSQLite::Clear()
{_STT();
	m_nRows = 0;
	m_sQuery.Destroy();
	m_pbResult.Destroy();

	OnClear();
}

oexBOOL CSQLite::Open( oexCSTR x_pDb )
{_STT();
	if ( !oexCHECK_PTR( x_pDb ) )
	{	oexERROR( 0, oexT( "Invalid parameter" ) );
		return oexFALSE;
	} // end if

	// Attempt to open database
	oexINT res = sqlite3_open( oexStrToMbPtr( x_pDb ), (sqlite3**)&m_sqobj );
	if ( res )
	{	m_sErr = oexMbToStr( sqlite3_errmsg( (sqlite3*)m_sqobj ) );
		oexERROR( res, m_sErr );
		Destroy();
		return oexFALSE;
	} // end if

	return oexTRUE;
}

oexBOOL CSQLite::IsTable( oexCSTR x_pTable )
{_STT();
	return QueryTableInfo( x_pTable ) && NumRows();
}

oexBOOL CSQLite::Exec( CStr x_sQuery )
{_STT();
	// Lose previous results
	Clear();

	// Sanity check params
	if ( !oexCHECK_PTR( m_sqobj ) || !x_sQuery.Length() )
		return oexFALSE;

	// Save the last query
	m_sQuery = x_sQuery;

	// Debug mode?
	if ( m_bDebugMode )
	{	oexNOTICE( 0, x_sQuery );
		oexEcho( x_sQuery.Ptr() );
	} // end if

	// Execute the query
	char *pErr = oexNULL;
	oexINT res = sqlite3_exec( (sqlite3*)m_sqobj, oexStrToMb( m_sQuery ).Ptr(), CSQLite::_Callback, this, &pErr );
	if ( SQLITE_OK != res )
	{ 	m_sErr = oexT( "Query Failed : " );
		m_sErr << m_sQuery << oexNL;
		if ( oexCHECK_PTR( pErr ) )
			m_sErr << oexMbToStr( pErr );
		else
			m_sErr << oexT( "Invalid pointer returned from sqlite3_exec()" );
		oexERROR( res, m_sErr );
		return oexFALSE;
	} // end if

	return oexTRUE;
}

oexBOOL CSQLite::QueryTableInfo( oexCSTR x_pTable )
{_STT();
	return Exec( CStr() << oexT( "SELECT * FROM SQLite_Master WHERE `name`='" )
					   << Escape( x_pTable ) << oexT( "'" ) );
}

oexBOOL CSQLite::QueryColumnInfo()
{_STT();
	return Exec( oexT( "PRAGMA table_info(test)" ) );
}

CStr CSQLite::Escape( CStr sStr )
{_STT();
	char *s = sqlite3_mprintf( "%q", oexStrToMb( sStr ).Ptr() );
	CStr sRet = oexMbToStr( s );
	sqlite3_free( s );
	return sRet;
}

oexBOOL CSQLite::Insert( oexCSTR pTable, CPropertyBag &pb )
{_STT();
	if ( !oexCHECK_PTR( pTable ) || !*pTable )
		return oexFALSE;

	// Create field / value strings
	CStr sF, sV;
	for ( oex::CPropertyBag::iterator it; pb.List().Next( it ); )
	{	if ( sF.Length() ) sF << oexT( "," ), sV << oexT( "," );
		sF << oexT( "'" ) << Escape( it.Node()->key ) << oexT( "'" );
		sV << oexT( "'" ) << Escape( it->ToString() ) << oexT( "'" );
	} // end for

	// Create insert command
	return Exec( CStr() << oexT( "INSERT INTO `" ) << pTable << oexT( "` (" )
		                << sF << oexT( ") VALUES(" ) << sV << oexT( ")" ) );
}

oexBOOL CSQLite::Update( oexCSTR pTable, oexCSTR pWhere, CPropertyBag &pb )
{_STT();
	if ( !oexCHECK_PTR( pTable ) || !*pTable
		 || !oexCHECK_PTR( pWhere ) || !*pWhere )
		return oexFALSE;

	// Create field=value strings
	CStr sV;
	for ( oex::CPropertyBag::iterator it; pb.List().Next( it ); )
	{	if ( sV.Length() ) sV << oexT( ", " );
		sV << oexT( "'" ) << Escape( it.Node()->key ) << oexT( "'='" )
						  << Escape( it->ToString() ) << oexT( "'" );
	} // end for

	// Create insert command
	return Exec( CStr() << oexT( "UPDATE `" ) << pTable
						<< oexT( "` SET " ) << sV << oexT( " WHERE " ) << pWhere );
}

oexBOOL CSQLite::Delete( oexCSTR pTable, CPropertyBag &pb, oexCSTR pCond )
{_STT();
	if ( !oexCHECK_PTR( pTable ) || !*pTable || !pb.Size() )
		return oexFALSE;

	if ( !oexCHECK_PTR( pCond ) || !*pCond )
		pCond = oexT( "AND" );

	// Create field=value strings
	CStr sV;
	for ( oex::CPropertyBag::iterator it; pb.List().Next( it ); )
	{	if ( sV.Length() ) sV << oexT( " " ) << pCond << oexT( " " );
		sV << oexT( "`" ) << Escape( it.Node()->key ) << oexT( "`='" )
						  << Escape( it->ToString() ) << oexT( "'" );
	} // end for

	// Create insert command
	return Exec( CStr() << oexT( "DELETE FROM " ) << pTable
						<< oexT( " WHERE " ) << sV );
}

#endif // OEX_ENABLE_SQLITE
