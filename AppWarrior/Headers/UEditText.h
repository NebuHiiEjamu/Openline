/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UGraphics.h"

typedef struct SEditText *TEditText;
typedef void (*TEditTextDrawProc)(TEditText inRef, const SRect& inRect);
typedef void (*TEditTextScreenDeltaProc)(TEditText inRef, Int32& outHoriz, Int32& outVert);

enum
{
	enterKeyAction_None  	= 0, // none
	enterKeyAction_NewLine 	= 1, // new line in the text field
	enterKeyAction_Hit  	= 2  // calls Hit()
};

class UEditText
{
	public:
		// new and dispose
		static TEditText New(const SRect& inBounds, Uint16 inMargin = 0);
		static void Dispose(TEditText inRef);
		
		// bounds
		static void SetBounds(TEditText inRef, const SRect& inBounds);
		static void GetBounds(TEditText inRef, SRect& outBounds);
		static Uint32 GetFullHeight(TEditText inRef);
		static Uint16 GetMargin(TEditText inRef);
		
		// font
		static void SetFont(TEditText inRef, TFontDesc inFont);
		static void SetFont(TEditText inRef, const Uint8 *inName, const Uint8 *inStyle, Uint32 inSize, Uint32 inEffect = 0);
		static TFontDesc GetFont(TEditText inRef);
		static void SetFontSize(TEditText inRef, Uint32 inSize);
		static Uint32 GetFontSize(TEditText inRef);
		static void SetColor(TEditText inRef, const SColor& inColor);
		static void GetColor(TEditText inRef, SColor& outColor);
		static void SetAlign(TEditText inRef, Uint32 inAlign);
		static Uint32 GetAlign(TEditText inRef);
		
		// properties
		static void SetSelect(TEditText inRef, Uint32 inSelectStart, Uint32 inSelectEnd);
		static void GetSelect(TEditText inRef, Uint32& outSelectStart, Uint32& outSelectEnd);
		static void GetSelectRegion(TEditText inRef, TRegion outRgn);
		static void GetSelectRect(TEditText inRef, SRect& outRect);
		static bool SetActive(TEditText inRef, bool inActive);
		static bool IsActive(TEditText inRef);
		static void GetActiveRect(TEditText inRef, SRect& outRect);
		static void SetRef(TEditText inRef, void *inValue);
		static void *GetRef(TEditText inRef);
		
		// edit status
		static void SetEditable(TEditText inRef, bool inEditable);
		static bool IsEditable(TEditText inRef);
		static void SetSelectable(TEditText inRef, bool inSelectable);
		static bool IsSelectable(TEditText inRef);
		
		// text
		static void SetText(TEditText inRef, const void *inText, Uint32 inLength);
		static Uint32 GetText(TEditText inRef, void *outText, Uint32 inMaxLength);
		static Uint32 GetText(TEditText inRef, Uint32 inOffset, Uint32 inSourceSize, void *outText, Uint32 inMaxSize);
		static void SetTextHandle(TEditText inRef, THdl inHdl);
		static THdl GetTextHandle(TEditText inRef);
		static THdl DetachTextHandle(TEditText inRef);
		static void InsertText(TEditText inRef, Uint32 inOffset, const void *inText, Uint32 inTextLength, bool inUpdateSelect = false);
		static void ReplaceText(TEditText inRef, Uint32 inOffset, Uint32 inExistingLength, const void *inText, Uint32 inTextLength, bool inUpdateSelect = false);
		static void DeleteText(TEditText inRef, Uint32 inOffset, Uint32 inTextLength, bool inUpdateSelect = false);
		static Uint32 MoveText(TEditText inRef, Uint32 inOffset, Uint32 inTextSize, Uint32 inNewOffset, bool inUpdateSelect = false);
		static bool IsEmpty(TEditText inRef);
		static Uint32 GetTextSize(TEditText inRef);
		
		// clipboard
		static void ClipCut(TEditText inRef);
		static void ClipCopy(TEditText inRef);
		static void ClipPaste(TEditText inRef);
		
		// drawing
		static void SetDrawHandler(TEditText inRef, TEditTextDrawProc inProc);
		static TEditTextDrawProc GetDrawHandler(TEditText inRef);
		static void Draw(TEditText inRef, TImage inImage, const SRect& inUpdateRect, Uint16 inDepth = 8);
		static void SetScreenDeltaHandler(TEditText inRef, TEditTextScreenDeltaProc inProc);
		static TEditTextScreenDeltaProc GetScreenDeltaHandler(TEditText inRef);

		// events
		static void MouseDown(TEditText inRef, const SMouseMsgData& inInfo);
		static void MouseUp(TEditText inRef, const SMouseMsgData& inInfo);
		static void MouseEnter(TEditText inRef, const SMouseMsgData& inInfo);
		static void MouseLeave(TEditText inRef, const SMouseMsgData& inInfo);
		static void MouseMove(TEditText inRef, const SMouseMsgData& inInfo);
		static bool KeyChar(TEditText inRef, const SKeyMsgData& inInfo);
		static void DragEnter(TEditText inRef, const SDragMsgData& inInfo);
		static void DragLeave(TEditText inRef, const SDragMsgData& inInfo);
		static void DragMove(TEditText inRef, const SDragMsgData& inInfo);
		static bool Drop(TEditText inRef, const SDragMsgData& inInfo);
//		static bool CanAcceptDrop(TEditText inRef, TDrag inDrag);
		static bool IsSelfDrag(TEditText inRef, TDrag inDrag);

		// misc
		static void BlinkCaret(TEditText inRef);
		static bool GetCaretRect(TEditText inRef, SRect& outRect);
		
		// low-level functions
		static void LowCalcLines(TImage inImage, THdl inText, THdl outOffsets, Uint32 inMaxWidth, Uint32& outLineCount);
		static bool LowInsert(TImage inImage, THdl ioText, THdl ioOffsets, Uint32 inMaxWidth, Uint32& ioLineCount, Uint32 inOffset, const void *inNewText, Uint32 inNewTextSize, Uint32 *outDrawStart, Uint32 *outDrawEnd);
		static bool LowDelete(TImage inImage, THdl ioText, THdl ioOffsets, Uint32 inMaxWidth, Uint32& ioLineCount, Uint32 inOffset, Uint32 inDeleteSize, Uint32 *outDrawStart, Uint32 *outDrawEnd);
		static bool LowReplace(TImage inImage, THdl ioText, THdl ioOffsets, Uint32 inMaxWidth, Uint32& ioLineCount, Uint32 inOffset, Uint32 inExistingSize, const void *inNewText, Uint32 inNewTextSize, Uint32 *outDrawStart, Uint32 *outDrawEnd);
};



