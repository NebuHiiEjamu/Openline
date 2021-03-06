/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "URegion.h"
#include "UMemory.h"

// shape fills
enum 
{
	fill_None				= 0,	// shape not drawn
	fill_OpenFrame			= 1,	// framed, one edge left open
	fill_ClosedFrame		= 2,	// framed, closed completely
	fill_EvenOdd			= 3,	// filled using even-odd rule
	fill_Winding			= 4,	// filled using winding-number rule
	fill_InverseEvenOdd		= 5,	// filled inverse of even-odd rule
	fill_InverseWinding		= 6,	// filled inverse of winding-number

	fill_Frame				= fill_OpenFrame,							
	fill_Hollow				= fill_ClosedFrame,							
	fill_Solid				= fill_EvenOdd,							
	fill_InverseSolid		= fill_InverseEvenOdd,
	fill_Inverse			= fill_InverseEvenOdd
};

// transfer modes
enum 
{
	mode_None				= 0,	// no transfer occurs
	mode_Copy				= 1,	// the source color component is copied to the destination
	mode_Add				= 2,	// the source color component is added to the destination component
	mode_Blend				= 3,	// the result is the average of the source and destination color components
	mode_Migrate			= 4,	// the destination color component is moved toward the source component
	mode_Minimum			= 5,	// the source component replaces the destination component only if the source component has a smaller value
	mode_Maximum			= 6,	// the source component replaces the destination component only if the source component has a larger value
	mode_Highlight			= 7,	// the source component and operand component are swapped in the destination
	mode_And				= 8,	// the bits of the source color and destination color are combined using an AND operation
	mode_Or					= 9,	// the bits of the source color and destination color are combined using an OR operation
	mode_Xor				= 10,
	mode_RampAnd			= 11,
	mode_RampOr				= 12,
	mode_RampXor			= 13,
	mode_Over				= 14,
	mode_Atop				= 15,
	mode_Exclude			= 16,
	mode_Fade				= 17
};

// shape types
enum 
{
	shape_None				= 0,
	shape_Empty				= 1,
	shape_Point				= 2,
	shape_Line				= 3,
	shape_Curve				= 4,
	shape_Rectangle			= 5,
	shape_Polygon			= 6,
	shape_Path				= 7,
	shape_Pixmap			= 8,
	shape_Text				= 9,
	shape_Glyph				= 10,
	shape_Layout			= 11,
	shape_Full				= 12,
	shape_Picture			= 13
};

// ink attributes
enum 
{
	inkAttr_AlignDither			= 0x0001,
	inkAttr_ForceDither			= 0x0002,
	inkAttr_SuppressDither		= 0x0004,
	inkAttr_SuppressHalftone	= 0x0008
};

// point types for paths
enum 
{
	pointType_MoveTo		= 1,		// start disjoint figure
	pointType_LineTo		= 2,		// straight line
	pointType_3BezierTo		= 3,		// quadratic bezier curve (must have 2 consecutive)
	pointType_4BezierTo		= 4,		// cubic bezier curve (must have 3 consecutive)
	pointType_5BezierTo		= 5,
	pointType_6BezierTo		= 6,
	
	pointType_CloseFigure	= 0x80,		// automatically close figure
	
	pointType_QuadraticBezierTo			= pointType_3BezierTo,
	pointType_CubicBezierTo				= pointType_4BezierTo,
	pointType_QuadraticBezierCloseTo	= pointType_3BezierTo | pointType_CloseFigure,
	pointType_CubicBezierCloseTo		= pointType_4BezierTo | pointType_CloseFigure
};

enum 
{
	fontEffect_Plain		= 0x0000,
	fontEffect_Bold			= 0x0001,	// bold style or synthesize default amount of bolding
	fontEffect_Italic		= 0x0002,	// italic style or synthesize default amount of shear
	fontEffect_Underline	= 0x0004,
	fontEffect_Outline		= 0x0008,
	fontEffect_StrikeOut	= 0x0010,
	fontEffect_Condense		= 0x0020,
	fontEffect_Extend		= 0x0040
};

