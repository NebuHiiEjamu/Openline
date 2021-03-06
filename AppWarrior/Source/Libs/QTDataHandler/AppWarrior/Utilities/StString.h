// =====================================================================
//  StString.h                           (C) Hotline Communications 2000
// =====================================================================
//
// Stack based string converters for temporary buffers.

#ifndef _H_StStringOS_
#define _H_StStringOS_

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "CString.h"
#include "UStringConverter.h"

HL_Begin_Namespace_BigRedH

// Convert from a CString to a C Style string.
class StCStyleString {
	public:
						StCStyleString (
							const CString& inParent
							, UStringConverter::EStringEncoding inEncoding 
								= UStringConverter::eDefaultEncoding
							, bool allowFallbacks = true )
							: mBuffer (
								UStringConverter::UnistringToCStr(
									inParent.c_str()
									,inEncoding
									,allowFallbacks))
							{}
	
						operator const char* () const 
							{ return mBuffer; }
						operator char* ()
							{ return mBuffer; }
		
						~StCStyleString()
							{ delete []mBuffer;	}

	private:
		char* 			mBuffer;
		
						StCStyleString	(const StCStyleString&);
						StCStyleString& operator= (const StCStyleString&);
};

// Convert from a CString to a C Style string,
// and copy the data back into the original CString
// on the destructor.
class StCopybackCStyleString {
	public:
		inline			StCopybackCStyleString (
								CString& inParent
								,std::size_t minSize = 0
								, UStringConverter::EStringEncoding inEncoding
									= UStringConverter::eDefaultEncoding
								,bool allowFallbacks = true);

						operator const char* () const 
							{ return mBuffer; }
						operator char* () 		
							{ return mBuffer; }
	
						~StCopybackCStyleString()
							{
								mParent = CString(mBuffer);
								delete []mBuffer;
							}

	private:
		CString& 		mParent;	
		char* 			mBuffer;
		
						StCopybackCStyleString(const StCopybackCStyleString&);
							
		StCopybackCStyleString& 
						operator= (const StCopybackCStyleString&);
};

// Convert a CString to a pascal style string
class StPStyleString {
public:
						StPStyleString (
							const CString& inParent
							, UStringConverter::EStringEncoding inEncoding
								= UStringConverter::eDefaultEncoding
							, bool allowFallbacks = true )
						{
							mBuffer = UStringConverter::UnistringToPStr(inParent.c_str()
								,inEncoding ,allowFallbacks);
						}

						operator const unsigned char* () const 
							{ return mBuffer; }
						operator unsigned char* () 
							{ return mBuffer; }
	
						~StPStyleString()
						{
							delete []mBuffer;
						}

	private:
		unsigned char* 	mBuffer;
		
						StPStyleString(const StPStyleString&);
		StPStyleString& operator= (const StPStyleString&);
};


// Convert from a CString to a pascal Style string,
// and copy the data back into the original CString
// on the destructor.
class StCopybackPStyleString {
	public:
		inline 			StCopybackPStyleString (
								CString& inParent
								,std::size_t minSize = 0
								, UStringConverter::EStringEncoding inEncoding
									= UStringConverter::eDefaultEncoding
								,bool allowFallbacks = true);
										
						operator const unsigned char* () const
							{ return mBuffer; }
						operator unsigned char* () 		
							{ return mBuffer; }
		
						~StCopybackPStyleString()
						{
							mParent = CString(mBuffer);
							delete []mBuffer;
						}

	private:
		CString& 		mParent;
		unsigned char* 	mBuffer;
		
						StCopybackPStyleString(const StCopybackPStyleString&);
						
	StCopybackPStyleString& 
						operator= (const StCopybackPStyleString&);
};


// ---------------------------------------------------------------------
//  StCopybackCStyleString                              [inline][public]
// ---------------------------------------------------------------------
// 
// Constructor

inline StCopybackCStyleString::StCopybackCStyleString (
						CString& inParent
						, std::size_t minSize
						, UStringConverter::EStringEncoding inEncoding
						, bool allowFallbacks)
	: mParent(inParent)
{
	std::size_t bufferSize = std::max(minSize, mParent.length() * 2 + 1);
	mBuffer = new char[bufferSize];
	// Append a null terminator
	mBuffer[mParent.length()] = '\0';
	for (std::size_t index = 0; index < mParent.length(); ++index)
		mBuffer[index] = UStringConverter::UnicharToChar(mParent[index]
			,inEncoding, allowFallbacks);
}


// ---------------------------------------------------------------------
//  StCopybackPStyleString                              [inline][public]
// ---------------------------------------------------------------------
// 
// Constructor

inline StCopybackPStyleString::StCopybackPStyleString (
							CString& inParent
							, std::size_t minSize
							, UStringConverter::EStringEncoding inEncoding
							, bool allowFallbacks)
	: mParent(inParent)
{
	std::size_t bufferSize = std::max(minSize, mParent.length() * 2 + 1);
	mBuffer = new unsigned char[bufferSize];
	// Stuff the length into the first element
	mBuffer[0] = static_cast<unsigned char>(mParent.length());
	for (std::size_t index = 0; index < mParent.length(); ++index)
		mBuffer[index+1] = static_cast<unsigned char> (
				UStringConverter::UnicharToChar(mParent[index]
						,inEncoding
						,allowFallbacks));
}


HL_End_Namespace_BigRedH
	
#endif