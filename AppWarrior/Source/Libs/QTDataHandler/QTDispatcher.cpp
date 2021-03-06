/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
//
//  platform-independed implementation of QuickTime component's routines

#include "aw.h"
#undef PRAGMA_ONCE // to avoid macro redefinition warning
#include <Components.h>
#include <QuickTimeComponents.h>
#include "QTDispatcher.h"
#include "CDataHandlerComponent.h"
#include "CDataHandlerConnection.h"

HL_Begin_Namespace_BigRedH

// Note: all below functions are called through the main dispatcher.
// All of them have to be defined using pascal calling convention


// ---------------------------------------------------------------------
//  DataHandlerOpen 
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kComponentOpenSelect selector
// We ask the component class to costruct new connection object here
pascal ComponentResult DataHandlerOpen(CDataHandlerConnection* handler, ComponentInstance self)
{
	try
	{
		CDataHandlerConnection* new_connection = 
			CDataHandlerComponent::Instance().NewConnection();

		SetComponentInstanceStorage(self, (Handle)new_connection);
		#if TARGET_OS_MAC
//		SetComponentInstanceA5(self, *(long *)0x904);
		#endif
		return new_connection==nil? memFullErr: noErr;
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerOpen\n");
		return memFullErr;
	}
}

// ---------------------------------------------------------------------
//  DataHandlerClose
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kComponentCloseSelect selector
// We ask the component class to destroy given connection object here
pascal ComponentResult DataHandlerClose(CDataHandlerConnection* handler, ComponentInstance self)
{
	try
	{
		CDataHandlerComponent::Instance().DeleteConnection(handler);
		return noErr;
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerClose\n");
		return paramErr;
	}
}