enum 
{
	fontWeight_Default		= 0,
	fontWeight_Thin			= 100,
	fontWeight_ExtraLight	= 200,
	fontWeight_UltraLight	= 200,
	fontWeight_Light		= 300,
	fontWeight_Normal		= 400,
	fontWeight_Regular		= 400,
	fontWeight_Medium		= 500,
	fontWeight_Semibold		= 600,
	fontWeight_Demibold		= 600,
	fontWeight_Bold			= 700,
	fontWeight_ExtraBold	= 800,
	fontWeight_UltraBold	= 800,
	fontWeight_Heavy		= 900,
	fontWeight_Black		= 900
};

enum 
{
	textAlign_Default		= 0,
	textAlign_Left			= 1,
	textAlign_Right			= 2,
	textAlign_Center		= 3,
	textAlign_Full			= 4
};

Uint8 *const kDefaultFont = (Uint8 *)max_Uint32;
Uint8 *const kSystemFont = (Uint8 *)(max_Uint32-1);
Uint8 *const kFixedFont = (Uint8 *)(max_Uint32-2);
Uint8 *const kSansFont = (Uint8 *)(max_Uint32-3);
Uint8 *const kSerifFont = (Uint8 *)(max_Uint32-4);

#pragma options align=packed
struct SFontMetrics 
{
	Uint32 ascent;			// height above baseline including accent marks
	Uint32 descent;			// depth below baseline
	Uint32 lineSpace;		// extra space to separate lines
	Uint32 internal;		// space for accent marks
	Uint32 rsvd[4];			// set to 0
};

struct SFontDesc 
{
	Uint8 *name;			// font family name, eg "Helvetica", "Times New Roman"
	Uint8 *style;			// style name, eg "ultra bold", "bold italic", "narrow"
	Uint32 size;			// height in pixels
	Uint32 effect;			// synthesized effects, see fontEffect constants
	Uint32 weight;			// synthesized bold, see fontWeight constants
	Uint32 shear;			// synthesized italic
	Uint32 encoding;		// 0=default
	Uint32 textAlign;		// used with UFontDesc
	SColor *color;			// if supplied (not nil), calls SetInkColor()
	SColor *customColor;	// used with UFontDesc, usually nil
	Uint32 customVal;		// used with UFontDesc, usually 0
	Uint32 rsvd[5];			// must all be 0
};
#pragma options align=reset

// forward declare
class SColor;
class SRect;
class SRoundRect;
class SLine;

// reference types
typedef class TImageObj *TImage;
typedef class TFontDescObj *TFontDesc;
typedef struct SPattern **TPattern;
typedef struct SPicture **TPicture;
struct SPixmap;

// simple pattern
#pragma options align=mac68k
struct SSimplePattern 
{
	Uint8 pat[8];
};
typedef struct SSimplePattern SSimplePattern;
#pragma options align=reset

// icon transform types
enum 
{
	transform_None			= 0x0000,
	transform_Dark			= 0x4000,
	transform_Light			= 0x0001
};

class UGraphics
{
	public:
		// initialize
		static void Init();
	
		// coordinate system origin
		static void SetOrigin(TImage inImage, const SPoint& inOrigin);
		static void GetOrigin(TImage inImage, SPoint& outOrigin);
		static void AddOrigin(TImage inImage, Int32 inHorizDelta, Int32 inVertDelta);
		static void ResetOrigin(TImage inImage);
		
		// clipping properties
		static void SetClip(TImage inImage, TRegion inClip);
		static void SetClip(TImage inImage, const SRect& inClip);
		static void GetClip(TImage inImage, TRegion outClip);
		static void GetClip(TImage inImage, SRect& outClip);
		static void SetNoClip(TImage inImage);
		static void IntersectClip(TImage inImage, const SRect& inClip);
		static void IntersectClip(TImage inImage, TRegion inClip);
		static void IntersectClip(TImage inImage, TRegion inRgn, const SRect& inRect);
		static void MoveClip(TImage inImage, Int32 inHorizDelta, Int32 inVertDelta);
		
