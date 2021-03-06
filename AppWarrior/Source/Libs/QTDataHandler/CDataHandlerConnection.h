// =====================================================================
//  CDataHandlerConnection.h             (C) Hotline Communications 2000
// =====================================================================
// 
//  A class that wraps working with a connection to the datahandling component.
//  QuickTime has a pointer to "Storage" (private data) associated with every 
//  connection, and we use it as a pointer to object of CDataHandlerConnection
//
// =====================================================================

#ifndef _H_DATAHANDLERCONNECTION_
#define _H_DATAHANDLERCONNECTION_


//-----------------------------------------------------------------------
// Includes
#include <Components.h>
#include <QuickTimeComponents.h>
#include "CDataProvider.h"
#include "CMutex.h"

HL_Begin_Namespace_BigRedH


class CDataHandlerConnection
{
private:
	CDataProvider*		mProvider;
	CString				mURL;
public:
    
	CDataHandlerConnection();

	virtual ~CDataHandlerConnection();

    // save DataRef for future use
    ComponentResult SetDataRef(Handle inDataref);

    // get filename with an extention
    ComponentResult GetFileName(StringPtr outFileName);

	// get MIME type 
	ComponentResult GetMimeType(StringPtr outMimeType);

	// get File type 
	ComponentResult GetFileType(OSType *outFileType);

    // Get full size of the movie
    ComponentResult GetFullSize(long* outSize);

    // get available size of the movie
    ComponentResult GetAvailableSize(long* outSize);

    // spend time. We check here if more data is available
    ComponentResult DataTask();

    // open current dataref for read
    ComponentResult OpenForRead();

    // close current dataref
    ComponentResult CloseForRead();

    // read data sync ar async
	ComponentResult ScheduleData(
        Ptr inPlaceToPutData,
        UInt32 inFileOffset,
        UInt32 inDataSize,
        UInt32 inRefCon,
        DataHSchedulePtr inScheduleRec,
        DataHCompletionUPP inCompletionRtn);

	// finish reading data
	ComponentResult  FinishData (
		Ptr inPlaceToPutData, 
		Boolean inCancel);

	// stop downloading and close a TCP stream
	ComponentResult FlushData();

    // compare given dataref with current
    bool CompareDataRef(Handle inDataref);
};


HL_End_Namespace_BigRedH

#endif //_H_DATAHANDLERCONNECTION_
