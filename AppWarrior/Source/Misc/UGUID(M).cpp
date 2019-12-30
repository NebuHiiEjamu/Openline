#if MACINTOSH
// this should be cleaned up.  A MD5 algorythm should be added to UMemory
// (or maybe there should be a UDigest class with MD5, CRC, Adler, etc)

#if TARGET_API_MAC_CARBON
Int16 _GetSystemVersion();
#else
#include "UUIDLib.h"
#endif

OSErr GenerateUUID(Uint8 *outUUID);

// mac error handling
void _FailMacError(Int16 inMacError, const Int8 *inFile, Uint32 inLine);
inline void _CheckMacError(Int16 inMacError, const Int8 *inFile, Uint32 inLine) { if (inMacError) _FailMacError(inMacError, inFile, inLine); }
#if DEBUG
	#define FailMacError(id)		_FailMacError(id, __FILE__, __LINE__)
	#define CheckMacError(id)		_CheckMacError(id, __FILE__, __LINE__)
#else
	#define FailMacError(id)		_FailMacError(id, nil, 0)
	#define CheckMacError(id)		_CheckMacError(id, nil, 0)
#endif


void UGUID::Generate(SGUID &outGUID)
{
	CheckMacError(GenerateUUID((Uint8 *)(&outGUID)));
}

bool UGUID::IsEqual(const SGUID &inGUIDA, const SGUID &inGUIDB)
{
	return UMemory::Equal(&inGUIDA, &inGUIDB, sizeof(SGUID));
}

Uint32 UGUID::Flatten(const SGUID &inGUID, void *outData)
{
	*((SGUID *)outData) = inGUID;
	return sizeof(SGUID);
}

void UGUID::Unflatten(SGUID &outGUID, const void *inData)
{
	outGUID = *((SGUID *)inData);
}

bool SGUID::operator<(const SGUID &inGUID) const
{
	return UMemory::Compare(this, &inGUID, sizeof(SGUID)) == -1;
}

bool SGUID::operator>(const SGUID &inGUID) const
{
	return UMemory::Compare(this, &inGUID, sizeof(SGUID)) == 1;
}

bool SGUID::operator<=(const SGUID &inGUID) const
{
	return UMemory::Compare(this, &inGUID, sizeof(SGUID)) <= 0;
}

bool SGUID::operator>=(const SGUID &inGUID) const
{
	return UMemory::Compare(this, &inGUID, sizeof(SGUID)) >= 0;
}


#pragma mark -

#if TARGET_API_MAC_CARBON

OSErr GenerateUUID(Uint8 *outUUID)
{
	CFUUIDRef uuid = ::CFUUIDCreate(kCFAllocatorDefault);
	CFUUIDBytes uuidBytes = ::CFUUIDGetUUIDBytes(uuid);		// this works only under Mac OS X
	
	UMemory::Copy(outUUID, &uuidBytes, sizeof(CFUUIDBytes));
	
	// check if we are running under MacOS X, otherwise generate random values
	if (_GetSystemVersion() < 0x0A00)
	{
		Uint32 *pUUID = (Uint32 *)outUUID;
		*pUUID = UMath::GetRandom();
		*(pUUID + 1) = UMath::GetRandom();
		*(pUUID + 2) = UMath::GetRandom();
		// don't modify the last 4 bytes
	}

	return noErr;
}

bool GetEthernetAddr(SEthernetAddress& outEthernetAddr)
{
    InetInterfaceInfo stInetInterfaceInfo;
    if (::OTInetGetInterfaceInfo(&stInetInterfaceInfo, kDefaultInetInterface) == kOTNoError)
    {
    	if (stInetInterfaceInfo.fHWAddrLen >= sizeof(outEthernetAddr.eaddr))
    	{
    		UMemory::Copy(outEthernetAddr.eaddr, stInetInterfaceInfo.fHWAddr, sizeof(outEthernetAddr.eaddr));
			return true;
		}
 	}
	
	outEthernetAddr.SetNull();
	return false;
}

#else

/* GLOBAL.H - RSAREF types and constants
 */

/* PROTOTYPES should be set to one if and only if the compiler supports
   function argument prototyping.
   The following makes PROTOTYPES default to 0 if it has not already
   been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 1		// modified to show support
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

/************************************************************************/

/*	internal.h
 *
 *  Internal declarations and routines that are common
 *	across modules
 */

#ifndef PandaWave_INTERNAL
#define PandaWave_INTERNAL

/************************************************************************/
/*																		*/
/*	Various internal routines											*/
/*																		*/
/************************************************************************/

typedef struct
{
    unsigned char eaddr[6];      /* 6 bytes of ethernet hardware address */
} uuid_address_t;

typedef struct {
	unsigned long lo;
	unsigned long hi;
} uuid_time_t;

extern void memset(void *, char, long);
extern void memcpy(void *, const void *, long);

extern OSErr GetEthernetAddr(uuid_address_t *addr);
extern OSErr GenRandomEthernet(uuid_address_t *addr);

extern OSErr ReadPrefData(void);
extern void  WritePrefData(void);

/*
 *	Preferences file management
 */

extern uuid_address_t GSavedENetAddr;

extern uuid_time_t GLastTime;			/* Clock state info */
extern unsigned short GTimeAdjust;
extern unsigned short GClockSeq;

#endif // PandaWave_INTERNAL

/************************************************************************/

/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */
 
#ifndef PROTO_LIST
#include "global.h"
#endif

/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void MD5Init PROTO_LIST ((MD5_CTX *));
void MD5Update PROTO_LIST
  ((MD5_CTX *, unsigned char *, unsigned int));
void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));
void MD5Print PROTO_LIST ((char *, unsigned char [16]));

#ifdef __cplusplus
}
#endif


#pragma mark -

/*	GenRandomEthernet.c
 *
 *		This generates a random address when an ethernet card is
 *	not present.
 */

//#include "internal.h"
//#include "md5.h"

/************************************************************************/
/*																		*/
/*	Internal Structures													*/
/*																		*/
/************************************************************************/

/*	RandomMacStuff
 *
 *		Random macintosh factors
 */

struct RandomMacStuff {
	UnsignedWide		curprocessor;		/* How long ago turned on? */
	unsigned long		curtime;			/* Current clock time */
	short				vBootVol;
	long				vSysDirID;
	long				vCreateDate;		/* Boot volume attributes */
	long				vLastBackup;
	short				vAttrib;
	short				vFileCount;
	short				vDirStart;
	short				vDirLength;
	short				vAllocBlocks;
	long				vAllocSize;
	long				vClumpSize;
	short				vBlockMap;
	long				vNextFile;
	short				vFreeBlocks;
	Point				mousePos;			/* Current mouse pointer pos */
};


/************************************************************************/
/*																		*/
/*	Generation Routine													*/
/*																		*/
/************************************************************************/

/*	GenRandomEthernet
 *
 *		This generates a random 48-bit string using some random factors
 *	from the Macintosh operating system, using the MD5 messaging
 *	algorithm to generate the bits.
 */