		// misc properties
		static Uint16 GetDepth(TImage inImage);
		static void Reset(TImage inImage);

		// pen properties
		static void ResetPen(TImage inImage);
		static void SetPenSize(TImage inImage, Uint32 inSize);
		static Uint32 GetPenSize(TImage inImage);

		// ink properties
		static void ResetInk(TImage inImage);
		static void SetInkAttributes(TImage inImage, Uint32 inAttrib);
		static void SetInkColor(TImage inImage, const SColor& inColor);
		static void SetInkMode(TImage inImage, Uint32 inTransferMode, const SColor *inOperand = nil);
		static Uint32 GetInkAttributes(TImage inImage);
		static void GetInkColor(TImage inImage, SColor& outColor);
		static void GetInkMode(TImage inImage, Uint32& outTransferMode, SColor& outOperand);
		static Uint32 GetInkMode(TImage inImage);
		static void GetNearestColor(TImage inImage, SColor& ioColor);

		// draw shapes
		static void DrawPixel(TImage inImage, const SPoint& inPixel);
		static void DrawLine(TImage inImage, const SLine& inLine, Uint32 inOptions = 0);
		static void DrawLines(TImage inImage, const SLine *inLines, Uint32 inCount, Uint32 inOptions = 0);
		static void DrawRect(TImage inImage, const SRect& inRect, Uint32 inFill);
		static void DrawOval(TImage inImage, const SRect& inRect, Uint32 inFill);
		static void DrawRoundRect(TImage inImage, const SRoundRect& inRect, Uint32 inFill);
		static void DrawPolygon(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, Uint32 inFill, Uint32 inOptions = 0);
		static void DrawRegion(TImage inImage, TRegion inRgn, Uint32 inFill);
		static void FillRect(TImage inImage, const SRect& inRect);
		static void FrameRect(TImage inImage, const SRect& inRect);
		static void SetPixel(TImage inImage, const SPoint& inPixel, const SColor& inColor);
		static void SetPixels(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, const SColor& inColor);
		static void GetPixel(TImage inImage, const SPoint& inPixel, SColor& outColor);
		static void FloodFill(TImage inImage, const SPoint& inPixel);
		
		// bezier functions
		static Uint32 CalcBezierPoints(const SPoint *inControlPoints, Uint32 inControlCount, SPoint *outPoints, Uint32 inMaxCount);
		static Uint32 EstimateBezierPointCount(const SPoint *inControlPoints, Uint32 inControlCount);
		
		// font properties
		static void ResetFont(TImage inImage);
		static void SetFontName(TImage inImage, const Uint8 *inName, const Uint8 *inStyle = nil);
		static void SetFontSize(TImage inImage, Uint32 inSize);
		static Uint32 GetFontSize(TImage inImage);
		static void SetFontEffect(TImage inImage, Uint32 inFlags);
		static Uint32 GetFontEffect(TImage inImage);
		static void SetFont(TImage inImage, const Uint8 *inName, const Uint8 *inStyle, Uint32 inSize, Uint32 inEffect = 0);
		static void SetFont(TImage inImage, const SFontDesc& inInfo);
		static void SetFont(TImage inImage, TFontDesc inInfo);

		// font and text information
		static void GetFontMetrics(TImage inImage, SFontMetrics& outInfo);
		static Uint32 GetFontHeight(TImage inImage);
		static Uint32 GetFontLineHeight(TImage inImage);
		static Uint32 GetCharWidth(TImage inImage, Uint16 inChar, Uint32 inEncoding = 0);
		static Uint32 GetTextWidth(TImage inImage, const void *inText, Uint32 inTextSize, Uint32 inEncoding = 0);
		static Uint32 GetTextLineBreak(TImage inImage, const void *inText, Uint32 inTextSize, Uint32 inMaxWidth);
		static Uint32 WidthToChar(TImage inImage, const void *inText, Uint32 inTextSize, Uint32 inWidth, bool *outLeftSide = nil);

