// =====================================================================
//  UAWError.h                           (C) Hotline Communications 1999
// =====================================================================
// 

#ifndef _H_UAWError_
#define _H_UAWError_

#if PRAGMA_ONCE
	#pragma once
#endif				    

HL_Begin_Namespace_BigRedH
class UAWError
{
	private:
		enum EConstants	{	TypeBitLen = 16,
							TypeOffset = (32-TypeBitLen)	};
	public:
		enum EAWError		
		{
			 eNone 						= 0
				// this will force a translation in the Exception constructor
				,eTranslate 
				// Whenever you get an OS error code, but it cannot be translated.
				,eUnknownOSError
				// Whenever you catch (...), throw with this.
				,eUnknownExceptionError
				// When you get a paramError, throw with this
				// i.e. any time parameters to an API are bogus
				,eRequire
			,eMemory					= 2 << TypeOffset
				,eMemoryNotEnough		= eMemory
				,eMemoryInvalid

			,eFile						= 3 << TypeOffset
				,eFileNotFound			= eFile
				,eAccessDenied
				,eEndOfFile
				,eNoSuchItem
				,eNoSuchDisk
				,eNoSuchFolder
				,eDifferentDisks
				,eItemLocked
				,eDiskLocked
				,eFileInUse
				,eFileExpected
				,eItemAlreadyExists
				,eNotEnoughSpace
				,eBadName
				,eBadMove
				,eDiskDamaged
				,eDiskDisappeared
				,eDiskOffline
				,eDiskSoftwareDamaged
				,eDiskFormatUnsupported
				,eSourceDestinationSame
				,eUnknownFileSysError
				,eFileAlreadyOpen

			,eNetwork					= 4 << TypeOffset
				,eNetworkAddressInUse	= eNetwork
				,eNetworkCannotResolve
				,eNetworkCannotCreate
				,eNetworkCannotBind
				,eNetworkCannotGetAvailable
				,eNetworkHostUnreachable
				,eNetworkUnknown
				,eNetworkConnectionTimedOut
				,eNetworkConnectionRefused
				,eNetworkBadAddress
				,eNetworkConnectionAborted

			,eResource					= 5 << TypeOffset
				,eResourceNotFound		= eResource
				,eResourceInvalid
				,eResourceAddFailed
				,eResourceRemoveFailed

			,eMisc						= 6 << TypeOffset
				,eTimeout				= eMisc
				,eServiceCannotCreate

			,eThread					= 7 << TypeOffset
				,eCreateThread			= eThread
				,eInvalidThreadID
				,eSemaphoreCannotCreate
		};

		static EAWError		GetErrorType( EAWError inCode )
								{ return EAWError(inCode & ~((1<<TypeBitLen)-1)); }

		static UInt32		GetErrorSubCode( EAWError inCode )
								{ return UInt32(inCode&((1<<TypeBitLen)-1)); }

		static EAWError		TranslateErrorCode( OS_ErrorCode inErrorCode );
		//static CString      MessageFromErrorCode( OS_ErrorCode inErrorCode );
}; // class CError

typedef UAWError::EAWError AW_ErrorCode;

HL_End_Namespace_BigRedH

#endif // _H_UAWError_