OSErr GenRandomEthernet(uuid_address_t *addr)
{
	struct RandomMacStuff stuff;
	ParamBlockRec param;
	MD5_CTX context;
	unsigned char md5res[16];
	int i;
	
	/*
	 *	Did I save a random ethernet value already? If so,
	 *	return that instead. (The illegal value is all zeros;
	 *	this means that we didn't load preferences data.)
	 */
	
	for (i = 0; i < 6; i++) {
		if (GSavedENetAddr.eaddr[i]) break;
	}
	if (i < 6) {
		*addr = GSavedENetAddr;
		return noErr;
	}

	/*
	 *	Unable to open or load the pregenerated UUID from the
	 *	preferences file. This generates one randomly and writes
	 *	it to the preferences file.
	 */
	
	GetDateTime(&stuff.curtime);
	Microseconds(&stuff.curprocessor);
	
	/*
	 *	Get mouse state
	 */
	
	GetMouse(&stuff.mousePos);
	
	/*
	 *	Now get the boot volumn
	 */
	
	FindFolder(kOnSystemDisk,
			   kSystemFolderType,
			   kDontCreateFolder,
			   &stuff.vBootVol,
			   &stuff.vSysDirID);
	
	/*
	 *	And get HD parameters
	 */
	
	param.volumeParam.ioVolIndex = 0;
	param.volumeParam.ioNamePtr = NULL;
	param.volumeParam.ioVRefNum = stuff.vBootVol;
	if (0 == PBGetVInfo(&param,false)) {
		stuff.vCreateDate  = param.volumeParam.ioVCrDate;
		stuff.vLastBackup  = param.volumeParam.ioVLsBkUp;
		stuff.vAttrib      = param.volumeParam.ioVAtrb;
		stuff.vFileCount   = param.volumeParam.ioVNmFls;
		stuff.vDirStart    = param.volumeParam.ioVDirSt;
		stuff.vDirLength   = param.volumeParam.ioVBlLn;
		stuff.vAllocBlocks = param.volumeParam.ioVNmAlBlks;
		stuff.vAllocSize   = param.volumeParam.ioVAlBlkSiz;
		stuff.vClumpSize   = param.volumeParam.ioVClpSiz;
		stuff.vBlockMap    = param.volumeParam.ioAlBlSt;
		stuff.vNextFile    = param.volumeParam.ioVNxtFNum;
		stuff.vFreeBlocks  = param.volumeParam.ioVFrBlk;
	}
	
	
	/*
	 *	Now activate the MD5 algorithm with these parameters
	 */
	
	MD5Init(&context);
	MD5Update(&context,(unsigned char *)&stuff,sizeof(stuff));
	MD5Final(md5res,&context);
	
	/*
	 *	And copy over the bits
	 */
	
	for (i = 0; i < 6; i++) addr->eaddr[i] = md5res[i+5];
	addr->eaddr[0] |= 0x80;					/* Set high bit of this thing */
	
	/*
	 *	Now save this for writing out later
	 */
	
	GSavedENetAddr = *addr;
	return noErr;
}


#pragma mark -


/*	uuid.c
 *
 *		Modifications made by William Woody to make this thing
 *	work on the Macintosh.
 */

/*
 * 
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *                 permission to use, copy, modify, and distribute this
 * file for any purpose is hereby granted without fee, provided that
 * the above copyright notices and this notice appears in all source
 * code copies, and that none of the names of Open Software
 * Foundation, Inc., Hewlett-Packard Company, or Digital Equipment
 * Corporation be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Neither Open Software Foundation, Inc., Hewlett-
 * Packard Company, nor Digital Equipment Corporation makes any
 * representations about the suitability of this software for any
 * purpose.
 * 
 */
/*
 */
/*
**
**  NAME:
**
**      uuid.c
**
**  FACILITY:
**
**      UUID
**
**  ABSTRACT:
**
**      UUID - routines that manipulate uuid's
**
**
*/

//#include <UUIDLib.h>
//#include "internal.h"

/*
 * Internal structure of universal unique IDs (UUIDs).
 *
 * There are three "variants" of UUIDs that this code knows about.  The
 * variant #0 is what was defined in the 1989 HP/Apollo Network Computing
 * Architecture (NCA) specification and implemented in NCS 1.x and DECrpc
 * v1.  Variant #1 is what was defined for the joint HP/DEC specification
 * for the OSF (in DEC's "UID Architecture Functional Specification Version
 * X1.0.4") and implemented in NCS 2.0, DECrpc v2, and OSF 1.0 DCE RPC.
 * Variant #2 is defined by Microsoft.
 *
 * This code creates only variant #1 UUIDs.
 * 
 * The three UUID variants can exist on the same wire because they have
 * distinct values in the 3 MSB bits of octet 8 (see table below).  Do
 * NOT confuse the version number with these 3 bits.  (Note the distinct
 * use of the terms "version" and "variant".) Variant #0 had no version
 * field in it.  Changes to variant #1 (should any ever need to be made)
 * can be accomodated using the current form's 4 bit version field.
 * 
 * The UUID record structure MUST NOT contain padding between fields.
 * The total size = 128 bits.
 *
 * To minimize confusion about bit assignment within octets, the UUID
 * record definition is defined only in terms of fields that are integral
 * numbers of octets.
 *
 * Depending on the network data representation, the multi-octet unsigned
 * integer fields are subject to byte swapping when communicated between
 * dissimilar endian machines.  Note that all three UUID variants have
 * the same record structure; this allows this byte swapping to occur.
 * (The ways in which the contents of the fields are generated can and
 * do vary.)
 *
 * The following information applies to variant #1 UUIDs:
 *
 * The lowest addressed octet contains the global/local bit and the
 * unicast/multicast bit, and is the first octet of the address transmitted
 * on an 802.3 LAN.
 *
 * The adjusted time stamp is split into three fields, and the clockSeq
 * is split into two fields.
 *
 * |<------------------------- 32 bits -------------------------->|
 *
 * +--------------------------------------------------------------+
 * |                     low 32 bits of time                      |  0-3  .time_low
 * +-------------------------------+-------------------------------
 * |     mid 16 bits of time       |  4-5               .time_mid
 * +-------+-----------------------+
 * | vers. |   hi 12 bits of time  |  6-7               .time_hi_and_version
 * +-------+-------+---------------+
 * |Res|  clkSeqHi |  8                                 .clock_seq_hi_and_reserved
 * +---------------+
 * |   clkSeqLow   |  9                                 .clock_seq_low
 * +---------------+----------...-----+
 * |            node ID               |  8-16           .node
 * +--------------------------...-----+
 *
 * --------------------------------------------------------------------------
 *
 * The structure layout of all three UUID variants is fixed for all time.
 * I.e., the layout consists of a 32 bit int, 2 16 bit ints, and 8 8
 * bit ints.  The current form version field does NOT determine/affect
 * the layout.  This enables us to do certain operations safely on the
 * variants of UUIDs without regard to variant; this increases the utility
 * of this code even as the version number changes (i.e., this code does
 * NOT need to check the version field).
 *
 * The "Res" field in the octet #8 is the so-called "reserved" bit-field
 * and determines whether or not the uuid is a old, current or other
 * UUID as follows:
 *
 *      MS-bit  2MS-bit  3MS-bit      Variant
 *      ---------------------------------------------
 *         0       x        x       0 (NCS 1.5)
 *         1       0        x       1 (DCE 1.0 RPC)
 *         1       1        0       2 (Microsoft)
 *         1       1        1       unspecified
 *
 * --------------------------------------------------------------------------
 *
 * Internal structure of variant #0 UUIDs
 *
 * The first 6 octets are the number of 4 usec units of time that have
 * passed since 1/1/80 0000 GMT.  The next 2 octets are reserved for
 * future use.  The next octet is an address family.  The next 7 octets
 * are a host ID in the form allowed by the specified address family.
 *
 * Note that while the family field (octet 8) was originally conceived
 * of as being able to hold values in the range [0..255], only [0..13]
 * were ever used.  Thus, the 2 MSB of this field are always 0 and are
 * used to distinguish old and current UUID forms.
 *
 * +--------------------------------------------------------------+
 * |                    high 32 bits of time                      |  0-3  .time_high
 * +-------------------------------+-------------------------------
 * |     low 16 bits of time       |  4-5               .time_low
 * +-------+-----------------------+
 * |         reserved              |  6-7               .reserved
 * +---------------+---------------+
 * |    family     |   8                                .family
 * +---------------+----------...-----+
 * |            node ID               |  9-16           .node
 * +--------------------------...-----+
 *
 */

