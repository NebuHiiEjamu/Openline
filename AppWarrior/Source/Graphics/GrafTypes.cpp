/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "GrafTypes.h"

// geometric constants
const SRect kZeroRect	(0, 0, 0, 0);
const SPoint kZeroPoint	(0, 0);

/*
 * Color Constants
 */

#define RGB24(r, g, b)			(r * 257, g * 257, b * 257)

const SColor color_Gray0		(0x0000);
const SColor color_Gray1		(0x1111);
const SColor color_Gray2		(0x2222);
const SColor color_Gray3		(0x3333);
const SColor color_Gray4		(0x4444);
const SColor color_Gray5		(0x5555);
const SColor color_Gray6		(0x6666);
const SColor color_Gray7		(0x7777);
const SColor color_Gray8		(0x8888);
const SColor color_Gray9		(0x9999);
const SColor color_GrayA		(0xAAAA);
const SColor color_GrayB		(0xBBBB);
const SColor color_GrayC		(0xCCCC);
const SColor color_GrayD		(0xDDDD);
const SColor color_GrayE		(0xEEEE);
const SColor color_GrayF		(0xFFFF);

const SColor color_White		(0xFFFF, 0xFFFF, 0xFFFF);
const SColor color_Black		(0x0000, 0x0000, 0x0000);
const SColor color_Gray			(0x7F7F, 0x7F7F, 0x7F7F);

const SColor color_Red			(0xFFFF, 0x0000, 0x0000);
const SColor color_Green		(0x0000, 0xFFFF, 0x0000);
const SColor color_Blue			(0x0000, 0x0000, 0xFFFF);

const SColor color_Cyan			(0x0000, 0xFFFF, 0xFFFF);
const SColor color_Magenta		(0xFFFF, 0x0000, 0xFFFF);
const SColor color_Yellow		(0xFFFF, 0xFFFF, 0x0000);
const SColor color_Orange		(0xFFFF, 0x7F7F, 0x0000);
const SColor color_Chartreuse   (0x7F7F, 0xFFFF, 0x0000);
const SColor color_Aqua			(0x0000, 0xFFFF, 0x7F7F);
const SColor color_Slate		(0x0000, 0x7F7F, 0xFFFF);
const SColor color_Purple		(0x7F7F, 0x0000, 0xFFFF);
const SColor color_Maroon		(0xFFFF, 0x0000, 0x7F7F);
const SColor color_Brown		(0x5F5F, 0x3F3F, 0x0000);
const SColor color_Pink			(0xFFFF, 0x9F9F, 0xAFAF);
const SColor color_Turquoise	(0x0000, 0xFFFF, 0xDFDF);