// ---------------------------------------------------------------------
//  DataHandlerCanDo
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kComponentCanDoSelect selector
// Component class is responding which selectors it can recognize and respond to
pascal ComponentResult DataHandlerCanDo( CDataHandlerConnection* handler, short selector)
{
	try
	{
		return (ComponentResult)CDataHandlerComponent::Instance().CanDo(selector);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerCanDo\n");
		return memFullErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerVersion
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kComponentVersionSelect selector
// The component class returns current version of the component
pascal ComponentResult DataHandlerVersion( CDataHandlerConnection* handler)
{
	try
	{
		return (ComponentResult)CDataHandlerComponent::Instance().GetVersion();
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerVersion\n");
		return memFullErr;
	}
}






// ---------------------------------------------------------------------
//  DataHandlerCanUseDataRef
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHCanUseDataRefSelect selector
// The component class tells if it can handle such dataref
pascal ComponentResult DataHandlerCanUseDataRef(CDataHandlerConnection* handler, Handle dataref, long * flags)
{
	try
	{
		if (CDataHandlerComponent::Instance().CanUseDataRef(dataref))
			*flags = kDataHCanRead | kDataHSpecialRead;
		else
			*flags = 0;
		return noErr;
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerCanUseDataRef\n");
		return memFullErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerSetDataRef
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHSetDataRefSelect selector
// Using this request, QuickTime tells given connection to use the DataRef
// It is a good place to start downloading of an external file
pascal ComponentResult DataHandlerSetDataRef(CDataHandlerConnection* handler, Handle dataref)
{
	try
	{
		return handler->SetDataRef(dataref);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerSetDataRef\n");
		return openErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerGetFileName
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHGetFileNameSelect selector
// The connection class returns a string with a filename, so QuickTime can determine 
// if it has a ".mov" extention, and show the string as a caption
pascal ComponentResult DataHandlerGetFileName (CDataHandlerConnection* handler, StringPtr filename)
{
	try
	{
		return handler->GetFileName(filename);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerGetFileName\n");
		return paramErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerGetMimeType
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHGetMimeTypeSelect selector
// The connection class returns a string with a mimetype, so QuickTime can determine the file type
pascal ComponentResult DataHandlerGetMimeType (CDataHandlerConnection* handler, StringPtr mimetype)
{
	try
	{
		return handler->GetMimeType(mimetype);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerGetMimeType\n");
		return openErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerGetFileType
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHGetFileTypeSelect selector
// The connection class returns a OSType for the file
pascal ComponentResult DataHandlerGetFileType (CDataHandlerConnection* handler, OSType *osType)
{
	try
	{
		return handler->GetFileType(osType);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerGetFileType\n");
		return openErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerGetAvailableFileSize
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHGetAvailableFileSizeSelect selector
// The connection class returns the amount of data it has for this moment
pascal ComponentResult  DataHandlerGetAvailableFileSize (CDataHandlerConnection* handler, long * filesize)
{
	try
	{
		return handler->GetAvailableSize(filesize);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerGetAvailableFileSize\n");
		return openErr;
	}
}

// ---------------------------------------------------------------------
//  DataHandlerGetFileSize
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHGetFileSizeSelect selector
// The connection class returns the size of the whole file
pascal ComponentResult DataHandlerGetFileSize(CDataHandlerConnection* handler, long * filesize)
{
	try
	{
		return handler->GetFullSize(filesize);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerGetFileSize\n");
		return openErr;
	}
}

// ---------------------------------------------------------------------
//  DataHandlerOpenForRead
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHOpenForReadSelect selector
// A connection class must open a file (or another data source) and be ready 
// to read data from it. Note: QuickTime actually don't use this selector, so
// we must open the file ourselves and keep it open for a future use
pascal ComponentResult DataHandlerOpenForRead(CDataHandlerConnection* handler)
{
	try
	{
		return handler->OpenForRead();
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerOpenForRead\n");
		return openErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerCloseForRead
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHCloseForReadSelect selector
// A connection class must close a file (or another data source).
pascal ComponentResult DataHandlerCloseForRead(CDataHandlerConnection* handler)
{
	try
	{
		return handler->CloseForRead();
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerCloseForRead\n");
		return closErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerGetScheduleAheadTime
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHGetScheduleAheadTimeSelect selector
// This is to tell QuickTime how much time ahead we prefer to read before play it
pascal ComponentResult DataHandlerGetScheduleAheadTime(CDataHandlerConnection* handler, long * milliseconds)
{
	try
	{
		*milliseconds = CDataHandlerComponent::Instance().GetAheadTime();
		return noErr;
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerGetScheduleAheadTime\n");
		return memFullErr;
	}
}

// ---------------------------------------------------------------------
//  DataHandlerCompareDataRef
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHCompareDataRefSelect selector
// QuickTime asks us if two DataRefs are pointing to the same data source
pascal ComponentResult DataHandlerCompareDataRef(CDataHandlerConnection* handler, Handle dataref, Boolean *equal)
{
	try
	{
		*equal = handler->CompareDataRef(dataref);
		return noErr;
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerCompareDataRef\n");
		return paramErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerDataTask
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHDataTaskSelect selector
// QuickTime calls that function frequently during playback.
// Currently the handler does nothing at all.
pascal ComponentResult  DataHandlerDataTask (CDataHandlerConnection* handler)
{
	try
	{
		return handler->DataTask();
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerDataTask\n");
		return paramErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerScheduleData
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHScheduleDataSelect selector
// QuickTime uses that fucntion to read data, both synchronously or asyncronously
pascal ComponentResult  DataHandlerScheduleData (CDataHandlerConnection* handler, Ptr placeToPutData,
                                           long fileOffset, long dataSize, long refCon,
                                           DataHSchedulePtr scheduleRecPtr, DataHCompletionUPP completionRtn)
{
	try
	{
		return handler->ScheduleData(
			placeToPutData, 
			fileOffset, 
			dataSize, 
			refCon, 
			scheduleRecPtr, 
			completionRtn);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerScheduleData\n");
		return readErr;
	}
}


// ---------------------------------------------------------------------
//  DataHandlerFinishData
// ---------------------------------------------------------------------
// This function is called in response to QuickTime's kDataHFinishDataSelect selector
// QuickTime uses that fucntion both to speed up getting of data or remove previous request
pascal ComponentResult  DataHandlerFinishData (CDataHandlerConnection* handler, Ptr placeToPutData, Boolean cancel)
{
	try
	{
		return handler->FinishData(placeToPutData, cancel);
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerFinishData\n");
		return readErr;
	}
}

// ---------------------------------------------------------------------
//  DataHandlerFlushData
// ---------------------------------------------------------------------
// This function is called in response to kDataHFlushDataSelect selector
// Will stop downloading file from the stream if any
pascal ComponentResult  DataHandlerFlushData (CDataHandlerConnection* handler)
{
	try
	{
		return handler->FlushData();
	}
	catch (...)
	{
		UDebug::Message("!!! Exception in DataHandlerFlushData\n");
		return readErr;
	}
}


HL_End_Namespace_BigRedH