/***************************************************************************
 *
 * Local definitions
 *
 **************************************************************************/

const long      uuid_c_version          = 1;

/*
 * local defines used in uuid bit-diddling
 */
#define HI_WORD(w)                  ((w) >> 16)
#define RAND_MASK                   0x3fff      /* same as CLOCK_SEQ_LAST */

#define TIME_MID_MASK               0x0000ffff
#define TIME_HIGH_MASK              0x0fff0000
#define TIME_HIGH_SHIFT_COUNT       16

/*
 *	The following was modified in order to prevent overlap because
 *	our clock is (theoretically) accurate to 1 Microsecond.
 */

#define MAX_TIME_ADJUST             9			/* Max adjust before tick */

#define CLOCK_SEQ_LOW_MASK          0xff
#define CLOCK_SEQ_HIGH_MASK         0x3f00
#define CLOCK_SEQ_HIGH_SHIFT_COUNT  8
#define CLOCK_SEQ_FIRST             1
#define CLOCK_SEQ_LAST              0x3fff      /* same as RAND_MASK */

/*
 * Note: If CLOCK_SEQ_BIT_BANG == TRUE, then we can avoid the modulo
 * operation.  This should save us a divide instruction and speed
 * things up.
 */

#ifndef CLOCK_SEQ_BIT_BANG
#define CLOCK_SEQ_BIT_BANG          1
#endif

#if CLOCK_SEQ_BIT_BANG
#define CLOCK_SEQ_BUMP(seq)         ((*seq) = ((*seq) + 1) & CLOCK_SEQ_LAST)
#else
#define CLOCK_SEQ_BUMP(seq)         ((*seq) = ((*seq) + 1) % (CLOCK_SEQ_LAST+1))
#endif

#define UUID_VERSION_BITS           (uuid_c_version << 12)
#define UUID_RESERVED_BITS          0x80

#define IS_OLD_UUID(uuid) (((uuid)->clock_seq_hi_and_reserved & 0xc0) != 0x80)


/************************************************************************/
/*																		*/
/*	New Routines														*/
/*																		*/
/************************************************************************/
#if IDONTNEEDTHIS
static void memcpy(void *d, const void *s, long len)
{
	char *dest = (char *)d;
	const char *src = (const char *)s;
	while (len-- > 0) *dest++ = *src++;
}
#endif
/*
 * saved copy of our IEEE 802 address for quick reference
 */

static uuid_address_t saved_addr;
static int got_address = FALSE;
static int last_addr_result = FALSE;


/*
**++
**
**  ROUTINE NAME:       uuid_get_address
**
**  SCOPE:              PUBLIC
**
**  DESCRIPTION:
**
**  Return our IEEE 802 address.
**
**  This function is not really "public", but more like the SPI functions
**  -- available but not part of the official API.  We've done this so
**  that other subsystems (of which there are hopefully few or none)
**  that need the IEEE 802 address can use this function rather than
**  duplicating the gore it does (or more specifically, the gore that
**  "uuid__get_os_address" does).
**
**  INPUTS:             none
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      addr            IEEE 802 address
**
**      status          return status value
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       none
**
**--
**/

static int uuid_get_address(uuid_address_t *addr)
{
	extern OSErr GetEthernetAddr(uuid_address_t *);
	
    /*
     * just return address we determined previously if we've
     * already got one
     */
    
    if (got_address) {
        memcpy (addr, &saved_addr, sizeof (uuid_address_t));
        return last_addr_result;
    }

    /*
     * Otherwise, call the system specific routine.
     */
     
    last_addr_result = GetEthernetAddr(addr);
    
    /*
     *	Was this an error? If so, I need to generate a random
     *	sequence to use in place of an Ethernet address.
     */
    
    if (last_addr_result) {
    	last_addr_result = GenRandomEthernet(addr);
    }
    
    got_address = TRUE;
    if (last_addr_result == 0) {
    	/* On no error copy */
        memcpy (&saved_addr, addr, sizeof (uuid_address_t));
	}
	return last_addr_result;
}

/****************************************************************************
 *
 * local data declarations
 *
 ****************************************************************************/

typedef struct {
	unsigned long lo;
	unsigned long hi;
} unsigned64_t;

/*
 * declarations used in UTC time calculations
 */
 
static uuid_time_t          time_now;     /* utc time as of last query        */
//static uuid_time_t          time_last;    /* utc time last time I looked      */
//static unsigned short       time_adjust;  /* 'adjustment' to ensure uniqness  */
//static unsigned short       clock_seq;    /* 'adjustment' for backwards clocks*/

/*
 * true_random variables
 */

static unsigned long     rand_m;         /* multiplier                       */
static unsigned long     rand_ia;        /* adder #1                         */
static unsigned long     rand_ib;        /* adder #2                         */
static unsigned long     rand_irand;     /* random value                     */

typedef enum
{
    uuid_e_less_than, uuid_e_equal_to, uuid_e_greater_than
} uuid_compval_t;

/****************************************************************************
 *
 * local function declarations
 *
 ****************************************************************************/

/*
 * I N I T
 *
 * Startup initialization routine for UUID module.
 */

static OSErr init (void);

/*
 * T R U E _ R A N D O M _ I N I T
 */

static void true_random_init (void);

/*
 * T R U E _ R A N D O M
 */
static unsigned short true_random (void);


/*
 * N E W _ C L O C K _ S E Q
 *
 * Ensure clock_seq is up-to-date
 *
 * Note: clock_seq is architected to be 14-bits (unsigned) but
 *       I've put it in here as 16-bits since there isn't a
 *       14-bit unsigned integer type (yet)
 */ 
static void new_clock_seq ( unsigned short * /*clock_seq*/);

/*
 * S T R U C T U R E _ I S _ K N O W N
 *
 * Does the UUID have the known standard structure layout?
 */
 
int structure_is_known ( uuid_t * /*uuid*/);

/*
 * T I M E _ C M P
 *
 * Compares two UUID times (64-bit DEC UID UTC values)
 */
static uuid_compval_t time_cmp (
        uuid_time_t *        /*time1*/,
        uuid_time_t *        /*time2*/
    );


/*****************************************************************************
 *
 *  Macro definitions
 *
 ****************************************************************************/

/*
 * ensure we've been initialized
 */
static int uuid_init_done = FALSE;

#define EmptyArg
#define UUID_VERIFY_INIT(Arg)          \
    if (! uuid_init_done)           \
    {                               \
        init (status);              \
        if (*status != uuid_s_ok)   \
        {                           \
            return Arg;                 \
        }                           \
    }

/*
 * Check the reserved bits to make sure the UUID is of the known structure.
 */

#define CHECK_STRUCTURE(uuid) \
( \
    (((uuid)->clock_seq_hi_and_reserved & 0x80) == 0x00) || /* var #0 */ \
    (((uuid)->clock_seq_hi_and_reserved & 0xc0) == 0x80) || /* var #1 */ \
    (((uuid)->clock_seq_hi_and_reserved & 0xe0) == 0xc0)    /* var #2 */ \
)

/*
 * The following macros invoke CHECK_STRUCTURE(), check that the return
 * value is okay and if not, they set the status variable appropriately
 * and return either a boolean FALSE, nothing (for void procedures),
 * or a value passed to the macro.  This has been done so that checking
 * can be done more simply and values are returned where appropriate
 * to keep compilers happy.
 *
 * bCHECK_STRUCTURE - returns boolean FALSE
 * vCHECK_STRUCTURE - returns nothing (void)
 * rCHECK_STRUCTURE - returns 'r' macro parameter
 */

