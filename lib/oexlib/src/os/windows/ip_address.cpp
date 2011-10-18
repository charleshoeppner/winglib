/*------------------------------------------------------------------
// ip_address.cpp
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
#include "std_os.h"

OEX_USING_NAMESPACE
using namespace OEX_NAMESPACE::os;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

oexBOOL CIpAddress::SetRawAddress( oexINT64 x_llIp, oexINT32 x_uPort, oexINT32 x_uType )
{_STT();
    // Set the ip address
    if ( x_uType == eAddrTypeIpv4 )
        m_uIpv4 = (oexUINT32)x_llIp, m_uIpv4Extra = 0;
    else m_llIpv6 = x_llIp;

    // Port number
    m_uPort = x_uPort;

    // Type information
    m_uType = x_uType;

    // Set the crc
    m_uCrc = 0;
    oexUCHAR ucHash[ CCrcHash::eHashSize ];
    CCrcHash::Hash( &ucHash, &m_guid, sizeof( m_guid ) );
    m_uCrc = *(oexUINT16*)&ucHash;

    return oexTRUE;
}

oexBOOL CIpAddress::ValidateAddress()
{_STT();
    // Save the crc
    oexUINT16 uCrc = m_uCrc;

	// Clear the crc while we hash
	m_uCrc = 0;

    // Create hash
    oexUCHAR ucHash[ CCrcHash::eHashSize ];
    CCrcHash::Hash( &ucHash, &m_guid, sizeof( m_guid ) );

    // Verify the hash value
    if ( uCrc != *(oexUINT16*)&ucHash )
    {   oexASSERT( 0 ); 
		Destroy(); 
		return oexFALSE; 
	} // end if

    // Restore the crc
    m_uCrc = uCrc;

    return oexTRUE;
}

CStr CIpAddress::GetHostName()
{_STT();
	
	// Look up the host name
	char szHostName[ MAX_PATH ] = { 0 };
	if( ::gethostname( szHostName, sizeof( szHostName ) ) )
		return CStr();
	
	return CStr( oexMbToStr( szHostName ) );
}

CStr CIpAddress::GetFullHostName()
{_STT();
	CStr sDomain = GetDomainName();
	if ( sDomain.Length() )
		return sDomain << oexT( "\\" ) << GetHostName();
//		return GetHostName() << oexT( "." ) << sDomain;
	return GetHostName();
}

#define NET_API_FUNCTION __stdcall
typedef WCHAR* LMSTR;
typedef DWORD NET_API_STATUS;
typedef struct _WKSTA_INFO_100 
{	DWORD wki100_platform_id;
	LMSTR wki100_computername;
	LMSTR wki100_langroup;
	DWORD wki100_ver_major;
	DWORD wki100_ver_minor;
} WKSTA_INFO_100, *PWKSTA_INFO_100, *LPWKSTA_INFO_100;

typedef NET_API_STATUS (NET_API_FUNCTION *pfn_NetWkstaGetInfo)( LPWSTR servername, DWORD level, LPBYTE *bufptr );
typedef NET_API_STATUS (NET_API_FUNCTION *pfn_NetApiBufferFree)( LPVOID Buffer );

CStr CIpAddress::GetDomainName( oexCSTR x_pServer )
{_STT();

	CStr sRet;

	// Load netapi32.dll
	HMODULE hLib = LoadLibrary( oexT( "netapi32.dll" ) );
	if ( !hLib )
		return sRet;

	// Get function pointers
	pfn_NetApiBufferFree pNetApiBufferFree = (pfn_NetApiBufferFree)GetProcAddress( hLib, oexT( "NetApiBufferFree" ) );
	pfn_NetWkstaGetInfo pNetWkstaGetInfo = (pfn_NetWkstaGetInfo)GetProcAddress( hLib, oexT( "NetWkstaGetInfo" ) );

	// Attempt to read the domain name
	WKSTA_INFO_100 *pwi100 = oexNULL;
	if ( pNetWkstaGetInfo 
		 && !pNetWkstaGetInfo( ( x_pServer && *x_pServer ) ? (LPWSTR)oexStrToStrWPtr( x_pServer ) : oexNULL, 100, (LPBYTE*)&pwi100 ) )
		if ( pwi100 && pwi100->wki100_langroup )
			sRet = oexStrWToStr( pwi100->wki100_langroup );

	// Free buffer
	if ( pNetApiBufferFree && pwi100 )
		pNetApiBufferFree( pwi100 );

	// Free library
	FreeLibrary( hLib );

	// Send the domain name along
	return sRet;
}

oexBOOL CIpAddress::SetDotAddress( oexCSTR x_pDotAddress, oexINT32 x_uPort, oexINT32 x_uType )
{_STT();
#if defined( OEX_NOSOCKET2 )
	return oexFALSE;
#else
    if ( !oexVERIFY( x_pDotAddress ) )
        return oexFALSE;

    // Convert the dot address
    u_long ip = ntohl( inet_addr( oexStrToStr8Ptr( x_pDotAddress ) ) );
    if ( INADDR_NONE == ip )
        return oexFALSE;

    SetRawAddress( ip, x_uPort, x_uType );

    return oexTRUE;
#endif
}

CStr CIpAddress::GetDotAddress()
{_STT();
#if defined( OEX_NOSOCKET2 )
	return oexFALSE;
#else
    in_addr ia;

    // Put the address in the structure
    ia.S_un.S_addr = htonl( (u_long)GetIpv4() );

	// Create dot address if needed
	return oexStr8ToStr( inet_ntoa( ia ) );
#endif
}

oexCONST oexGUID* CIpAddress::GetId( oexGUID *x_pGuid )
{_STT();
    os::CSys::MemCpy( x_pGuid, &m_guid, sizeof( m_guid ) );
    return x_pGuid;
}

CIpAddress& CIpAddress::SetId( oexCONST oexGUID *x_pGuid )
{_STT();
    os::CSys::MemCpy( &m_guid, x_pGuid, sizeof( m_guid ) );
    return *this;
}

CStr CIpAddress::BuildUrl( CPropertyBag &x_pbUi )
{_STT();
	CStr str;

	// Scheme
	if ( x_pbUi[ oexT( "scheme" ) ].ToString().Length() )
		str << x_pbUi[ oexT( "scheme" ) ].ToString() << oexT( "://" );

	// Username and password?
	if ( x_pbUi[ oexT( "username" ) ].ToString().Length() )
	{
		if ( x_pbUi[ oexT( "password" ) ].ToString().Length() )
			str << x_pbUi[ oexT( "username" ) ].ToString()
			    << oexT( ":" )
			    << x_pbUi[ oexT( "password" ) ].ToString()
			    << oexT( "@" );
		else
			str << x_pbUi[ oexT( "username" ) ].ToString() << oexT( "@" );

	} // end if

	// Username and password?
	if ( x_pbUi[ oexT( "host" ) ].ToString().Length() )
	{
		if ( x_pbUi[ oexT( "host" ) ].ToString().Length() )
			str << x_pbUi[ oexT( "host" ) ].ToString()
			    << oexT( ":" )
			    << x_pbUi[ oexT( "port" ) ].ToString();
		else
			str << x_pbUi[ oexT( "host" ) ].ToString();

	} // end if

	// Ensure separator
	if ( oexT( '/' ) != *x_pbUi[ oexT( "path" ) ].ToString().Ptr()
	     && oexT( '\\' ) != *x_pbUi[ oexT( "path" ) ].ToString().Ptr() )
		str << oexT( '/' );

	// Append the path
	str << x_pbUi[ oexT( "path" ) ].ToString();

	// Adding separator
	if ( x_pbUi[ oexT( "extra" ) ].ToString().Length() )
		str << oexT( "?" ) << x_pbUi[ oexT( "extra" ) ].ToString();

	return str;
}


// Assuming formating like...
//  http://user:password@www.somesite.com/directory/somefile.php?param=1&param=2
CPropertyBag CIpAddress::ParseUrl( oexCSTR pUrl, oexUINT uMaxBufferSize )
{_STT();
    CPropertyBag pb;

    if ( !oexVERIFY( pUrl ) )
        return CPropertyBag();

    CStr str = pUrl;

	// Read in the scheme
	pb[ oexT( "scheme" ) ].ToString() = str.Parse( oexT( ":" ) );

	// Trim off leading forward slashes
	str.LTrim( oexT( ":/" ) );

	// Is there a username / password?
	CStr tmp = str.Parse( oexT( "@" ) );
	if ( tmp.Length() )
	{
		// Skip the @
		str++;

		// Divide username and password
		CStr s = tmp.Parse( oexT( ":" ) );
		if ( s.Length () )
		{	pb[ oexT( "username" ) ].ToString() = s;
			tmp++; pb[ oexT( "password" ) ].ToString() = tmp;
		} // end if
		else
			pb[ oexT( "username" ) ].ToString() = tmp;

	} // end if

	// Is there a username / password?
	tmp = str.Parse( oexT( "/" ) );
	if ( tmp.Length() )
	{
		CStr s = tmp.Parse( oexT( ":" ) );
		if ( s.Length () )
		{	pb[ oexT( "host" ) ].ToString() = s;
			tmp++; pb[ oexT( "port" ) ].ToString() = tmp;
		} // end if
		else
			pb[ oexT( "host" ) ].ToString() = tmp;

	} // end if

	// Grab the path
	pb[ oexT( "path" ) ].ToString() = str.Parse( oexT( "?" ) );

	if ( str.Length() )
	{
		// Trim separator if any
		if ( oexT( '?' ) == *str.Ptr() )
		{
			str.LTrim( 1 );

			// Anything left over?
			if ( str.Length() )
				pb[ oexT( "extra" ) ].ToString() = str;
		} // end if

		else
			pb[ oexT( "path" ) ].ToString() += str; 

	} // end if

	return pb;
}

/*

	// Dropping this MS stuff because of problems in WINCE

CStr CIpAddress::BuildUrl( CPropertyBag &x_pbUi )
{_STT();
    URL_COMPONENTS uc;
    os::CSys::Zero( &uc, sizeof( uc ) );
    uc.dwStructSize = sizeof( URL_COMPONENTS );

    uc.dwSchemeLength = x_pbUi[ oexT( "scheme" ) ].ToString().Length();
    uc.lpszScheme = x_pbUi[ oexT( "scheme" ) ].ToString()._Ptr();

    uc.dwHostNameLength = x_pbUi[ oexT( "host" ) ].ToString().Length();
    uc.lpszHostName = x_pbUi[ oexT( "host" ) ].ToString()._Ptr();

    uc.dwUserNameLength = x_pbUi[ oexT( "username" ) ].ToString().Length();
    uc.lpszUserName = x_pbUi[ oexT( "username" ) ].ToString()._Ptr();

    uc.dwPasswordLength = x_pbUi[ oexT( "password" ) ].ToString().Length();
    uc.lpszPassword = x_pbUi[ oexT( "password" ) ].ToString()._Ptr();

    uc.dwUrlPathLength = x_pbUi[ oexT( "path" ) ].ToString().Length();
    uc.lpszUrlPath = x_pbUi[ oexT( "path" ) ].ToString()._Ptr();

    // !!! Right or wrong, I'm going to hide this detail.
    CStr extra;
    if ( x_pbUi[ oexT( "extra" ) ].ToString().Length() )
    {   extra << oexT( "?" ) << x_pbUi[ oexT( "extra" ) ].ToString();
        uc.dwExtraInfoLength = extra.Length();
        uc.lpszExtraInfo = extra._Ptr();
    } // end if

    else
    {   uc.dwExtraInfoLength = x_pbUi[ oexT( "extra" ) ].ToString().Length();
        uc.lpszExtraInfo = x_pbUi[ oexT( "extra" ) ].ToString()._Ptr();
    } // end else

    uc.nScheme = (INTERNET_SCHEME)x_pbUi[ oexT( "scheme_id" ) ].ToLong();

    uc.nPort = (INTERNET_PORT)x_pbUi[ oexT( "port" ) ].ToLong();

    CStr str;
    DWORD dwLen = oexSTRSIZE;
    if ( !str.OexAllocate( oexSTRSIZE ) )
        return CStr();

    // Attempt to create url
    if ( !InternetCreateUrl( &uc, ICU_ESCAPE, str._Ptr(), &dwLen ) )
    {
        // Was the buffer too small?
        if ( ERROR_INSUFFICIENT_BUFFER != GetLastError() )
            return CStr();

        dwLen += 2;
        if ( !str.Allocate( dwLen ) )
            return CStr();

        // Try again
        if ( !InternetCreateUrl( &uc, ICU_ESCAPE, str._Ptr(), &dwLen ) )
            return CStr();

    } // end if

    return str;
}

CPropertyBag CIpAddress::ParseUrl( oexCSTR pUrl, oexUINT uMaxBufferSize )
{
    CPropertyBag pb;

    if ( !oexVERIFY( pUrl ) )
        return CPropertyBag();

    oexUINT uLen = zstr::Length( pUrl );

    // I assume the components can't be longer than the url
    if ( !uMaxBufferSize )
        uMaxBufferSize = uLen + 2;

    URL_COMPONENTS uc;
    os::CSys::Zero( &uc, sizeof( uc ) );
    uc.dwStructSize = sizeof( URL_COMPONENTS );

    uc.dwSchemeLength = uMaxBufferSize;
    uc.lpszScheme = pb[ oexT( "scheme" ) ].ToString().OexAllocate( uMaxBufferSize );

    uc.dwHostNameLength = uMaxBufferSize;
    uc.lpszHostName = pb[ oexT( "host" ) ].ToString().OexAllocate( uMaxBufferSize );

    uc.dwUserNameLength = uMaxBufferSize;
    uc.lpszUserName = pb[ oexT( "username" ) ].ToString().OexAllocate( uMaxBufferSize );

    uc.dwPasswordLength = uMaxBufferSize;
    uc.lpszPassword = pb[ oexT( "password" ) ].ToString().OexAllocate( uMaxBufferSize );

    uc.dwUrlPathLength = uMaxBufferSize;
    uc.lpszUrlPath = pb[ oexT( "path" ) ].ToString().OexAllocate( uMaxBufferSize );

    uc.dwExtraInfoLength = uMaxBufferSize;
    uc.lpszExtraInfo = pb[ oexT( "extra" ) ].ToString().OexAllocate( uMaxBufferSize );

    // Attempt to crack the url
    if ( !InternetCrackUrl( pUrl, uLen, ICU_DECODE | ICU_ESCAPE, &uc ) )
        return CPropertyBag();

    // Grab the port and scheme id
    pb[ oexT( "scheme_id" ) ] = uc.nScheme;
    pb[ oexT( "port" ) ] = uc.nPort;

    // !!! What a pain... See the above hack to hide this detail
    if ( oexT( '?' ) == *pb[ oexT( "extra" ) ].ToString().Ptr() )
        pb[ oexT( "extra" ) ].ToString().LTrim( 1 );

    return pb;
}
*/

