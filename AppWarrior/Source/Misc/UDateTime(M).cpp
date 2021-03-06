#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UDateTime.h"

#include <Timer.h>

static Uint32 _DTDateToText(Uint32 inSecs, void *outText, Uint32 inMaxSize, Uint32 inOptions);
void _MacSecondsToStamp(Uint32 inSecs, SDateTimeStamp& outStamp);
Uint32 _StampToMacSeconds(const SDateTimeStamp& inStamp);

bool SDateTimeStamp::operator==(const SDateTimeStamp& dts) const
{
	return _StampToMacSeconds(*this) == _StampToMacSeconds(dts);
}

bool SDateTimeStamp::operator!=(const SDateTimeStamp& dts) const
{
	return _StampToMacSeconds(*this) != _StampToMacSeconds(dts);
}


bool SDateTimeStamp::operator>(const SDateTimeStamp& dts) const
{
	return _StampToMacSeconds(*this) > _StampToMacSeconds(dts);
}

bool SDateTimeStamp::operator<(const SDateTimeStamp& dts) const
{
	return _StampToMacSeconds(*this) < _StampToMacSeconds(dts);
}

bool SDateTimeStamp::operator>=(const SDateTimeStamp& dts) const
{
	return _StampToMacSeconds(*this) >= _StampToMacSeconds(dts);
}

bool SDateTimeStamp::operator<=(const SDateTimeStamp& dts) const
{
	return _StampToMacSeconds(*this) <= _StampToMacSeconds(dts);
}

Uint32 SDateTimeStamp::Flatten(void *outData)
{
	*((SDateTimeStamp*)(outData)) = *this;
	return sizeof(SDateTimeStamp);
}

Uint32 SDateTimeStamp::Unflatten(const void *inData)
{
	*this = *((SDateTimeStamp*)(inData));
	return sizeof(SDateTimeStamp);
}

#pragma mark -

/*
 * GetSeconds() returns the number of seconds that have elapsed since a certain
 * time.  It's useful for elapsed time measurements in the minutes or hours.
 */
Uint32 UDateTime::GetSeconds()
{
	Uint32 n;
	::GetDateTime(&n);
	return n;
}

/*
 * GetMilliseconds() returns the number of milliseconds (1 sec = 1000 ms) that
 * have elapsed since a certain time (typically since the computer started up,
 * but don't rely on it).  It's useful for elapsed time measurements over a
 * short period - don't use GetMilliseconds() over a long time period (more
 * than a minute).
 */
Uint32 UDateTime::GetMilliseconds()
{
	UnsignedWide microsecs;
	::Microseconds(&microsecs);

	return (microsecs.lo / 1000);			// 1 second = 1000000 microseconds

// **** is this better?  will it overflow?
//	return (microsecs.hi*4294967L + microsecs.lo/1000);		// 4294967L = 2^32 / 1000
}

/*
 * GetCalendarDate() sets <outInfo> to the current date and time in the
 * calendar format specified by <inCalendar>.
 */
void UDateTime::GetCalendarDate(Uint32 inCalendar, SCalendarDate& outInfo)
{
	Require(inCalendar == calendar_Gregorian);
	Uint32 secs;
	::GetDateTime(&secs);
	::SecondsToDate(secs, (DateTimeRec *)&outInfo);
	outInfo.val = 0;
}

void UDateTime::GetDateTimeStamp(SDateTimeStamp& outInfo)
{
	Uint32 secs;
	::GetDateTime(&secs);
	
	outInfo.year = 1904;
	outInfo.msecs = 0;
	outInfo.seconds = secs;
}

Uint32 UDateTime::DateToText(const SCalendarDate& inInfo, void *outText, Uint32 inMaxSize, Uint32 inOptions)
{
	Uint32 secs;
	::DateToSeconds((DateTimeRec *)&inInfo, &secs);
	return _DTDateToText(secs, outText, inMaxSize, inOptions);
}

Uint32 UDateTime::DateToText(const SDateTimeStamp& inInfo, void *outText, Uint32 inMaxSize, Uint32 inOptions)
{
	return _DTDateToText(_StampToMacSeconds(inInfo), outText, inMaxSize, inOptions);
}

Uint32 UDateTime::DateToText(void *outText, Uint32 inMaxSize, Uint32 inOptions)
{
	Uint32 secs;
	::GetDateTime(&secs);
	return _DTDateToText(secs, outText, inMaxSize, inOptions);
}

// return the difference, in seconds, between local time and GMT
Int32 UDateTime::GetGMTDelta()
{
	MachineLocation stTimeZoneInfo;
	ClearStruct(stTimeZoneInfo);
	
	::ReadLocation(&stTimeZoneInfo);		
	Int32 nGMTDelta = stTimeZoneInfo.u.gmtDelta & 0x00FFFFFF;
	
	if (GetBit(&nGMTDelta, 23))
		nGMTDelta = nGMTDelta | 0xFF000000;
			
	return nGMTDelta;
}

static Uint32 _DTDateToText(Uint32 inSecs, void *outText, Uint32 inMaxSize, Uint32 inOptions)
{
	Require(inMaxSize > 31);
	Str255 dateStr, timeStr;
	Uint8 *p = (Uint8 *)outText;
	
	dateStr[0] = timeStr[0] = 0; 
	
	if (inOptions & kShortDateText)
		::DateString(inSecs, shortDate, dateStr, nil);
	else if (inOptions & kShortDateFullYearText)
	{
		DateTimeRec dateTime;
		::SecondsToDate(inSecs, &dateTime);
		dateStr[0] = UText::Format(dateStr + 1, inMaxSize, "%hu/%hu/%hu", dateTime.month, dateTime.day, dateTime.year);
	}
	else if (inOptions & kAbbrevDateText)
		::DateString(inSecs, abbrevDate, dateStr, nil);
	else if (inOptions & kLongDateText)
		::DateString(inSecs, longDate, dateStr, nil);
	
	if (inOptions & kTimeText)
		::TimeString(inSecs, false, timeStr, nil);
	else if (inOptions & kTimeWithSecsText)
		::TimeString(inSecs, true, timeStr, nil);

	if (dateStr[0])
	{
		::BlockMoveData(dateStr+1, p, dateStr[0]);
		p += dateStr[0];
	}

	if (dateStr[0] && timeStr[0])
	{
		if (inOptions & (kShortDateText|kShortDateFullYearText))
			*p++ = ' ';
		else
		{
			*p++ = ',';
			*p++ = ' ';
		}
	}
	
	if (timeStr[0])
	{
		::BlockMoveData(timeStr+1, p, timeStr[0]);
		p += timeStr[0];
	}
	
	return p - (Uint8 *)outText;
}

void _MacSecondsToStamp(Uint32 inSecs, SDateTimeStamp& outStamp)
{
	outStamp.year = 1904;		// mac seconds start at midnight 1/1/1904
	outStamp.msecs = 0;
	outStamp.seconds = inSecs;
}

Uint32 _StampToMacSeconds(const SDateTimeStamp& inStamp)
{
	DateTimeRec dt;
	Uint32 secs;

	if (inStamp.year < 1904) return 0;	// mac seconds can't represent dates older than 1904

	dt.year = inStamp.year;
	dt.month = dt.day = 1;
	dt.hour = dt.minute = dt.second = dt.dayOfWeek = 0;

	::DateToSeconds(&dt, &secs);
	
	return secs + inStamp.seconds + (inStamp.msecs / 1000);
}








#endif /* MACINTOSH */