#define bCHECK_STRUCTURE(uuid, status) \
{ \
    if (!CHECK_STRUCTURE (uuid)) \
    { \
        *(status) = uuid_s_bad_version; \
        return (FALSE); \
    } \
}

#define vCHECK_STRUCTURE(uuid, status) \
{ \
    if (!CHECK_STRUCTURE (uuid)) \
    { \
        *(status) = uuid_s_bad_version; \
        return; \
    } \
}

#define rCHECK_STRUCTURE(uuid, status, result) \
{ \
    if (!CHECK_STRUCTURE (uuid)) \
    { \
        *(status) = uuid_s_bad_version; \
        return (result); \
    } \
}


/*
 *  Define constant designation difference in Unix and DTSS base times:
 *  DTSS UTC base time is October 15, 1582.
 *  Unix base time is January 1, 1970.
 */
#define uuid_c_os_base_time_diff_lo     0x13814000
#define uuid_c_os_base_time_diff_hi     0x01B21DD2

#ifndef UUID_C_100NS_PER_SEC
#define UUID_C_100NS_PER_SEC            10000000
#endif

#ifndef UUID_C_100NS_PER_USEC
#define UUID_C_100NS_PER_USEC           10
#endif

static int got_first_time = 0;
static unsigned64_t read_micro;


/*
 * UADD_UVLW_2_UVLW - macro to add two unsigned 64-bit long integers
 *                      (ie. add two unsigned 'very' long words)
 *
 * Important note: It is important that this macro accommodate (and it does)
 *                 invocations where one of the addends is also the sum.
 *
 * This macro was snarfed from the DTSS group and was originally:
 *
 * UTCadd - macro to add two UTC times
 *
 * add lo and high order longword separately, using sign bits of the low-order
 * longwords to determine carry.  sign bits are tested before addition in two
 * cases - where sign bits match. when the addend sign bits differ the sign of
 * the result is also tested:
 *
 *        sign            sign
 *      addend 1        addend 2        carry?
 *
 *          1               1            TRUE
 *          1               0            TRUE if sign of sum clear
 *          0               1            TRUE if sign of sum clear
 *          0               0            FALSE
 */