const SColor color_cadmium_lemon			RGB24(255,227,3);
const SColor color_cadmium_light_yellow		RGB24(255,176,15);
const SColor color_aureoline_yellow			RGB24(255,168,36);
const SColor color_naples_deep_yellow		RGB24(255,168,18);
const SColor color_cadmium_yellow			RGB24(255,153,18);
const SColor color_cadmium_deep_yellow		RGB24(255,122,10);
const SColor color_cadmium_orange			RGB24(255,97,3);
const SColor color_cadmium_light_red		RGB24(255,3,13);
const SColor color_cadmium_deep_red			RGB24(227,18,48);
const SColor color_geranium_lake			RGB24(227,18,48);
const SColor color_alizarin_crimson			RGB24(227,38,54);
const SColor color_rose_madder				RGB24(227,54,56);
const SColor color_madder_deep_lake			RGB24(227,46,48);
const SColor color_brown_madder				RGB24(219,41,41);
const SColor color_permanent_red_violet		RGB24(219,38,69);
const SColor color_cobalt_deep_violet		RGB24(145,33,158);
const SColor color_ultramarine_violet		RGB24(92,36,110);
const SColor color_ultramarine_blue			RGB24(18,10,143);
const SColor color_cobalt_blue				RGB24(61,89,171);
const SColor color_royal_blue				RGB24(3,145,194);
const SColor color_cerulean_blue			RGB24(5,184,204);
const SColor color_manganese_blue			RGB24(3,168,157);
const SColor color_indigo					RGB24(8,46,84);
const SColor color_turquoise_blue			RGB24(0,199,140);
const SColor color_emerald_green			RGB24(0,201,87);
const SColor color_permanent_green			RGB24(10,201,43);
const SColor color_viridian_light			RGB24(110,255,112);
const SColor color_cobalt_green				RGB24(61,145,64);
const SColor color_cinnabar_green			RGB24(97,179,41);
const SColor color_sap_green				RGB24(48,128,20);
const SColor color_chromium_oxide_green		RGB24(102,128,20);
const SColor color_terre_verte				RGB24(56,94,15);
const SColor color_yellow_ochre				RGB24(227,130,23);
const SColor color_mars_yellow				RGB24(227,112,26);
const SColor color_raw_sienna				RGB24(199,97,20);
const SColor color_mars_orange				RGB24(150,69,20);
const SColor color_gold_ochre				RGB24(199,120,38);
const SColor color_brown_ochre				RGB24(135,66,31);
const SColor color_deep_ochre				RGB24(115,61,36);
const SColor color_burnt_umber				RGB24(138,51,36);
const SColor color_burnt_sienna				RGB24(138,54,15);
const SColor color_flesh					RGB24(255,125,64);
const SColor color_flesh_ochre				RGB24(255,87,33);
const SColor color_english_red				RGB24(212,61,26);
const SColor color_venetian_red				RGB24(212,26,31);
const SColor color_indian_red				RGB24(176,23,31);
const SColor color_raw_umber				RGB24(115,74,18);
const SColor color_greenish_umber			RGB24(3,61,13);
const SColor color_van_dyck_brown			RGB24(94,38,5);
const SColor color_sepia					RGB24(94,38,18);
const SColor color_warm_grey				RGB24(128,128,105);
const SColor color_cold_grey				RGB24(128,138,135);
const SColor color_ivory_black				RGB24(41,36,33);
const SColor color_lamp_black				RGB24(46,71,59);
const SColor color_titanium_white			RGB24(252,255,240);
const SColor color_zinc_white				RGB24(252,242,255);
const SColor color_pale_gold				RGB24(199,171,97);
const SColor color_gold						RGB24(199,156,48);
const SColor color_old_gold					RGB24(128,110,26);
const SColor color_pink_gold				RGB24(255,179,115);
const SColor color_white_gold				RGB24(255,224,143);
const SColor color_yellow_gold				RGB24(189,189,74);
const SColor color_green_gold				RGB24(204,212,112);
const SColor color_platinum					RGB24(212,201,143);
const SColor color_silver					RGB24(207,209,179);
const SColor color_antique_silver			RGB24(135,133,120);
const SColor color_chrome					RGB24(181,209,188);
const SColor color_steel					RGB24(140,158,150);
const SColor color_copper					RGB24(247,153,71);
const SColor color_antique_copper			RGB24(176,84,15);
const SColor color_oxidized_copper			RGB24(128,255,176);
const SColor color_bronze					RGB24(140,115,36);
const SColor color_brass					RGB24(176,161,59);
const SColor color_iron						RGB24(140,158,150);
const SColor color_rusted_iron				RGB24(171,59,20);
const SColor color_lead						RGB24(64,54,43);
const SColor color_fluorescent_pink			RGB24(255,214,245);
const SColor color_fluorescent_green		RGB24(186,255,196);
const SColor color_fluorescent_blue			RGB24(214,240,255);
const SColor color_incadescent_high			RGB24(255,247,161);
const SColor color_incadescent_low			RGB24(255,232,107);
const SColor color_moonlight				RGB24(191,207,255);
const SColor color_sodium					RGB24(255,173,31);
const SColor color_daylight					RGB24(255,245,235);
const SColor color_dawn						RGB24(255,176,156);
const SColor color_afternoon				RGB24(255,181,94);
const SColor color_dusk						RGB24(255,163,86);
const SColor color_purple_gray				RGB24(208,163,196);

/* -------------------------------------------------------------------------- */