		// draw text
		static void DrawText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags);
		static void DrawTruncText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags);
		static void DrawTextBox(TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign);
		static Uint32 GetTextBoxHeight(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign);
		static void DrawTextLines(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inEncoding, const Uint32 *inLineOffsets, Uint32 inStartLine, Uint32 inEndLine, Uint32 inLineHeight, Uint32 inAlign);

		// offscreen drawing
		static TImage NewCompatibleImage(Uint32 inWidth, Uint32 inHeight);
		static void DisposeImage(TImage inImage);
		static void LockImage(TImage inImage);
		static void UnlockImage(TImage inImage);
		static void StretchPixels(TImage inDest, const SRect& inDestRect, TImage inSource, const SRect& inSourceRect, Uint32 inOptions = 0);
		static void GetImageSize(TImage inImage, Uint32& outWidth, Uint32& outHeight);
	
		// copying pixels
		static void CopyPixels(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions = 0);
		static void CopyPixels(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions = 0);
		static void CopyPixels(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions = 0);
		static void CopyPixelsMasked(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, TImage inTransMask, Uint32 inOptions = 0);
		static void CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions = 0);
		static void CopyPixelsTrans(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions = 0);
		static void CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions = 0);
		static bool ValidateCopyRects(const SRect& inDstBnd, const SRect& inSrcBnd, SPoint& ioDstPt, SPoint& ioSrcPt, Uint32& ioWidth, Uint32& ioHeight);

		// raw pixel access
		static void GetRawPixels(TImage inImage, const SRect& inRect, void *outData, Uint32 inOptions = 0);
		static void SetRawPixels(TImage inImage, const SRect& inRect, const void *inData, Uint32 inOptions = 0);
		static Uint32 MinimizeColorTable(const Uint32 *inTable, Uint32 inCount, Uint32 *outTable);
		static Uint32 MergeColorTable(Uint32 *ioColorTab, Uint32 inInitialCount, Uint32 inMaxColors, const Uint32 *inNewColors, Uint32 inNewColorCount);
		static Uint32 GetNearestColor(Uint32 inColor, const Uint32 *inColorTab, Uint32 inColorCount, Uint32 inOptions = 0);
		static bool GetExactColor(Uint32 inColor, const Uint32 *inColorTab, Uint32 inColorCount, Uint32& outIndex);
		static bool GetExactColor(Uint16 inColor, const Uint16 *inColorTab, Uint32 inColorCount, Uint32& outIndex);
		static Uint32 BuildColorTable(TImage inImage, const SRect& inRect, Uint32 inMethod, Uint32 *outColors, Uint32 inMaxColors);

		// misc
		static void GetLinePoly(const SPoint& inStart, const SPoint& inEnd, Uint32 inWidth, SPoint *outPts);
		static bool UserSelectColor(SColor& ioColor, const Uint8 *inPrompt = nil, Uint32 inOptions = 0);

		/*********** BEGIN OLD FUNCTIONS LIKELY TO CHANGE ***********/

		// color properties
		static void SetBackColor(TImage inImage, const SColor& inColor);
		static void GetBackColor(TImage inImage, SColor& outColor);
		static void SetWhiteBack(TImage inImage);
		static void SetColorToBack(TImage inImage);

		// pattern properties
		static void SetPattern(TImage inImage, TPattern inPat);
		static void SetPattern(TImage inImage, const SSimplePattern& inPat);
		static void GetPattern(TImage inImage, TPattern outPat);
		static void SetBackPattern(TImage inImage, TPattern inPat);
		static void SetBackPattern(TImage inImage, const SSimplePattern& inPat);
		static void GetBackPattern(TImage inImage, TPattern outPat);

		// images
		static TImage NewImage(const SRect& inBounds);

		// coordinate properties
		//static void SetBounds(TImage inImage, const SRect& inBounds);
		static void GetBounds(TImage inImage, SRect& outBounds);

		// pixels
		static void CopyPixels(TImage inDestPort, const SRect& inDestRect, TImage inSourcePort, const SRect& inSourceRect, Int16 inMode, TRegion inClip = nil);
		static void ScrollPixels(TImage inImage, const SRect& inSourceRect, Int16 inDistHoriz, Int16 inDistVert, TRegion outUpdateRgn);
		
		// pictures
		static void DrawPicture(TImage inImage, TPicture inPict, const SRect& inDestRect);
		static void DrawPicture(TImage inImage, Int16 inPictID, const SRect& inDestRect);
		static Int32 GetPictureHeight(TPicture inPict);
		static Int32 GetPictureWidth(TPicture inPict);
		
		// misc
		static TImage GetDummyImage();
		static TImage GetDesktopImage();
		static TPattern GetResPattern(Int16 inID);
		static void DisposePattern(TPattern inPat);
		
		/************ END FUNCTIONS LIKELY TO CHANGE ************/
};