#define UADD_UVLW_2_UVLW(add1, add2, sum)                               \
    if (!(((add1)->lo&0x80000000UL) ^ ((add2)->lo&0x80000000UL)))           \
    {                                                                   \
        if (((add1)->lo&0x80000000UL))                                    \
        {                                                               \
            (sum)->lo = (add1)->lo + (add2)->lo ;                       \
            (sum)->hi = (add1)->hi + (add2)->hi+1 ;                     \
        }                                                               \
        else                                                            \
        {                                                               \
            (sum)->lo  = (add1)->lo + (add2)->lo ;                      \
            (sum)->hi = (add1)->hi + (add2)->hi ;                       \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        (sum)->lo = (add1)->lo + (add2)->lo ;                           \
        (sum)->hi = (add1)->hi + (add2)->hi ;                           \
        if (!((sum)->lo&0x80000000UL))                                    \
            (sum)->hi++ ;                                               \
    }

/*
 * UADD_UW_2_UVLW - macro to add a 16-bit unsigned integer to
 *                   a 64-bit unsigned integer
 *
 * Note: see the UADD_UVLW_2_UVLW() macro
 *
 */
#define UADD_UW_2_UVLW(add1, add2, sum)                                 \
{                                                                       \
    (sum)->hi = (add2)->hi;                                             \
    if ((add2)->lo & 0x80000000UL)                                        \
    {                                                                   \
        (sum)->lo = (*add1) + (add2)->lo;                               \
        if (!((sum)->lo & 0x80000000UL))                                  \
        {                                                               \
            (sum)->hi++;                                                \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        (sum)->lo = (*add1) + (add2)->lo;                               \
    }                                                                   \
}

/*
 * U U I D _ _ G E T _ O S _ T I M E
 *
 * Get OS time - contains platform-specific code.
 */

static void uuid__get_os_time (uuid_time_t * uuid_time)
{
    void uuid__uemul(unsigned long u, unsigned long v, unsigned64_t *prodPtr);

	unsigned long		read_time;
	unsigned64_t		tmp;
	int carry;
	UnsignedWide		micro;
    unsigned64_t        usecs,
 						utc,
                        os_basetime_diff;
    MachineLocation     os_mach;
    long                delta_gmt;

	/*
	 *	Calculate the time in 100ns units since system boot
	 */
	
	if (!got_first_time) {
		got_first_time = 1;
		GetDateTime(&read_time);			// time in seconds
		
		ReadLocation(&os_mach);				// offset time to GMT
		delta_gmt = os_mach.u.gmtDelta;
		delta_gmt &= 0x00FFFFFF;
		if (delta_gmt & 0x00800000) delta_gmt |= 0xFF000000;
		read_time -= delta_gmt;
		
		Microseconds(&micro);				// time in 100ns units
		micro.hi *= 10;
		uuid__uemul(micro.lo,10,&tmp);
		micro.lo  = tmp.lo;
		micro.hi += tmp.hi;
		
		read_time -= 2082844800;			/* seconds from 1904 to 1970 */
		uuid__uemul(read_time, UUID_C_100NS_PER_SEC, &read_micro);
		
		carry = (micro.lo > read_micro.lo) ? 1 : 0;
		read_micro.lo -= micro.lo;
		read_micro.hi -= micro.hi;
		if (carry) read_micro.hi--;
	}
	
	/*
	 *	Now get the current time in microseconds and add to boot time
	 */
	
	Microseconds(&micro);
	usecs.hi = micro.hi * 10;
	uuid__uemul(micro.lo,10,&tmp);
	usecs.lo = tmp.lo;
	usecs.hi = usecs.hi + tmp.hi;
	
	UADD_UVLW_2_UVLW(&usecs,&read_micro,&utc);

    /*
     * Offset between DTSS formatted times and Unix formatted times.
     */
    os_basetime_diff.lo = uuid_c_os_base_time_diff_lo;
    os_basetime_diff.hi = uuid_c_os_base_time_diff_hi;
    UADD_UVLW_2_UVLW (&utc, &os_basetime_diff, uuid_time);

}

/*
**++
**
**  ROUTINE NAME:       init
**
**  SCOPE:              INTERNAL - declared locally
**
**  DESCRIPTION:
**
**  Startup initialization routine for the UUID module.
**
**  INPUTS:             none
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      status          return status value
**
**          uuid_s_ok
**          uuid_s_coding_error
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     void
**
**  SIDE EFFECTS:       sets uuid_init_done so this won't be done again
**
**--
**/

static OSErr init()
{
    /*
     * init the random number generator
     */
    
    true_random_init();

	/*
	 *	Read the preferences data from the Macintosh pref file
	 */
	
	ReadPrefData();

	/*
	 *	Get the time. Note that I renamed 'time_last' to
	 *	GLastTime to indicate that I'm using it elsewhere as
	 *	a shared library global.
	 */
		
    if ((GLastTime.hi == 0) && (GLastTime.lo == 0)) {
	    uuid__get_os_time (&GLastTime);
	    GClockSeq = true_random();
	}
    uuid_init_done = TRUE;
    return 0;
}

/*
** New name: GenerateUID
** 
**++
**
**  ROUTINE NAME:       uuid_create
**
**  SCOPE:              PUBLIC - declared in UUID.IDL
**
**  DESCRIPTION:
**
**  Create a new UUID. Note: we only know how to create the new
**  and improved UUIDs.
**
**  INPUTS:             none
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      uuid            A new UUID value
**
**      status          return status value
**
**          uuid_s_ok
**          uuid_s_coding_error
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     void
**
**  SIDE EFFECTS:       none
**
**--
**/

/*
PUBLIC void uuid_create
#ifdef _DCE_PROTO_
(
    uuid_t                  *uuid,
    unsigned long              *status
)
#else
(uuid, status)
uuid_t                  *uuid;
unsigned long              *status;
#endif
*/

OSErr GenerateUUID(uuid_t *uuid)
{
	OSErr					err;
	uuid_address_t			eaddr;
    int               got_no_time = FALSE;

	if (!uuid_init_done) {
		err = init();
		if (err) return err;
	}
    /*
     * get our hardware network address
     */
     
	if (0 != (err = uuid_get_address(&eaddr))) return err;

    do
    {
        /*
         * get the current time
         */
        uuid__get_os_time (&time_now);

        /*
         * do stuff like:
         *
         *  o check that our clock hasn't gone backwards and handle it
         *    accordingly with clock_seq
         *  o check that we're not generating uuid's faster than we
         *    can accommodate with our time_adjust fudge factor
         */
        switch (time_cmp (&time_now, &GLastTime))
        {
            case uuid_e_less_than:
                new_clock_seq (&GClockSeq);
                GTimeAdjust = 0;
                break;
            case uuid_e_greater_than:
                GTimeAdjust = 0;
                break;
            case uuid_e_equal_to:
                if (GTimeAdjust == MAX_TIME_ADJUST)
                {
                    /*
                     * spin your wheels while we wait for the clock to tick
                     */
                    got_no_time = TRUE;
                }
                else
                {
                    GTimeAdjust++;
                }
                break;
            default:
            	return kUUIDInternalError;
        }
    } while (got_no_time);

    GLastTime.lo = time_now.lo;
    GLastTime.hi = time_now.hi;

    if (GTimeAdjust != 0)
    {
        UADD_UW_2_UVLW (&GTimeAdjust, &time_now, &time_now);
    }

    /*
     * now construct a uuid with the information we've gathered
     * plus a few constants
     */
    uuid->time_low = time_now.lo;
    uuid->time_mid = time_now.hi & TIME_MID_MASK;

    uuid->time_hi_and_version = (time_now.hi & TIME_HIGH_MASK) >> TIME_HIGH_SHIFT_COUNT;
    uuid->time_hi_and_version |= UUID_VERSION_BITS;

    uuid->clock_seq_low = GClockSeq & CLOCK_SEQ_LOW_MASK;
    uuid->clock_seq_hi_and_reserved = (GClockSeq & CLOCK_SEQ_HIGH_MASK) >> CLOCK_SEQ_HIGH_SHIFT_COUNT;
    uuid->clock_seq_hi_and_reserved |= UUID_RESERVED_BITS;

    memcpy (uuid->node, &eaddr, sizeof (uuid_address_t));

	return 0;
}

OSErr GenerateUUID(Uint8 *outUUID)
{
	return GenerateUUID((uuid_t *)outUUID);
}

/*****************************************************************************
 *
 *  LOCAL MATH PROCEDURES - math procedures used internally by the UUID module
 *
 ****************************************************************************/

/*
** T I M E _ C M P
**
** Compares two UUID times (64-bit UTC values)
**/

static uuid_compval_t time_cmp(uuid_time_t *time1,uuid_time_t *time2)
{
    /*
     * first check the hi parts
     */
    if (time1->hi < time2->hi) return (uuid_e_less_than);
    if (time1->hi > time2->hi) return (uuid_e_greater_than);

    /*
     * hi parts are equal, check the lo parts
     */
    if (time1->lo < time2->lo) return (uuid_e_less_than);
    if (time1->lo > time2->lo) return (uuid_e_greater_than);

    return (uuid_e_equal_to);
}

/*
**  U U I D _ _ U E M U L
**
**  Functional Description:
**        32-bit unsigned quantity * 32-bit unsigned quantity
**        producing 64-bit unsigned result. This routine assumes
**        long's contain at least 32 bits. It makes no assumptions
**        about byte orderings.
**
**  Inputs:
**
**        u, v       Are the numbers to be multiplied passed by value
**
**  Outputs:
**
**        prodPtr    is a pointer to the 64-bit result
**
**  Note:
**        This algorithm is taken from: "The Art of Computer
**        Programming", by Donald E. Knuth. Vol 2. Section 4.3.1
**        Pages: 253-255.
**--
**/

static void uuid__uemul(unsigned long u, unsigned long v, unsigned64_t *prodPtr)
{
    /*
     * following the notation in Knuth, Vol. 2
     */
    unsigned long      uuid1, uuid2, v1, v2, temp;


    uuid1 = u >> 16;
    uuid2 = u & 0xffff;
    v1 = v >> 16;
    v2 = v & 0xffff;

    temp = uuid2 * v2;
    prodPtr->lo = temp & 0xffff;
    temp = uuid1 * v2 + (temp >> 16);
    prodPtr->hi = temp >> 16;
    temp = uuid2 * v1 + (temp & 0xffff);
    prodPtr->lo += (temp & 0xffff) << 16;
    prodPtr->hi += uuid1 * v1 + (temp >> 16);
}

/****************************************************************************
**
**    U U I D   T R U E   R A N D O M   N U M B E R   G E N E R A T O R
**
*****************************************************************************
**
** This random number generator (RNG) was found in the ALGORITHMS Notesfile.
**
** (Note 16.7, July 7, 1989 by Robert (RDVAX::)Gries, Cambridge Research Lab,
**  Computational Quality Group)
**
** It is really a "Multiple Prime Random Number Generator" (MPRNG) and is
** completely discussed in reference #1 (see below).
**
**   References:
**   1) "The Multiple Prime Random Number Generator" by Alexander Hass
**      pp. 368 to 381 in ACM Transactions on Mathematical Software,
**      December, 1987
**   2) "The Art of Computer Programming: Seminumerical Algorithms
**      (vol 2)" by Donald E. Knuth, pp. 39 to 113.
**
** A summary of the notesfile entry follows:
**
** Gries discusses the two RNG's available for ULTRIX-C.  The default RNG
** uses a Linear Congruential Method (very popular) and the second RNG uses
** a technique known as a linear feedback shift register.
**
** The first (default) RNG suffers from bit-cycles (patterns/repetition),
** ie. it's "not that random."
**
** While the second RNG passes all the emperical tests, there are "states"
** that become "stable", albeit contrived.
**
** Gries then presents the MPRNG and says that it passes all emperical
** tests listed in reference #2.  In addition, the number of calls to the
** MPRNG before a sequence of bit position repeats appears to have a normal
** distribution.
**
** Note (mbs): I have coded the Gries's MPRNG with the same constants that
** he used in his paper.  I have no way of knowing whether they are "ideal"
** for the range of numbers we are dealing with.
**
****************************************************************************/

/*
** T R U E _ R A N D O M _ I N I T
**
** Note: we "seed" the RNG with the bits from the clock and the PID
**
**/

static void true_random_init (void)
{
    uuid_time_t         t;
    unsigned short          *seedp, seed=0;


    /*
     * optimal/recommended starting values according to the reference
     */
    static unsigned long   rand_m_init     = 971;
    static unsigned long   rand_ia_init    = 11113;
    static unsigned long   rand_ib_init    = 104322;
    static unsigned long   rand_irand_init = 4181;

    rand_m = rand_m_init;
    rand_ia = rand_ia_init;
    rand_ib = rand_ib_init;
    rand_irand = rand_irand_init;

    /*
     * Generating our 'seed' value
     *
     * We start with the current time, but, since the resolution of clocks is
     * system hardware dependent (eg. Ultrix is 10 msec.) and most likely
     * coarser than our resolution (10 usec) we 'mixup' the bits by xor'ing
     * all the bits together.  This will have the effect of involving all of
     * the bits in the determination of the seed value while remaining system
     * independent.  Then for good measure to ensure a unique seed when there
     * are multiple processes creating UUID's on a system, we add in the PID.
     */
    uuid__get_os_time(&t);
    seedp = (unsigned short *)(&t);
    seed ^= *seedp++;
    seed ^= *seedp++;
    seed ^= *seedp++;
    seed ^= *seedp++;
    rand_irand += seed;
}

/*
** T R U E _ R A N D O M
**
** Note: we return a value which is 'tuned' to our purposes.  Anyone
** using this routine should modify the return value accordingly.
**/

static unsigned short true_random (void)
{
    rand_m += 7;
    rand_ia += 1907;
    rand_ib += 73939;

    if (rand_m >= 9973) rand_m -= 9871;
    if (rand_ia >= 99991) rand_ia -= 89989;
    if (rand_ib >= 224729) rand_ib -= 96233;

    rand_irand = (rand_irand * rand_m) + rand_ia + rand_ib;

    return (HI_WORD (rand_irand) ^ (rand_irand & RAND_MASK));
}

/*****************************************************************************
 *
 *  LOCAL PROCEDURES - procedures used staticly by the UUID module
 *
 ****************************************************************************/

/*
** N E W _ C L O C K _ S E Q
**
** Ensure *clkseq is up-to-date
**
** Note: clock_seq is architected to be 14-bits (unsigned) but
**       I've put it in here as 16-bits since there isn't a
**       14-bit unsigned integer type (yet)
**/

static void new_clock_seq 
#ifdef _DCE_PROTO_
(
    unsigned short              *clkseq
)
#else
(Uint16 *clkseq)
#endif
{
    /*
     * A clkseq value of 0 indicates that it hasn't been initialized.
     */
    if (*clkseq == 0)
    {
#ifdef UUID_NONVOLATILE_CLOCK
        *clkseq = uuid__read_clock();           /* read nonvolatile clock */
        if (*clkseq == 0)                       /* still not init'd ???   */
        {
            *clkseq = true_random();      /* yes, set random        */
        }
#else
        /*
         * with a volatile clock, we always init to a random number
         */
        *clkseq = true_random();
#endif
    }

    CLOCK_SEQ_BUMP (clkseq);
    if (*clkseq == 0)
    {
        *clkseq = *clkseq + 1;
    }

#ifdef UUID_NONVOLATILE_CLOCK
    uuid_write_clock (clkseq);
#endif
}



#pragma mark -


/*	SetGlobalTime.c
 *
 *		Get/set the last time value that was retrieved; this gets and
 *	sets the last time value that was used for uuid_t generation from
 *	the system clock
 */

//#include "internal.h"

/*	Globals
 *
 *		Preferences data ultimately go here
 */

uuid_address_t GSavedENetAddr;
uuid_time_t GLastTime;
unsigned short GTimeAdjust;
unsigned short GClockSeq;


/*	ReadPrefData
 *
 *		Read the preferences data into my global variables
 */

OSErr ReadPrefData(void)
{
	short refNum;
	FSSpec file;
	long len;
	OSErr err;

	/*
	 *	Zero out the saved preferences information
	 */
	
	memset((void *)&GSavedENetAddr,0,sizeof(GSavedENetAddr));
	memset((void *)&GLastTime,0,sizeof(GLastTime));
	GTimeAdjust = 0;
	GClockSeq = 0;

	/*
	 *	Find the preferences file name
	 */
	
	FindFolder(kOnSystemDisk,
			   kPreferencesFolderType,
			   kCreateFolder,
			   &file.vRefNum,
			   &file.parID);
	BlockMove((Ptr)"\pUUIDLib Preferences",(Ptr)file.name,20);
	
	/*
	 *	Now read the data
	 */
	
	if (0 == (err = FSpOpenDF(&file,fsRdPerm,&refNum))) {
		/*
		 *	Read the data as 6 bytes in the data fork
		 */
		
		len = 6;
		FSRead(refNum,&len,(Ptr)&(GSavedENetAddr.eaddr));
		if (len != 6) {
			memset((void *)&GSavedENetAddr,0,sizeof(GSavedENetAddr));
			FSClose(refNum);
			return -1;
		}
		
		/*
		 *	Now read the last timestamp as 2 4-byte values
		 */
		
		len = 4;
		FSRead(refNum,&len,(Ptr)&(GLastTime.hi));
		if (len != 4) {
			memset((void *)&GLastTime,0,sizeof(GLastTime));
			FSClose(refNum);
			return -1;
		}
		len = 4;
		FSRead(refNum,&len,(Ptr)&(GLastTime.lo));
		if (len != 4) {
			memset((void *)&GLastTime,0,sizeof(GLastTime));
			FSClose(refNum);
			return -1;
		}
		len = 2;
		FSRead(refNum,&len,(Ptr)&(GTimeAdjust));
		if (len != 2) {
			GTimeAdjust = 0;
			GClockSeq = 0;
			memset((void *)&GLastTime,0,sizeof(GLastTime));
			FSClose(refNum);
			return -1;
		}
		len = 2;
		FSRead(refNum,&len,(Ptr)&(GClockSeq));
		if (len != 2) {
			GTimeAdjust = 0;
			GClockSeq = 0;
			memset((void *)&GLastTime,0,sizeof(GLastTime));
			FSClose(refNum);
			return -1;
		}
		FSClose(refNum);
		return noErr;
	} else {
		return err;
	}
}

/*	WritePrefData
 *
 *		Write the preferences data back out to my global variables.
 *	This gets called a couple of times. First, this is called by
 *	my GetRandomEthernet routine if I generated a psudorandom MAC
 *	address. Second, this is called when the library is being
 *	terminated through the __terminate() CFM call.
 *
 *		Note this does it's best attempt at writing the data out,
 *	and relies on ReadPrefData to check for integrety of the actual
 *	saved file.
 */

void WritePrefData(void)
{
	short refNum;
	FSSpec file;
	long len;

	/*
	 *	Find the preferences file name
	 */
	
	FindFolder(kOnSystemDisk,
			   kPreferencesFolderType,
			   kCreateFolder,
			   &file.vRefNum,
			   &file.parID);
	BlockMove((Ptr)"\pUUIDLib Preferences",(Ptr)file.name,20);

	/*
	 *	And save the data
	 */

	FSpCreate(&file,'UUID','pref',smSystemScript);
	if (0 == FSpOpenDF(&file,fsRdWrPerm,&refNum)) {
		if (0 == SetEOF(refNum,0L)) {
			len = 6;
			FSWrite(refNum,&len,(Ptr)&(GSavedENetAddr.eaddr));
			len = 4;
			FSWrite(refNum,&len,(Ptr)&(GLastTime.hi));
			len = 4;
			FSWrite(refNum,&len,(Ptr)&(GLastTime.lo));
			len = 2;
			FSWrite(refNum,&len,(Ptr)&(GTimeAdjust));
			len = 2;
			FSWrite(refNum,&len,(Ptr)&(GClockSeq));
		}
		FSClose(refNum);
	}
}

#pragma mark -


/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

//#include <stdio.h>
//#include "global.h"
//#include "md5.h"

/* Constants for MD5Transform routine.
 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform PROTO_LIST ((UINT4 [4], unsigned char [64]));
static void Encode PROTO_LIST
  ((unsigned char *, UINT4 *, unsigned int));
static void Decode PROTO_LIST
  ((UINT4 *, unsigned char *, unsigned int));
static void MD5_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init (MD5_CTX *context)
//MD5_CTX *context;                                        /* context */
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void MD5Update (MD5_CTX *context, unsigned char *input, unsigned int inputLen)
//MD5_CTX *context;                                        /* context */
//unsigned char *input;                                /* input block */
//unsigned int inputLen;                     /* length of input block */
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 MD5_memcpy
   ((POINTER)&context->buffer[index], (POINTER)input, partLen);
 MD5Transform (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (context->state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  MD5_memcpy
 ((POINTER)&context->buffer[index], (POINTER)&input[i],
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final (unsigned char digest[16], MD5_CTX *context)
//unsigned char digest[16];                         /* message digest */
//MD5_CTX *context;                                       /* context */
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update (context, bits, 8);

  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform (UINT4 state[4], unsigned char block[64])
//UINT4 state[4];
//unsigned char block[64];
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information. */
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode (unsigned char *output, UINT4 *input, unsigned int len)
//unsigned char *output;
//UINT4 *input;
//unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode (UINT4 *output, unsigned char *input, unsigned int len)
//UINT4 *output;
//unsigned char *input;
//unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */

static void MD5_memcpy (POINTER output, POINTER input, unsigned int len)
//POINTER output;
//POINTER input;
//unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)
  output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset (POINTER output, int value, unsigned int len)
//POINTER output;
//int value;
//unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)
 ((char *)output)[i] = (char)value;
}

/* Prints a message digest in hexadecimal.
 */
void MD5Print (char *dest, unsigned char digest[16])
//char *dest;
//unsigned char digest[16];
{
  unsigned int i;

  for (i = 0; i < 16; i++) {
    sprintf (dest, "%02x", digest[i]);
    dest += 2;
  }
}


#pragma mark -

/*	GetEthernetAddr.c
 *
 *		Gets the ethernet address. This first attempts to get the
 *	Ethernet address directly from a hardware address, and if that
 *	fails, attempts to open the Ethernet driver. If both fail, this
 *	punts. The UUIDLib code will then call a separate routine to
 *	generate a psudo-random token which we hope (and largely believe)
 *	will be sufficiently random that in combination with generating
 *	tokens with a 100ns resolution will be good enough.
 */
 

#include <ENET.h>			/* Macintosh routines for getting enet addr */
#include <Slots.h>
//#include "internal.h"

#if __POWERPC__					/* PowerPC Macintosh specific */
#include <NameRegistry.h>
#endif

/************************************************************************/
/*																		*/
/*	DTS Code															*/
/*																		*/
/*		The following was lifted straight out of the Apple DTS code		*/
/*	'GetENetAddrDirect.c', and only works on PowerPC Macintoshes		*/
/*																		*/
/************************************************************************/

#if powerc					/* PowerPC Macintosh specific */

/*
 *	Random enumerations and structures
 */
 
enum {
	kUnsupported 		= 0,
	kPDMMachine			= 1,
	kPCIMachine			= 2,
	kCommSlotMachine	= 3,
	kPCIComm2Machine 	= 4
};

enum {
	kPDMEnetROMBase	= 0x50f08000
};

enum {
//	gestaltPowerBookG3Series 		= 312,
	gestaltPowerBookG3SeriesFSTN 	= 314,
	gestaltiMac						= 406
//	,gestaltPowerMacG3 				= 510
};

/*	DoesCPUHaveBuiltInEthernet
 *
 *		This determines the kind of Mac I am, and returns a constant
 *	which indicates the type of built in Ethernet support is there.
 */

UInt32 DoesCPUHaveBuiltInEthernet(void)
{
	long		response;
	OSStatus	err;
	UInt32		result = kUnsupported;
	
	err = Gestalt(gestaltMachineType, &response);
	switch (response)
	{
		case gestaltPowerMac8100_120:
		case gestaltAWS9150_80:
		case gestaltPowerMac8100_110:
		case gestaltPowerMac7100_80:
		case gestaltPowerMac8100_100:
		case gestaltAWS9150_120:
		case gestaltPowerMac8100_80:
		case gestaltPowerMac6100_60:
		case gestaltPowerMac6100_66:
		case gestaltPowerMac7100_66:
			result = kPDMMachine;
			break;
		
		case gestaltPowerMac9500:
		case gestaltPowerMac7500:
		case gestaltPowerMac8500:
		case gestaltPowerBook3400:
		case gestaltPowerBookG3:
		case gestaltPowerMac7200:
		case gestaltPowerMac7300:
		case gestaltPowerBookG3Series:
		case gestaltPowerBookG3SeriesFSTN:
		case gestaltPowerMacG3:
		case gestaltiMac:
			result = kPCIMachine;
			break;
			
		case gestaltPowerMac5200:
		case gestaltPowerMac6200:
			result = kCommSlotMachine;
			break;
			
		case gestaltPowerMac6400:
		case gestaltPowerMac5400:
		case gestaltPowerMac5500:
		case gestaltPowerMac6500:
		case gestaltPowerMac4400_160:
		case gestaltPowerMac4400:
			result = kPCIComm2Machine;
		
	}
	
	if (response == kUnsupported)
	{
		err = Gestalt(gestaltNameRegistryVersion, (long*) &response);
		if (err == noErr)
			result = kPCIMachine;
	}
	return result;
}

/*	ByteSwapValue
 *
 *		That's because the hardware address on some PowerPC platforms
 *	are stored in bit swapped order.
 */

static UInt8 ByteSwapValue(UInt8 val)
{
	UInt8	result = 0;
	
	if (val & 0x01)
		result |= 0x80;
		
	if (val & 0x02)
		result |= 0x40;
		
	if (val & 0x04)
		result |= 0x20;
		
	if (val & 0x08)
		result |= 0x10;
		
	if (val & 0x10)
		result |= 0x08;
		
	if (val & 0x20)
		result |= 0x04;
		
	if (val & 0x40)
		result |= 0x02;
		
	if (val & 0x80)
		result |= 0x01;
	return result;
}

/*	GetPDMBuiltInEnetAddr
 *
 *		This pulls the ENet MAC address from the ROMs on machines which
 *	store the MAC address there.
 */

static OSStatus GetPDMBuiltInEnetAddr(UInt8 *enetaddr)
{
	UInt32	i;
	UInt8	*val;
	long osver;
	OSErr err;
	
	/*
	 *	Can I even do this operation? Check if I'm on MacOS X, and
	 *	if I am, punt. Perhaps another technique will work?
	 *
	 *	### Right now, I don't know how to do this. So instead,
	 *	knowing that no version of the MacOS Blue Box will run
	 *	anything earlier than OS 8.5, I punt on 8.5 or later.
	 *	Stupid, I know, but at least this doesn't cause the world
	 *	to end...
	 *
	 *	Note that this wasn't in the Apple DTS sample code. Seems
	 *	like a good idea, though, given the impending world of
	 *	protected memory is comming soon...
	 */
	
	err = Gestalt(gestaltSystemVersion,&osver);
	if (err) return err;
	if ((osver & 0x0000FFFF) >= 0x0850) return -1;	/* Bogus error code */
	
	/*
	 *	Now do it.
	 */

	for (i = 0; i < 6; i++)
	{
		val = (UInt8 *)(kPDMEnetROMBase + i * 0x10);
		enetaddr[i] = ByteSwapValue(*val);
	}
	return noErr;
}

/*	GetPCIBuiltInEnetAddr
 *
 *		Later PCI PowerPC Macintosh systems have a "Name Registry" (no,
 *	I hadn't heard of it before either) which contains the ENet MAC
 *	address. This searches for the address
 */

static OSStatus GetPCIBuiltInEnetAddr(UInt8 *enetaddr)
{
	OSStatus				err = noErr;
    RegEntryIter            cookie;
    RegEntryID              theFoundEntry;
    unsigned char          	enetAddrStr[32] = "\plocal-mac-address";
    RegCStrEntryNamePtr     enetAddrCStr = p2cstr( enetAddrStr );
    RegEntryIterationOp     iterOp;
    UInt8					enetAddr[6];
    Boolean                 done = false;
    RegPropertyValueSize	theSize;
	
    err = RegistryEntryIDInit( &theFoundEntry );
	if (err != noErr)
	{
		return err;
	}

    err = RegistryEntryIterateCreate( &cookie );
	if (err != noErr)
	{
		return err;
	}

    iterOp = kRegIterDescendants;

    err = RegistryEntrySearch( &cookie, iterOp, &theFoundEntry, &done,
                                                       enetAddrCStr, nil, 0);
    
    if (err == noErr)
    {
	    theSize = sizeof(enetAddr);;
	    err = RegistryPropertyGet(&theFoundEntry, enetAddrCStr, &enetAddr, &theSize );
	    if (err == noErr)
	    	BlockMove(enetAddr, enetaddr, sizeof(enetAddr));
	}

    RegistryEntryIterateDispose( &cookie );

	return noErr;
}

/*	GetPICComm2EnetAddr
 *
 *		This attempts to get the ENet MAC address from a different
 *	location in the name registry--some models use a different token
 *	than the one searched for above.
 */

OSStatus GetPCIComm2EnetAddr(UInt8 *enetaddr)
{
	OSStatus				err = noErr;
    RegEntryIter            cookie;
    RegEntryID              theFoundEntry;
    unsigned char          	enetAddrStr[32] = "\pASNT,ethernet-address";
    RegCStrEntryNamePtr     enetAddrCStr = p2cstr( enetAddrStr );
    RegEntryIterationOp     iterOp;
    UInt8					*enetAddr;
    Boolean                 done = false;
    RegPropertyValueSize	theSize;
	
    err = RegistryEntryIDInit( &theFoundEntry );
	if (err != noErr)
	{
		return err;
	}

    err = RegistryEntryIterateCreate( &cookie );
	if (err != noErr)
	{
		return err;
	}

    iterOp = kRegIterDescendants;

    err = RegistryEntrySearch( &cookie, iterOp, &theFoundEntry, &done,
                                                       enetAddrCStr, nil, 0);
    
    if (err == noErr)
    {
	    theSize = sizeof(enetAddr);;
	    err = RegistryPropertyGet(&theFoundEntry, enetAddrCStr, &enetAddr, &theSize );
	    if (err == noErr)
	    	BlockMove(enetAddr, enetaddr, 6);
	}

    RegistryEntryIterateDispose( &cookie );

	return noErr;
}

/************************************************************************/
/*																		*/
/*	Access the driver													*/
/*																		*/
/************************************************************************/

/*	GetHEthernetAddr
 *
 *		This implements the routines necessary for pulling the Ethernet
 *	Address directly from the hardware of certain PowerPC Macintosh
 *	system. This doesn't work with MacOS X, so I first test to make
 *	sure I'm on qualifying hardware running an operating system that
 *	won't barf if I start poking around inside the Apple ROMs.
 *
 *		This code is based on the GetEnetAddrDirect.c routine
 *	DisplayBurnedInAddress, except that I copy the data into my
 *	data structure instead. If there was a problem, I punt and move on.
 */

static OSErr GetHEthernetAddr(uuid_address_t *addr)
{
	OSStatus err;
	UInt32 cputype;
	UInt8 enetaddr[6];
	int i;
	
	cputype = DoesCPUHaveBuiltInEthernet();
	switch (cputype) {
		case kPDMMachine:
			err = GetPDMBuiltInEnetAddr(enetaddr);
			if (err) return err;
			break;
		case kPCIMachine:
			err = GetPCIBuiltInEnetAddr(enetaddr);
			if (err) return err;
			break;
		case kCommSlotMachine:
		case kPCIComm2Machine:
			/* This may or may not have an ethernet card in a slot. */
			/* Punt, and hope my driver code below handles this. */
			return -1;			/* Bogus error code */
		default:
			/* This is not a supported machine. Punt. */
			/* If the driver code below gets an ENet MAC addr, great */
			return -1;
	}
	
	/*
	 *	If we get here, our hardware technique worked. Copy the
	 *	address to the uuid_address_t structure
	 */
	
	for (i = 0; i < 6; i++) addr->eaddr[i] = enetaddr[i];
	return noErr;
}

#endif /* powerc */

/*	GetEthernetAddr
 *
 *		This attempts to open the .ENET driver and get the card information
 *	directly from that driver. Punt if I fail.
 *
 *		This uses the method described in IM:Networking, on the section on
 *	using the EtherNet driver. (It turns out the algorithms outlined in
 *	other places for opening the .ENET driver were not as reliable.)
 *
 *		Note that I do not close the driver. The reason for this is that
 *	other services may be using the .ENET driver. In particular on earlier
 *	versions of the MacOS, both parts of Open Transport and AppleTalk sit
 *	on top of the .ENET driver. And closing the .ENET driver leads to some
 *	really bad results.
 */

OSErr GetEthernetAddr(uuid_address_t *addr)
{
	char buffer[78];
	EParamBlock theEPB;

	SpBlock sp;
	ParamBlockRec pb;
	short refNum;
	short found;
	OSErr err;
	
	/*
	 *	Before I try the driver, first try getting the hardware
	 *	address without the driver. This only works on certain
	 *	models of PowerPC hardware.
	 */
	
#if powerc
	if (!GetHEthernetAddr(addr)) 
		return noErr;
#endif
	
	/*
	 *	Couldn't get it from the hardware, so I punt and try
	 *	to get it from the driver instead.
	 */
	
	found = 0;
	refNum = 0;

	memset(&sp,0,sizeof(sp));
	memset(&pb,0,sizeof(pb));
		
	sp.spParamData = 1;
	sp.spCategory = 4;		// network card
	sp.spCType = 1;			// ethernet
	sp.spTBMask = 3;
	sp.spSlot = 0;
	
	while ((err = SNextTypeSRsrc(&sp)) == noErr) {
		pb.slotDevParam.ioNamePtr = "\p.ENET";
		pb.slotDevParam.ioSPermssn = fsCurPerm;
		pb.slotDevParam.ioSlot = sp.spSlot;
		pb.slotDevParam.ioID = sp.spID;
		err = OpenSlot(&pb,FALSE);
		if (!err) break;
	}
	
	if (err) {
		/*
		 *	We didn't find a thing--try the ENET0 driver instead.
		 *	This is an alternative driver which is supplied for
		 *	non-NuBUS and slot-managed ethernet devices.
		 */
		
		err = OpenDriver("\p.ENET0",&refNum);
		if (err) return err;
	} else {
		refNum = pb.slotDevParam.ioSRefNum;
	}
	if (found) return err;			// didn't find card
	
	memset(buffer,0,sizeof(buffer));
	theEPB.ioRefNum = refNum;
	theEPB.u.EParms1.ePointer = buffer;
	theEPB.u.EParms1.eBuffSize = 78;
	theEPB.ioNamePtr = NULL;
	err = EGetInfo(&theEPB,0);

	memcpy(addr,buffer,sizeof(uuid_address_t));
	return err;
}

bool GetEthernetAddr(SEthernetAddress& outEthernetAddr)
{
	if (GetEthernetAddr((uuid_address_t *)(&outEthernetAddr)) == noErr)
		return true;
	
	outEthernetAddr.SetNull();
	return false;
}

#endif // TARGET_API_MAC_CARBON

#endif // MACINTOSH
