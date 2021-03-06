/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"
#include "UEditText.h"
#include "CScrollerView.h"

class CTextView : public CView
{	
	public:
		// construction
		CTextView(CViewHandler *inHandler, const SRect& inBounds, Uint16 inMargin = 0);
		virtual ~CTextView();
		
		// font
		void SetFont(TFontDesc inFont);
		void SetFont(const Uint8 *inName, const Uint8 *inStyle, Uint32 inSize, Uint32 inEffect = 0);
		TFontDesc GetFont() const												{	return UEditText::GetFont(mText);							}
		void SetFontSize(Uint32 inSize);
		Uint32 GetFontSize() const												{	return UEditText::GetFontSize(mText);						}
		void SetColor(const SColor& inColor)									{	UEditText::SetColor(mText, inColor);						}
		void GetColor(SColor& outColor) const									{	UEditText::GetColor(mText, outColor);						}
		void SetAlign(Uint32 inAlign)											{	UEditText::SetAlign(mText, inAlign);						}
		Uint32 GetAlign() const													{	return UEditText::GetAlign(mText);							}
		
		// properties
		void SetSelection(Uint32 inSelectStart, Uint32 inSelectEnd)				{	UEditText::SetSelect(mText, inSelectStart, inSelectEnd);	}
		void GetSelection(Uint32& outSelectStart, Uint32& outSelectEnd) const	{	UEditText::GetSelect(mText, outSelectStart, outSelectEnd);	}
		void GetSelectionRegion(TRegion outRgn) const							{	UEditText::GetSelectRegion(mText, outRgn);					}
		void SetEditable(bool inEditable)										{	UEditText::SetEditable(mText, inEditable);					}
		bool IsEditable() const													{	return UEditText::IsEditable(mText);						}
		void SetSelectable(bool inSelectable)									{	UEditText::SetSelectable(mText, inSelectable);				}
		bool IsSelectable() const												{	return UEditText::IsSelectable(mText);						}

		// text
		void SetText(const void *inText, Uint32 inLength);
		Uint32 GetText(void *outText, Uint32 inMaxLength) const					{	return UEditText::GetText(mText, outText, inMaxLength);		}
		void SetTextHandle(THdl inHdl);
		THdl GetTextHandle() const												{	return UEditText::GetTextHandle(mText);						}
		THdl DetachTextHandle();
		void InsertText(Uint32 inOffset, const void *inText, Uint32 inTextLength, bool inUpdateSelect = false);
		void ReplaceText(Uint32 inOffset, Uint32 inExistingLength, const void *inText, Uint32 inTextLength, bool inUpdateSelect = false);
		void DeleteText(Uint32 inOffset, Uint32 inTextLength, bool inUpdateSelect = false);
		bool IsEmpty() const													{	return UEditText::IsEmpty(mText);							}
		Uint32 GetTextSize() const												{	return UEditText::GetTextSize(mText);						}

		// misc
		virtual void Timer(TTimer inTimer);
		virtual Uint32 GetFullHeight() const;
		void SetEnterKeyAction(Uint16 inReturnAction, Uint16 inEnterAction = enterKeyAction_None);
		Uint16 GetReturnKeyAction();
		Uint16 GetEnterKeyAction();
//		virtual bool CanAcceptDrop(TDrag inDrag) const;
		virtual bool IsSelfDrag(TDrag inDrag) const;
		virtual bool ChangeState(Uint16 inState);
		void SetTabSelectText(bool inTabSelect);
		virtual bool TabFocusNext();
		virtual bool TabFocusPrev();
		virtual bool SetEnable(bool inEnable);
		virtual bool SetBounds(const SRect& inBounds);
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);

		// events
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual bool KeyDown(const SKeyMsgData& inInfo);
		virtual void KeyUp(const SKeyMsgData& inInfo);
		virtual bool KeyRepeat(const SKeyMsgData& inInfo);
		virtual void DragEnter(const SDragMsgData& inInfo);
		virtual void DragMove(const SDragMsgData& inInfo);
		virtual void DragLeave(const SDragMsgData& inInfo);
		virtual bool Drop(const SDragMsgData& inInfo);

	protected:
		TEditText mText;
		TTimer mCaretTimer;
		Uint16 mReturnKeyAction;
		Uint16 mEnterKeyAction;
		bool mTabSelect;
		
		// internal functions
		static void TextDrawHandler(TEditText inRef, const SRect& inRect);
		static void TextScreenDeltaHandler(TEditText inRef, Int32& outHoriz, Int32& outVert);
		void UpdateActive();
};

CScrollerView *MakeTextBoxView(CViewHandler *inHandler, 
							   const SRect& inBounds, 
							   Uint16 inOptions = scrollerOption_Border, 
							   CTextView **outTextView = nil);


