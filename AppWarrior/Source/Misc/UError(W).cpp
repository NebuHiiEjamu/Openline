#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UError.h"

/* -------------------------------------------------------------------------- */

void UError::Init()
{

}

Uint32 UError::GetMessage(const SError& inError, void *outText, Uint32 inMaxSize)
{
	const Uint8 *kUnknownErrorMsg = "\pAn unknown error has occured.";
	const Uint8 *kNotEnoughMemMsg = "\pNot enough memory.  Try closing windows.";
	
	Uint32 s = 0;
	
	try
	{
		TRez rz = URez::SearchChain('EMSG', inError.type);
		if (rz)
		{
			THdl h;
			StRezLoader rzLoad(rz, 'EMSG', inError.type, h);
			
			{
				void *p;
				StHandleLocker lock(h, p);
				s = UIDVarArray::GetItem(p, h->GetSize(), inError.id, outText, inMaxSize);
			}
		}
	}
	catch(...) {}
	
	if (s == 0)
	{
		const Uint8 *tp;
		
		if (inError.type == errorType_Memory && inError.id == memError_NotEnough)
			tp = kNotEnoughMemMsg;
		else
			tp = kUnknownErrorMsg;

		s = tp[0];
		if (s > inMaxSize) s = inMaxSize;
		
		::CopyMemory(outText, tp+1, s);
	}

	return s;
}

void UError::Alert(const SError& inError)
{
	Int8 str[1024];
	Uint32 s;
	
	if (inError.silent) return;
	
	s = UError::GetMessage(inError, str, 255);
	
	str[s++] = '\r';
	str[s++] = '\r';

	s += UError::GetDetailMessage(inError, str + s, 128);
	
	str[s] = 0;
	
	::MessageBox(NULL, str, "ERROR", MB_TASKMODAL | MB_ICONWARNING);
}


/* -------------------------------------------------------------------------- */
#pragma mark -

struct SErrorEntry {
	Int32 win;
	Int16 type, id;
};

// entries MUST be sorted by the win error code, lowest to highest
static const SErrorEntry gErrorTable[] = {
	{ 2,		errorType_FileSys,		fsError_NoSuchFile				},
	{ 3,		errorType_FileSys,		fsError_NoSuchItem				},
	{ 5,		errorType_FileSys,		fsError_AccessDenied			},
	{ 8,		errorType_Memory,		memError_NotEnough				},
	{ 14,		errorType_Memory,		memError_NotEnough				},
	{ 15,		errorType_FileSys,		fsError_NoSuchDisk				},
	{ 17,		errorType_FileSys,		fsError_DifferentDisks			},
	{ 19,		errorType_FileSys,		fsError_ItemLocked				},
	{ 32,		errorType_FileSys,		fsError_FileInUse				},
	{ 50,		errorType_FileSys,		fsError_OperationNotSupported	},
	{ 52,		errorType_FileSys,		fsError_ItemAlreadyExists		},
	{ 80,		errorType_FileSys,		fsError_ItemAlreadyExists		},
	{ 108,		errorType_FileSys,		fsError_DiskLocked				},
	{ 112,		errorType_FileSys,		fsError_NotEnoughSpace			},
	{ 120,		errorType_FileSys,		fsError_OperationNotSupported	},
	{ 123,		errorType_FileSys,		fsError_BadName					},
	{ 161,		errorType_FileSys,		fsError_BadName					},
	{ 183,		errorType_FileSys,		fsError_ItemAlreadyExists		},
	{ 206,		errorType_FileSys,		fsError_BadName					},
	{ 1393,		errorType_FileSys,		fsError_DiskDamaged				},
	{ 1400,		errorType_Memory,		memError_BlockInvalid			},
	{ 1413,		errorType_Misc,			error_OutOfRange				},
	{ 10048,	errorType_Transport,	transError_AddressInUse			},
	{ 10049,	errorType_Transport,	transError_AddressInUse			},
	{ 10050,	errorType_Transport,	transError_HostUnreachable		},
	{ 10051,	errorType_Transport,	transError_HostUnreachable		},
	{ 10052,	errorType_Transport,	transError_HostUnreachable		},
	{ 10053,	errorType_Transport,	transError_Unknown				},
	{ 10054,	errorType_Transport,	transError_Unknown				},
	{ 10055,	errorType_Transport,	transError_Unknown				},
	{ 10056,	errorType_Transport,	transError_Unknown				},
	{ 10057,	errorType_Transport,	transError_Unknown				},
	{ 10058,	errorType_Transport,	transError_Unknown				},
	{ 10059,	errorType_Transport,	transError_Unknown				},
	{ 10060,	errorType_Transport,	transError_ConnectionTimedOut	},
	{ 10061,	errorType_Transport,	transError_ConnectionRefused	},
	{ 10062,	errorType_Transport,	transError_Unknown				},
	{ 10063,	errorType_Transport,	transError_Unknown				},
	{ 10064,	errorType_Transport,	transError_ConnectionTimedOut	},
	{ 10065,	errorType_Transport,	transError_HostUnreachable		},
	{ 11001,	errorType_Transport,	transError_BadAddress			},
	{ 11002,	errorType_Transport,	transError_BadAddress			}
};

void _WinToGenError(Int32 inWinError, Int16& outType, Int16& outID)
{
	Int32 lowBound	= 1;
	Int32 highBound	= sizeof(gErrorTable) / sizeof(SErrorEntry);
	Int32 i;
	const SErrorEntry *table = gErrorTable;
	const SErrorEntry *entry;
	
	do
	{
		i = (lowBound + highBound) >> 1;	// same as:  (lowBound + highBound) / 2
		entry = table + i - 1;
		
		if (inWinError == entry->win)
		{
			outType = entry->type;
			outID = entry->id;
			return;
		}
		else if (inWinError < entry->win)
			highBound = i - 1;
		else
			lowBound = i + 1;
	
	} while (lowBound <= highBound);
	
	outType = errorType_Misc;
	outID = error_Unknown;
}

void _FailLastWinError(const Int8 *inFile, Uint32 inLine)
{
	SError err;
	
	err.file = inFile;
	err.line = inLine;
	err.fatal = err.silent = false;
	err.special = ::GetLastError();
	
	_WinToGenError(err.special, err.type, err.id);
	
	throw err;
}

void _FailWinError(Int32 inWinError, const Int8 *inFile, Uint32 inLine)
{
	SError err;
	
	err.file = inFile;
	err.line = inLine;
	err.fatal = err.silent = false;
	err.special = inWinError;
	
	_WinToGenError(inWinError, err.type, err.id);
	
	throw err;
}



#endif