oexBOOL CIpAddress::LookupUrl( oexCSTR x_pUrl, oexINT32 x_uPort, oexINT32 x_uType )
{_STT();
    // Lose old info
    Destroy();

    // Ensure we have a valid pointer
    if ( !oexVERIFY_PTR( x_pUrl ) )
        return oexFALSE;

    // Crack the url
    CPropertyBag pbUrl = ParseUrl( x_pUrl );
    if ( !pbUrl.Size() )
        return oexFALSE;

    // Did we get a host name?
    if ( !pbUrl[ oexT( "host" ) ].ToString().Length() )
        return oexFALSE;

    // Get the port
    if ( !x_uPort )
        x_uPort = pbUrl[ oexT( "port" ) ].ToLong();

    // Save the type
    m_uType = x_uType;

    return LookupHost( pbUrl[ oexT( "host" ) ].ToString().Ptr(), x_uPort );
}

oexBOOL CIpAddress::LookupHost( oexCSTR x_pServer, oexINT32 x_uPort, oexINT32 x_uType )
{_STT();
#if defined( OEX_NOSOCKET2 )
	return oexFALSE;
#else
    // Lose old info
    Destroy();

    // Ensure we have a valid pointer
    if ( !oexVERIFY_PTR( x_pServer ) )
        return oexFALSE;

	// First try to interpret as dot address
	ULONG uAddr = inet_addr( oexStrToStr8Ptr( x_pServer ) );
	if ( INADDR_NONE == uAddr )
    {
        LPHOSTENT pHe = gethostbyname( oexStrToStr8Ptr( x_pServer ) );

        if ( !pHe )
            return oexFALSE;

        LPIN_ADDR pia = (LPIN_ADDR)*pHe->h_addr_list;
        if ( !oexVERIFY_PTR( pia ) )
            return oexFALSE;

        // Grab the address
        uAddr = *(DWORD*)&pia->S_un.S_addr;

    } // end if

    SetRawAddress( ntohl( uAddr ), x_uPort, x_uType );

    return oexTRUE;
#endif
}