typedef void (*TEnumFontNamesProc)(const Uint8 *inName, Uint32 inEncoding, Uint32 inFlags, void *inRef);

class UFontDesc
{
	public:
		// construction
		static TFontDesc New(const SFontDesc& inInfo);
		static TFontDesc New(const Uint8 *inName, const Uint8 *inStyle, Uint32 inSize, Uint32 inEffect = 0);
		static void Dispose(TFontDesc inRef);
		static TFontDesc Clone(TFontDesc inRef);
		static void SetLock(TFontDesc inRef, bool inLock);
		static bool IsLocked(TFontDesc inRef);

		// default settings
		static void SetDefault(const SFontDesc& inInfo);

		// font properties
		static void SetFontName(TFontDesc inRef, const Uint8 *inName, const Uint8 *inStyle = nil);
		static bool SetFontSize(TFontDesc inRef, Uint32 inSize);
		static Uint32 GetFontSize(TFontDesc inRef);
		static void SetFontEffect(TFontDesc inRef, Uint32 inFlags);
		static Uint32 GetFontEffect(TFontDesc inRef);

		// text properties
		static void SetAlign(TFontDesc inRef, Uint32 inAlign);
		static Uint32 GetAlign(TFontDesc inRef);
		static void SetColor(TFontDesc inRef, const SColor& inColor);
		static void GetColor(TFontDesc inRef, SColor& outColor);
		static void SetCustomColor(TFontDesc inRef, const SColor& inColor);
		static void GetCustomColor(TFontDesc inRef, SColor& outColor);
		static void SetCustomValue(TFontDesc inRef, Uint32 inVal);
		static Uint32 GetCustomValue(TFontDesc inRef);

		// flattening
		static THdl Flatten(const SFontDesc& inInfo);
		static TFontDesc Unflatten(const void *inData, Uint32 inDataSize);
		
		// listing fonts
		static void EnumFontNames(TEnumFontNamesProc inProc, void *inRef = nil);
};

// synonyms
typedef UGraphics UGraf;

// constants
extern const SSimplePattern pattern_White, pattern_LightGray, pattern_Gray, pattern_DarkGray, pattern_Black;

// globals
extern TRegion gWorkRgn;

/*
 * Stack Helpers
 */

class StImageLocker
{
	public:
		StImageLocker(TImage inImage) : mImage(inImage) 
		{	UGraphics::LockImage(inImage); }
		~StImageLocker() 
		{	UGraphics::UnlockImage(mImage); }
	
	protected:
		TImage mImage;
};


class StBackColorSaver
{
	public:
		StBackColorSaver(TImage inImage) : mImage(inImage) 
		{ UGraphics::GetBackColor(inImage, mBackColor); }
		~StBackColorSaver() 
		{ UGraphics::SetBackColor(mImage, mBackColor); }
	private:
		TImage mImage;
		SColor mBackColor;
};

