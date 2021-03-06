/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

	
/*	For ToText()/FromText():
 *	Note that the format is the same used by Microsoft and HP		
 *	xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx								
 *	time_low mid  hi/v clock  node										
 */
 
 
struct SGUID;

class UGUID
{
	public:
		static void Generate(SGUID &outGUID);
		static bool IsEqual(const SGUID &inGUIDA, const SGUID &inGUIDB);
		

		static Uint32 ToText(const SGUID &inGUID, void *outData, Uint32 inMaxSize);
		static bool FromText(SGUID &outGUID, const void *inData, Uint32 inDataSize);
		
		// for these, data must be 16 bytes
		static Uint32 Flatten(const SGUID &inGUID, void *outData);
		static void Unflatten(SGUID &outGUID, const void *inData);
};



struct SGUID
{
    Uint32		time_low;
    Uint16		time_mid;
    Uint16		time_hi_and_version;
    Uint8		clock_seq_hi_and_reserved;
    Uint8		clock_seq_low;
    Uint8		node[6];

	bool operator==(const SGUID &inGUID) const		{	return UGUID::IsEqual(*this, inGUID);	}
	bool operator!=(const SGUID &inGUID) const		{	return !UGUID::IsEqual(*this, inGUID);	}
	bool operator<(const SGUID &inGUID) const;
	bool operator>(const SGUID &inGUID) const;
	bool operator<=(const SGUID &inGUID) const;
	bool operator>=(const SGUID &inGUID) const;

};