// if the Point is not inside <r> then move it to the nearest edge
void SPoint::Constrain(const SRect& r)
{
	if (v < r.top)		v = r.top;
	if (v > r.bottom)	v = r.bottom;
	if (h < r.left)		h = r.left;
	if (h > r.right)	h = r.right;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// is the area of this rectangle greator than the area of the specified rectangle?
bool SRect::operator>(const SRect& r) const
{
	return ((bottom - top) * (right - left)) > ((r.bottom - r.top) * (r.right - left));
}

// is the area of this rectangle less than the area of the specified rectangle?
bool SRect::operator<(const SRect& r) const
{
	return ((bottom - top) * (right - left)) < ((r.bottom - r.top) * (r.right - left));
}

// is the area of this rectangle greator than or equal to the area of the specified rectangle?
bool SRect::operator>=(const SRect& r) const
{
	return ((bottom - top) * (right - left)) >= ((r.bottom - r.top) * (r.right - left));
}

// is the area of this rectangle less than or equal to the area of the specified rectangle?
bool SRect::operator<=(const SRect& r) const
{
	return ((bottom - top) * (right - left)) <= ((r.bottom - r.top) * (r.right - left));
}

// intersection
SRect SRect::operator&(const SRect& r) const
{
	SRect result;
	
	result.top = max(top, r.top);
	result.left = max(left, r.left);
	result.bottom = min(bottom, r.bottom);
	result.right = min(right, r.right);
	
	if (!result.IsValid())
		result.top = result.left = result.bottom = result.right = 0;
	
	return result;
}

// union
SRect SRect::operator|(const SRect& r) const
{
	SRect result;
	
	result.top = min(top, r.top);
	result.left = min(left, r.left);
	result.bottom = max(bottom, r.bottom);
	result.right = max(right, r.right);
	
	return result;
}

// union
SRect& SRect::operator|=(const SRect& r)
{
	if (r.right > r.left && r.bottom > r.top)				// if r is not empty
	{
		if (right <= left || bottom <= top)					// if this rect is empty
		{
			top = r.top;
			left = r.left;
			bottom = r.bottom;
			right = r.right;
		}
		else
		{
			if (top >= r.top)		top = r.top;
			if (left >= r.left)		left = r.left;
			if (bottom <= r.bottom)	bottom = r.bottom;
			if (right <= r.right)	right = r.right;
		}
	}
	return *this;
}

// does this rectangle intersect with the specified rectangle?
bool SRect::Intersects(const SRect& inRect) const
{
	return (max(left, inRect.left) < min(right, inRect.right)) && (max(top, inRect.top) < min(bottom, inRect.bottom));	
}

// returns true if the rectangles intersect
bool SRect::GetIntersection(const SRect& inRect, SRect& outSect) const
{
	outSect.left = max(left, inRect.left);
	outSect.top = max(top, inRect.top);
	outSect.right = min(right, inRect.right);
	outSect.bottom = min(bottom, inRect.bottom);
	
	if ((outSect.left < outSect.right) && (outSect.top < outSect.bottom))
		return true;

	outSect.SetEmpty();
	return false;
}

void SRect::GetUnion(const SRect& inRect, SRect& outUnion) const
{
	outUnion.top = min(top, inRect.top);
	outUnion.left = min(left, inRect.left);
	outUnion.bottom = max(bottom, inRect.bottom);
	outUnion.right = max(right, inRect.right);
}

// make it such that this rectangle is completely enclosed by the specified rectangle
void SRect::Constrain(const SRect& r)
{
#if DEBUG
	if (r.IsInvalid()) DebugBreak("SRect::Constrain - rectangle is invalid");
#endif
	
	if (top < r.top)			top = r.top;
	else if (top > r.bottom)	top = r.bottom;

	if (left < r.left)			left = r.left;
	else if (left > r.right)	left = r.right;

	if (bottom > r.bottom)		bottom = r.bottom;
	else if (bottom < r.top)	bottom = r.top;

	if (right > r.right)		right = r.right;
	else if (right < r.left)	right = r.left;
}

void SRect::Validate()
{
	Int32 x;
	
	if (top > bottom)
	{
		x = top;
		top = bottom;
		bottom = x;
	}
	
	if (left > right)
	{
		x = left;
		left = right;
		right = x;
	}
}

void SRect::Center(const SRect& base)
{
	Int32 n;
	
	n = right - left;
	left = (base.left + base.right - n) / 2;
	right = left + n;

	n = bottom - top;
	top = (base.bottom + base.top - n) / 2;
	bottom = top + n;
}

void SRect::CenterHoriz(const SRect& base)
{
	Int32 n = right - left;
	//left = base.left + ((base.right - base.left - n) / 2);
	left = (base.left + base.right - n) / 2;
	right = left + n;
}

void SRect::CenterVert(const SRect& base)
{
	Int32 n = bottom - top;
	//top = base.top + ((base.bottom - base.top - n) / 2);
	top = (base.bottom + base.top - n) / 2;
	bottom = top + n;
}

void SRect::MoveTo(Int32 inHorizLoc, Int32 inVertLoc)
{
	Int32 h = bottom - top;
	Int32 w = right - left;
	left = inHorizLoc;
	right = left + w;
	top = inVertLoc;
	bottom = top + h;
}

void SRect::Align(Uint16 inAlign, const SRect& inBase)
{
	Int32 n;
	
	// align horizontally
	n = right - left;
	if (inAlign & align_Left)
	{
		left = inBase.left;
		right = left + n;
	}
	else if (inAlign & align_Right)
	{
		right = inBase.right;
		left = right - n;
	}
	else if (inAlign & align_CenterHoriz)
	{
		left = (inBase.left + inBase.right - n) / 2;
		right = left + n;
	}
	else if (inAlign & align_InsideHoriz)
	{
		if (left < inBase.left)
		{
			left = inBase.left;
			right = left + n;
		}
		else if (right > inBase.right)
		{
			right = inBase.right;
			left = right - n;
			
			// make sure left side is always inside
			if (left < inBase.left)
			{
				left = inBase.left;
				right = left + n;
			}
		}
	}
	
	// align vertically
	n = bottom - top;
	if (inAlign & align_Top)
	{
		top = inBase.top;
		bottom = top + n;
	}
	else if (inAlign & align_Bottom)
	{
		bottom = inBase.bottom;
		top = bottom - n;
	}
	else if (inAlign & align_CenterVert)
	{
		top = (inBase.bottom + inBase.top - n) / 2;
		bottom = top + n;
	}
	else if (inAlign & align_InsideVert)
	{
		if (top < inBase.top)
		{
			top = inBase.top;
			bottom = top + n;
		}
		else if (bottom > inBase.bottom)
		{
			bottom = inBase.bottom;
			top = bottom - n;
			
			// make sure top is always inside
			if (top < inBase.top)
			{
				top = inBase.top;
				bottom = top + n;
			}
		}
	}
}

// rotate 90 degrees
void SRect::Rotate()
{
	Int32 n;
	
	n = left;
	left = top;
	top = n;
	
	n = right;
	right = bottom;
	bottom = n;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

bool SColor::operator>(const SColor& r) const
{
	return ((Uint32)red + green + blue) > ((Uint32)r.red + r.green + r.blue);
}

bool SColor::operator<(const SColor& r) const
{
	return ((Uint32)red + green + blue) < ((Uint32)r.red + r.green + r.blue);
}

bool SColor::operator>=(const SColor& r) const
{
	return ((Uint32)red + green + blue) >= ((Uint32)r.red + r.green + r.blue);
}

bool SColor::operator<=(const SColor& r) const
{
	return ((Uint32)red + green + blue) <= ((Uint32)r.red + r.green + r.blue);
}

void SColor::Lighten(Uint16 inAmount)
{
	if ((0xFFFF - red) < inAmount)
		red = 0xFFFF;
	else
		red += inAmount;

	if ((0xFFFF - green) < inAmount)
		green = 0xFFFF;
	else
		green += inAmount;

	if ((0xFFFF - blue) < inAmount)
		blue = 0xFFFF;
	else
		blue += inAmount;
}

void SColor::Darken(Uint16 inAmount)
{
	if (red < inAmount)
		red = 0;
	else
		red -= inAmount;
	
	if (green < inAmount)
		green = 0;
	else
		green -= inAmount;
	
	if (blue < inAmount)
		blue = 0;
	else
		blue -= inAmount;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

SMatrix3::SMatrix3(const SMatrix3& inMat)
{
	map[0][0] = inMat.map[0][0];	map[0][1] = inMat.map[0][1];	map[0][2] = inMat.map[0][2];
	map[1][0] = inMat.map[1][0];	map[1][1] = inMat.map[1][1];	map[1][2] = inMat.map[1][2];
	map[2][0] = inMat.map[2][0];	map[2][1] = inMat.map[2][1];	map[2][2] = inMat.map[2][2];	
}

SMatrix3& SMatrix3::operator +=(const SMatrix3& inMat)
{
	map[0][0] += inMat.map[0][0];	map[0][1] += inMat.map[0][1];	map[0][2] += inMat.map[0][2];
	map[1][0] += inMat.map[1][0];	map[1][1] += inMat.map[1][1];	map[1][2] += inMat.map[1][2];
	map[2][0] += inMat.map[2][0];	map[2][1] += inMat.map[2][1];	map[2][2] += inMat.map[2][2];	
	return *this;
}

SMatrix3& SMatrix3::operator -=(const SMatrix3& inMat)
{
	map[0][0] -= inMat.map[0][0];	map[0][1] -= inMat.map[0][1];	map[0][2] -= inMat.map[0][2];
	map[1][0] -= inMat.map[1][0];	map[1][1] -= inMat.map[1][1];	map[1][2] -= inMat.map[1][2];
	map[2][0] -= inMat.map[2][0];	map[2][1] -= inMat.map[2][1];	map[2][2] -= inMat.map[2][2];	
	return *this;
}

SMatrix3& SMatrix3::operator *=(const SMatrix3& inMat)
{
	const fast_float (*mul)[3] = inMat.map;
	
	fast_float m00 = map[0][0];
	fast_float m01 = map[0][1];
	fast_float m02 = map[0][2];
	fast_float m10 = map[1][0];
	fast_float m11 = map[1][1];
	fast_float m12 = map[1][2];
	fast_float m20 = map[2][0];
	fast_float m21 = map[2][1];
	fast_float m22 = map[2][2];

	map[0][0] = (m00 * mul[0][0]) + (m01 * mul[1][0]) + (m02 * mul[2][0]);
	map[0][1] = (m00 * mul[0][1]) + (m01 * mul[1][1]) + (m02 * mul[2][1]);
	map[0][2] = (m00 * mul[0][2]) + (m01 * mul[1][2]) + (m02 * mul[2][2]);
	map[1][0] = (m10 * mul[0][0]) + (m11 * mul[1][0]) + (m12 * mul[2][0]);
	map[1][1] = (m10 * mul[0][1]) + (m11 * mul[1][1]) + (m12 * mul[2][1]);
	map[1][2] = (m10 * mul[0][2]) + (m11 * mul[1][2]) + (m12 * mul[2][2]);
	map[2][0] = (m20 * mul[0][0]) + (m21 * mul[1][0]) + (m22 * mul[2][0]);
	map[2][1] = (m20 * mul[0][1]) + (m21 * mul[1][1]) + (m22 * mul[2][1]);
	map[2][2] = (m20 * mul[0][2]) + (m21 * mul[1][2]) + (m22 * mul[2][2]);
	
	return *this;
}

SMatrix3& SMatrix3::operator *=(fast_float d)
{
	map[0][0] *= d;	map[0][1] *= d;	map[0][2] *= d;
	map[1][0] *= d;	map[1][1] *= d;	map[1][2] *= d;
	map[2][0] *= d;	map[2][1] *= d;	map[2][2] *= d;	
	return *this;
}

SMatrix3& SMatrix3::operator /=(fast_float d)
{
	fast_float di = 1 / d;	// multiply is faster than divide, so we eliminate a stack of divides this way
	map[0][0] *= di;	map[0][1] *= di;	map[0][2] *= di;
	map[1][0] *= di;	map[1][1] *= di;	map[1][2] *= di;
	map[2][0] *= di;	map[2][1] *= di;	map[2][2] *= di;	
	return *this;
}

void SMatrix3::SetIdentity()
{
	map[0][0] = 1;	map[0][1] = 0;	map[0][2] = 0;
	map[1][0] = 0;	map[1][1] = 1;	map[1][2] = 0;
	map[2][0] = 0;	map[2][1] = 0;	map[2][2] = 1;
}

void SMatrix3::Scale(fast_float inHorizFactor, fast_float inVertFactor)
{
	map[0][0] *= inHorizFactor;
	map[0][1] *= inVertFactor;
	
	map[1][0] *= inHorizFactor;
	map[1][1] *= inVertFactor;
	
	map[2][0] *= inHorizFactor;
	map[2][1] *= inVertFactor;
}

void SMatrix3::Rotate(fast_float inAngle)
{
	fast_float sinAngle = UMath::Sine(inAngle);
	fast_float cosAngle = UMath::Cosine(inAngle);

	fast_float m00 = map[0][0];
	fast_float m01 = map[0][1];
	fast_float m10 = map[1][0];
	fast_float m11 = map[1][1];
	fast_float m20 = map[2][0];
	fast_float m21 = map[2][1];

	map[0][0] = (m00 * cosAngle) - (m01 * sinAngle);
	map[0][1] = (m01 * cosAngle) + (m00 * sinAngle);
	map[1][0] = (m10 * cosAngle) - (m11 * sinAngle);
	map[1][1] = (m11 * cosAngle) + (m10 * sinAngle);
	map[2][0] = (m20 * cosAngle) - (m21 * sinAngle);
	map[2][1] = (m21 * cosAngle) + (m20 * sinAngle);
}

void SMatrix3::TransformPoint(SFloatPoint& ioPt) const
{
	fast_float x = ioPt.x;
	fast_float y = ioPt.y;
	
	ioPt.x = (x * map[0][0]) + (y * map[1][0]) + map[2][0];
	ioPt.y = (x * map[0][1]) + (y * map[1][1]) + map[2][1];
}

void SMatrix3::TransformPoints(SFloatPoint *ioPts, Uint32 inCount) const
{
	fast_float map00 = map[0][0];
	fast_float map01 = map[0][1];
	fast_float map10 = map[1][0];
	fast_float map11 = map[1][1];
	fast_float map20 = map[2][0];
	fast_float map21 = map[2][1];

	while (inCount--)
	{
		fast_float x = ioPts->x;
		fast_float y = ioPts->y;
		
		ioPts->x = (x * map00) + (y * map10) + map20;
		ioPts->y = (x * map01) + (y * map11) + map21;
	
		ioPts++;
	}
}

void SMatrix3::MakeRotateMatrix(fast_float inAngle, SMatrix3& outMat)
{
	fast_float sinAngle = UMath::Sine(inAngle);
	fast_float cosAngle = UMath::Cosine(inAngle);
	
	outMat[0][0] = cosAngle;	outMat[0][1] = sinAngle;	outMat[0][2] = 0;
	outMat[1][0] = -sinAngle;	outMat[1][1] = cosAngle;	outMat[1][2] = 0;
	outMat[2][0] = 0;			outMat[2][1] = 0;			outMat[2][2] = 1;
}

void SMatrix3::MakeScaleMatrix(fast_float x, fast_float y, SMatrix3& outMat)
{
	outMat[0][0] = x;	outMat[0][1] = 0;	outMat[0][2] = 0;
	outMat[1][0] = 0;	outMat[1][1] = y;	outMat[1][2] = 0;
	outMat[2][0] = 0;	outMat[2][1] = 0;	outMat[2][2] = 1;
}

void SMatrix3::MakeMoveMatrix(fast_float x, fast_float y, SMatrix3& outMat)
{
	outMat[0][0] = 1;	outMat[0][1] = 0;	outMat[0][2] = 0;
	outMat[1][0] = 0;	outMat[1][1] = 1;	outMat[1][2] = 0;
	outMat[2][0] = x;	outMat[2][1] = y;	outMat[2][2] = 1;
}