class StClipSaver
{
	public:
		StClipSaver(TImage inImage) : mImage(inImage) 
		{ UGraphics::GetClip(inImage, mClip); }
		~StClipSaver() 
		{ UGraphics::SetClip(mImage, mClip); }
		TRegion GetClip() const 
		{ return mClip; }
	private:
		TImage mImage;
		StRegion mClip;
};

class StOriginSaver
{
	public:
		StOriginSaver(TImage inImage) : mImage(inImage) 
		{ UGraphics::GetOrigin(inImage, mOrigin); }
		~StOriginSaver() 
		{ UGraphics::SetOrigin(mImage, mOrigin); }
	private:
		TImage mImage;
		SPoint mOrigin;
};

/*
 * UGraphics Object Interface
 */

class TImageObj
{
	public:
		void SetOrigin(const SPoint& inOrigin)																
		{	UGraphics::SetOrigin(this, inOrigin); }

		void GetOrigin(SPoint& outOrigin)
		{	UGraphics::GetOrigin(this, outOrigin); }

		void AddOrigin(Int32 inHorizDelta, Int32 inVertDelta)												
		{	UGraphics::AddOrigin(this, inHorizDelta, inVertDelta); }

		void ResetOrigin()																					
		{	UGraphics::ResetOrigin(this); }

		void SetClip(TRegion inClip) 
		{	UGraphics::SetClip(this, inClip); }

		void SetClip(const SRect& inClip) 
		{	UGraphics::SetClip(this, inClip); }

		void GetClip(TRegion outClip) 
		{	UGraphics::GetClip(this, outClip); }

		void GetClip(SRect& outClip) 
		{	UGraphics::GetClip(this, outClip); }

		void SetNoClip() 
		{	UGraphics::SetNoClip(this); }

		void IntersectClip(const SRect& inClip) 
		{	UGraphics::IntersectClip(this, inClip); }

		void IntersectClip(TRegion inClip) 
		{	UGraphics::IntersectClip(this, inClip); }

		void IntersectClip(TRegion inRgn, const SRect& inRect) 
		{	UGraphics::IntersectClip(this, inRgn, inRect); }

		void MoveClip(Int32 inHorizDelta, Int32 inVertDelta) 
		{	UGraphics::MoveClip(this, inHorizDelta, inVertDelta); }

		void Reset() 
		{	UGraphics::Reset(this); }

		void ResetPen() 
		{	UGraphics::ResetPen(this); }

		void SetPenSize(Uint32 inSize) 
		{	UGraphics::SetPenSize(this, inSize); }

		Uint32 GetPenSize() 
		{	return UGraphics::GetPenSize(this); }

		void ResetInk() 
		{	UGraphics::ResetInk(this); }

		void SetInkAttributes(Uint32 inAttrib) 
		{	UGraphics::SetInkAttributes(this, inAttrib); }

		void SetInkColor(const SColor& inColor) 
		{	UGraphics::SetInkColor(this, inColor); }

		void SetInkMode(Uint32 inTransferMode, const SColor *inOperand = nil) 
		{	UGraphics::SetInkMode(this, inTransferMode, inOperand); }

		Uint32 GetInkAttributes() 
		{	return UGraphics::GetInkAttributes(this); }

		void GetInkColor(SColor& outColor) 
		{	UGraphics::GetInkColor(this, outColor); }

		void GetInkMode(Uint32& outTransferMode, SColor& outOperand) 
		{	UGraphics::GetInkMode(this, outTransferMode, outOperand); }

		Uint32 GetInkMode() 
		{	return UGraphics::GetInkMode(this); }

		void GetNearestColor(SColor& ioColor) 
		{	UGraphics::GetNearestColor(this, ioColor); }

		void DrawPixel(const SPoint& inPixel) 
		{	UGraphics::DrawPixel(this, inPixel); }

		void DrawLine(const SLine& inLine, Uint32 inOptions = 0) 
		{	UGraphics::DrawLine(this, inLine, inOptions); }

		void DrawLines(const SLine *inLines, Uint32 inCount, Uint32 inOptions = 0) 
		{	UGraphics::DrawLines(this, inLines, inCount, inOptions); }

