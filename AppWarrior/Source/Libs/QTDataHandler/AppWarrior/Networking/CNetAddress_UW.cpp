/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// IP address

#include "AW.h"
#include "CNetAddress.h"
#if TARGET_OS_UNIX
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>
	#define WSAGetLastError() errno
#endif
#include "StString.h"
#include "CNetworkException.h"

HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  CNetAddress                                                 [public]
// ---------------------------------------------------------------------
// Default Constructor

CNetAddress::CNetAddress()
	: mIP(0), mPort(0)
{
}

// ---------------------------------------------------------------------
//  CNetAddress                                                 [public]
// ---------------------------------------------------------------------
// Constructor

CNetAddress::CNetAddress( const CString& inName, UInt16 inPort, bool inInit )
		: mIP(0), mPort( ::htons( inPort ) ), mName(inName)
{
	if(inInit)
	{
		GetIP();
	}
}


// ---------------------------------------------------------------------
//  CNetAddress                                                 [public]
// ---------------------------------------------------------------------
// Constructor

CNetAddress::CNetAddress( UInt32 inIP, UInt16 inPort )
		: mIP( ::htonl( inIP ) ), mPort( ::htons( inPort ) )
{
}


// ---------------------------------------------------------------------
//  CNetAddress                                                 [public]
// ---------------------------------------------------------------------
// Copy Constructor

CNetAddress::CNetAddress( const CNetAddress& inOther)
{
	operator = (inOther);
}


// ---------------------------------------------------------------------
//  ~CNetAddress                                                [public]
// ---------------------------------------------------------------------
// Destructor

CNetAddress::~CNetAddress()
{
}


// ---------------------------------------------------------------------
//  operator =                                                  [public]
// ---------------------------------------------------------------------
//

CNetAddress&
CNetAddress::operator = ( const CNetAddress& inOther )
{
	if(this != &inOther)
	{
		mIP   = inOther.mIP;
		mPort = inOther.mPort;
		mName = inOther.mName;
	}
	return *this;
}


// ---------------------------------------------------------------------
//  GetIP                                                       [public]
// ---------------------------------------------------------------------
// Returns IP address number. If not set, it tries to resolve it.

UInt32
CNetAddress::GetIP() const
{
	try
	{
		if( (mIP == 0) && !mName.empty() )
		{
			StCStyleString s(const_cast<CString&>(mName));
			UInt32 ipAddress;

			if((ipAddress = inet_addr(s)) == INADDR_NONE)
			{
				struct hostent* entry = gethostbyname(s);

				if(entry == NULL)
					THROW_NET_( eResolvingAddress, eNetworkCannotResolve, *this, OS_ErrorCode(::WSAGetLastError()) );
				ipAddress = inet_addr(inet_ntoa(*(in_addr*)*(entry->h_addr_list)));
				if (ipAddress == INADDR_NONE)
					THROW_NET_( eResolvingAddress, eNetworkCannotResolve, *this, kNoOSError );
			}
			mIP = ipAddress;
		}
	}
	catch (...)
	{
		RETHROW_NET_(eResolvingAddress,*this);
	}
	return mIP;
}


// ---------------------------------------------------------------------
//  GetName                                                     [public]
// ---------------------------------------------------------------------
// Returns a domain name. If not set, it's resolved.

const CString&
CNetAddress::GetName() const
{
	try
	{
		if( (mIP != 0) && mName.empty() )
		{
			hostent* entry = gethostbyaddr((char*)&mIP, 4, AF_INET);

			if(entry == NULL)
				THROW_NET_( eResolvingAddress, eNetworkCannotResolve, *this, OS_ErrorCode(::WSAGetLastError()) );
			mName = CString(entry->h_name);
		}
	}
	catch (...)
	{
		RETHROW_NET_(eResolvingAddress,*this);
	}
	return mName;
}


// ---------------------------------------------------------------------
//  GetHostAddress                                              [public]
// ---------------------------------------------------------------------

void
CNetAddress::GetHostAddress() const
{
	try
	{
		//
		const SInt16 _buffer_size_ = 512;
		//
		std::auto_ptr<char> buffer(new char[_buffer_size_]);
		
		if(gethostname(buffer.get(), _buffer_size_) != 0)
		{
			THROW_NET_(eGetHostAddress, eNetworkCannotResolve, *this, OS_ErrorCode(WSAGetLastError()));
		}

		hostent* entry;

		if((entry = gethostbyname(buffer.get())) == nil)
		{
			THROW_NET_(eGetHostAddress, eNetworkCannotResolve, *this, OS_ErrorCode(WSAGetLastError()));
		}

		UInt32 ip = inet_addr(inet_ntoa(*(in_addr*)*(entry->h_addr_list)));

		if(ip == INADDR_NONE)
		{
			THROW_NET_(eResolvingAddress, eNetworkCannotResolve, *this, kNoOSError);
		}

		mIP = ip;
	}
	catch(...)
	{
		RETHROW_NET_(eGetHostAddress, *this);
	}
}


HL_End_Namespace_BigRedH