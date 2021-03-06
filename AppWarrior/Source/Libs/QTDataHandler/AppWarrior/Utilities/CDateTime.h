//======================================================================
//  CDateTime.h                          (C) Hotline Communications 1999
//======================================================================
// Syntax:
//  Month 1-12      
//  Day 1-31        
//  DayOfWeek 1-7:  1 sunday, 2 monday, ... , 7 saturday
//  Hour 0-23       
//  Minute 0-59     
//  Second 0-59     
//
#ifndef _H_UDateTime_
#define _H_UDateTime_

#if PRAGMA_ONCE
#   pragma once
#endif

#ifndef _H_AW_
#   include "AW.h"
#endif

HL_Begin_Namespace_BigRedH

class CDateTime
{
	public:
								// ctor/dtor
								CDateTime();
								
								CDateTime(
									UInt16 inYear,
									UInt16 inMonth = 1,
									UInt16 inDay = 1,
									UInt16 inHour = 0,
									UInt16 inMinute = 0,
									UInt16 inSecond = 0 );
								
								CDateTime(
									SInt64 inValue );

								CDateTime(
									const CDateTime& inOther );
								
		virtual					~CDateTime();

		SInt64					GetDateTime() const;

		static bool				IsDateValid(
									UInt16 inYear,
									UInt16 inMonth,
									UInt16 inDay );

		bool					IsBetween(
									const CDateTime& inD1,
									const CDateTime& inD2
								) const;

		static CDateTime		GetCurrentDateTime();
		
		static void				GetCurrentDateTime(
									UInt16& outYear,
									UInt16& outMonth,
									UInt16& outDay,
									UInt16& outHour,
									UInt16& outMinute,
									UInt16& outSecond );

		void					SetupTime(
									UInt16 inYear,
									UInt16 inMonth  = 1,
									UInt16 inDay    = 1,
									UInt16 inHour   = 0,
									UInt16 inMinute = 0,
									UInt16 inSecond = 0 );
									
		void					SetupRealTime(
									UInt16 inHour   = 0,
									UInt16 inMinute = 0,
									UInt16 inSecond = 0 );

		UInt16					GetYear();

		static bool				IsYearLeap(
									UInt16 inYear );

		UInt16					GetMonth();

		UInt16					GetDay();

								// Returns the day ( 1 = Sunday,
								// 7 = Saturday )
		UInt16					GetDayOfWeek();
		
		static UInt16			GetDayOfWeek(
									UInt16  inYear,
									UInt16 inMonth,
									UInt16   inDay );

		CDateTime				AddDays(
									UInt16 inDays );
									
		CDateTime				SubtractDays(
									UInt16 inDays );

			// ** Hours **

		UInt16					GetHours();

		CDateTime				AddHours(
									UInt16 inHours );
									
		CDateTime				SubtractHours(
									UInt16 inHours );

			// ** Minutes **

		UInt16					GetMinutes();

		CDateTime				AddMinutes(
									UInt16 inMinutes );
									
		CDateTime				SubtractMinutes(
									UInt16 inMinutes );

			// ** Seconds **

		UInt16					GetSeconds();

		CDateTime				AddSeconds(
									UInt16 inSeconds );
									
		CDateTime				SubtractSeconds(
									UInt16 inSeconds );
									
		const CDateTime&		operator = (
									const CDateTime& inOther );

		CDateTime				operator + (
									const CDateTime& inOther );

		CDateTime				operator - (
									const CDateTime& inOther );

		bool					operator <  (
									const CDateTime& inOther
								) const;
								
		bool					operator <= (
									const CDateTime& inOther
								) const;
								
		bool					operator >  (
									const CDateTime& inOther
								) const;
								
		bool					operator >= (
									const CDateTime& inOther
								) const;
								
		bool					operator == (
									const CDateTime& inOther
								) const;
								
		bool					operator != (
									const CDateTime& inOther
								) const;
								
    protected:
    
		static SInt32			DateToNumber(
									UInt16 inYear,
									UInt16 inMonth,
									UInt16 inDay );
									
		static void				NumberToDate(
									SInt32 inValue,
									UInt16& outYear,
									UInt16& outMonth,
									UInt16& outDay );
private:

		SInt64					mValue;
};

HL_End_Namespace_BigRedH

#endif // _H_UDateTime_