		void DrawRect(const SRect& inRect, Uint32 inFill) 
		{	UGraphics::DrawRect(this, inRect, inFill); }

		void DrawOval(const SRect& inRect, Uint32 inFill) 
		{	UGraphics::DrawOval(this, inRect, inFill); }

		void DrawRoundRect(const SRoundRect& inRect, Uint32 inFill) 
		{	UGraphics::DrawRoundRect(this, inRect, inFill); }

		void DrawPolygon(const SPoint *inPointList, Uint32 inPointCount, Uint32 inFill, Uint32 inOpts = 0) 
		{	UGraphics::DrawPolygon(this, inPointList, inPointCount, inFill, inOpts);}

		void DrawRegion(TRegion inRgn, Uint32 inFill) 
		{	UGraphics::DrawRegion(this, inRgn, inFill); }

		void FillRect(const SRect& inRect) 
		{	UGraphics::FillRect(this, inRect); }

		void FrameRect(const SRect& inRect) 
		{	UGraphics::FrameRect(this, inRect); }

		void SetPixel(const SPoint& inPixel, const SColor& inColor) 
		{	UGraphics::SetPixel(this, inPixel, inColor); }

		void SetPixels(const SPoint *inPointList, Uint32 inPointCount, const SColor& inColor) 
		{	UGraphics::SetPixels(this, inPointList, inPointCount, inColor); }

		void GetPixel(const SPoint& inPixel, SColor& outColor) 
		{	UGraphics::GetPixel(this, inPixel, outColor); }

		void FloodFill(const SPoint& inPixel) 
		{	UGraphics::FloodFill(this, inPixel); }
		
		void ResetFont() 
		{	UGraphics::ResetFont(this); }

		void SetFontName(const Uint8 *inName, const Uint8 *inStyle = nil) 
		{	UGraphics::SetFontName(this, inName, inStyle); }

		void SetFontSize(Uint32 inSize) 
		{	UGraphics::SetFontSize(this, inSize); }

		Uint32 GetFontSize() 
		{	return UGraphics::GetFontSize(this); }

		void SetFontEffect(Uint32 inFlags) 
		{	UGraphics::SetFontEffect(this, inFlags); }

		Uint32 GetFontEffect() 
		{	return UGraphics::GetFontEffect(this); }

		void SetFont(const Uint8 *inName, const Uint8 *inStyle, Uint32 inSize, Uint32 inEffect = 0) 
		{	UGraphics::SetFont(this, inName, inStyle, inSize, inEffect); }

		void SetFont(const SFontDesc& inInfo) 
		{	UGraphics::SetFont(this, inInfo); }

		void SetFont(TFontDesc inInfo) 
		{	UGraphics::SetFont(this, inInfo); }


		void GetFontMetrics(SFontMetrics& outInfo) 
		{	UGraphics::GetFontMetrics(this, outInfo); }

		Uint32 GetFontHeight() 
		{	return UGraphics::GetFontHeight(this); }

		Uint32 GetFontLineHeight() 
		{	return UGraphics::GetFontLineHeight(this); }

		Uint32 GetCharWidth(Uint16 inChar, Uint32 inEncoding = 0) 
		{	return UGraphics::GetCharWidth(this, inChar, inEncoding); }

		Uint32 GetTextWidth(const void *inText, Uint32 inTextSize, Uint32 inEncoding = 0) 
		{	return UGraphics::GetTextWidth(this, inText, inTextSize, inEncoding); }

		Uint32 GetTextLineBreak(const void *inText, Uint32 inLength, Uint32 inMaxWidth) 
		{	return UGraphics::GetTextLineBreak(this, inText, inLength, inMaxWidth); }

		Uint32 WidthToChar(const void *inText, Uint32 inLength, Uint32 inWidth, bool *outLeftSide = nil) 
		{	return UGraphics::WidthToChar(this, inText, inLength, inWidth, outLeftSide); }


