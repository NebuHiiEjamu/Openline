/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UKeyboard.h"

/* -------------------------------------------------------------------------- */

// returns 0 if none
Uint32 UKeyboard::FindKeyCommand(const SKeyMsgData& inInfo, const SKeyCmd *inCmds, Uint32 inCount)
{
	Uint32 i;
	Uint16 findCode = inInfo.keyCode;
	Uint16 findChar = UKeyboard::KeyToChar(findCode, 0);	//UText::ToLower(inInfo.keyChar);
	Uint16 findMods = inInfo.mods;
	Uint16 mods;

	for (i=0; i!=inCount; i++)
	{
		mods = inCmds[i].mods;
		
		if (mods & kIsKeyCode)
		{
			if (inCmds[i].key == findCode)
			{
				mods &= ~kIsKeyCode;
				if ((findMods & mods) == mods)
					return inCmds[i].id;
			}
		}
		else
		{
			if ((inCmds[i].key == findChar) && ((findMods & mods) == mods))
				return inCmds[i].id;
		}
	}
	
	return 0;
}