		void DrawText(const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags) 
		{	UGraphics::DrawText(this, inBounds, inText, inTextSize, inEncoding, inAlignFlags); }

		void DrawTruncText(const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags) 
		{	UGraphics::DrawTruncText(this, inBounds, inText, inTextSize, inEncoding, inAlignFlags); }

		void DrawTextBox(const SRect& inBounds, const SRect& inUpdateRect, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign) 
		{	UGraphics::DrawTextBox(this, inBounds, inUpdateRect, inText, inTextSize, inEncoding, inAlign); }

		Uint32 GetTextBoxHeight(const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign) 
		{	return UGraphics::GetTextBoxHeight(this, inBounds, inText, inTextSize, inEncoding, inAlign); }

		void DrawTextLines(const SRect& inBounds, const void *inText, Uint32 inEncoding, const Uint32 *inLineOffsets, Uint32 inStartLine, Uint32 inEndLine, Uint32 inLineHeight, Uint32 inAlign) 
		{	UGraphics::DrawTextLines(this, inBounds, inText, inEncoding, inLineOffsets, inStartLine, inEndLine, inLineHeight, inAlign); }


		void GetRawPixels(const SRect& inRect, void *outData, Uint32 inOptions = 0) 
		{	UGraphics::GetRawPixels(this, inRect, outData, inOptions); }

		void SetRawPixels(const SRect& inRect, const void *inData, Uint32 inOptions = 0) 
		{	UGraphics::SetRawPixels(this, inRect, inData, inOptions); }

		void GetImageSize(Uint32& outWidth, Uint32& outHeight) 
		{	UGraphics::GetImageSize(this, outWidth, outHeight); }

		Uint32 BuildColorTable(const SRect& inRect, Uint32 inMethod, Uint32 *outColors, Uint32 inMaxColors) 
		{	return UGraphics::BuildColorTable(this, inRect, inMethod, outColors, inMaxColors); }

		void operator delete(void *p) 
		{	UGraphics::DisposeImage((TImage)p); }
	protected:
		TImageObj() 
		{}
};

/*
 * UFontDesc Object Interface
 */

class TFontDescObj
{
	public:
		TFontDesc Clone() 
		{	return UFontDesc::Clone(this); }
		void SetLock(bool inLock) 
		{	UFontDesc::SetLock(this, inLock); }
		bool IsLocked() 
		{	return UFontDesc::IsLocked(this); }

		void SetFontName(const Uint8 *inName, const Uint8 *inStyle = nil) 
		{	UFontDesc::SetFontName(this, inName, inStyle); }
		bool SetFontSize(Uint32 inSize) 
		{	return UFontDesc::SetFontSize(this, inSize); }
		Uint32 GetFontSize() 
		{	return UFontDesc::GetFontSize(this); }
		void SetFontEffect(Uint32 inFlags) 
		{	UFontDesc::SetFontEffect(this, inFlags); }
		Uint32 GetFontEffect() 
		{	return UFontDesc::GetFontEffect(this); }

		void SetAlign(Uint32 inAlign) 
		{	UFontDesc::SetAlign(this, inAlign); }
		Uint32 GetAlign() 
		{	return UFontDesc::GetAlign(this); }
		void SetColor(const SColor& inColor) 
		{	UFontDesc::SetColor(this, inColor); }
		void GetColor(SColor& outColor) 
		{	UFontDesc::GetColor(this, outColor); }
		void SetCustomColor(const SColor& inColor) 
		{	UFontDesc::SetCustomColor(this, inColor); }
		void GetCustomColor(SColor& outColor) 
		{	UFontDesc::GetCustomColor(this, outColor); }
		void SetCustomValue(Uint32 inVal) 
		{	UFontDesc::SetCustomValue(this, inVal); }
		Uint32 GetCustomValue() 
		{	return UFontDesc::GetCustomValue(this); }

		void operator delete(void *p) 
		{	UFontDesc::Dispose((TFontDesc)p); }
	protected:
		TFontDescObj() 
		{}				// force creation via UFontDesc
};

