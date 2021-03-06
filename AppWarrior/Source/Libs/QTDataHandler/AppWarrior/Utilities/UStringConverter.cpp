/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// Utility class that converts between Unicode characters and ASCII

#include "AW.h"
#include "UStringConverter.h"

HL_Begin_Namespace_BigRedH

using namespace std;


static const int kAsciiTableSize = 128;
static const int kMaxPascalStrLen = 256;

// This table represents the mapping to the Unicode equivalent
// of the extended ASCII characters (128->255) on the Mac. It starts
// from Mac ASCII character 128, and goes to character 255.
//
// TO DO - examine how byte ordering affects this table, if at all.
static const UInt16 MacAsciiToUnicodeTable[ kAsciiTableSize ] =
{
	0x00C4, // Big A with Diaeresis
	0x00C5, // Big A with ring
	0x00C7,	// Big C with cedille
	0x00C9, // Big E with acute accent
	0x00D1, // Big N with tilde
	0x00D6, // Big O with Diaeresis
	0x00DC, // Big U with Diaeresis
	0x00E1, // Small A with acute accent
	0x00E0, // Small A with grave accent
	0x00E2, // Small A with cap
	0x00E4,	// Small A with Diaeresis
	0x00E3, // Small A with tilde
	0x00E5,	// Small A with ring
	0x00E7,	// Small C with cedille
	0x00E9,	// Small E with acute accent
	0x00E8, // Small E with grave accent
	0x00EA,	// Small E with cap
	0x00EB,	// Small E with diaeresis
	0x00ED, // Small I with acute accent
	0x00EC, // Small I with grave
	0x00EE, // Small I with cap
	0x00EF,	// Small I with diaeresis
	0x00F1, // Small N with tilde
	0x00F3, // Small O with acute accent
	0x00F2, // Small O with grave
	0x00F4, // Small O with cap
	0x00F6, // Small O with Diaeresis
	0x00F5, // Small O with tilde
	0x00FA, // Small U with acute accent
	0x00F9, // Small U with grave accent
	0x00FB, // Small U with cap
	0x00FC, // Small U with Diaeresis
	0x2020, // Single Dagger
	0x00B0, // Degree sign
	0x00A2, // Cent symbol
	0x00A3, // Pound symbol
	0x00A7, // Section symbol
	0x2022, // Bullet
	0x00B6, // Paragraph symbol (pilcrow)
	0x00DF, // Small letter sharp S
	0x00AE, // Registered sign
	0x00A9, // Copyright sign
	0x2122, // Trademark sign
	0x2032, // Prime
	0x00A8, // Diaeresis
	0x2260, // Not equals sign
	0x00C6, // Capital letter AE
	0x00D8, // Capital O with stroke
	0x221E, // Lemniscate (infinity sign)
	0x00B1, // Plus-minus
	0x2264,	// Less-than-or-equals
	0x2265, // Greater-than-or-equals
	0x00A5, // Yen symbol
	0x00B5, // Metric "micro" prefix
	0x2202, // Partial derivative symbol
	0x2211, // N-ary summation operator
	0x220F, // N-ary product operator
	0x03C0, // Small letter pi
	0x222B, // Integral symbol
	0x00AA, // Feminine ordinal indicator
	0x00BA, // Masculine ordinal indicator
	0x03A9, // Capital letter omega
	0x00E6, // Small letter AE
	0x00F8, // Small O with stroke
	0x00BF, // Inverted question mark
	0x00A1, // Inverted exclamation mark
	0x00AC, // Negation symbol
	0x221A, // Square root symbol
	0x0192, // Curly F (function symbol)
	0x2248, // Approximately equal to
	0x25B3, // White up-pointing triangle
	0x00AB, // Left double-angle quotation mark
	0x00BB, // Right double-angle quotation mark
	0x2026, // Ellipsis
	0x0020, // Space (This is an assumption since the character is non-printing)
	0x00C0, // Capital A with grave
	0x00C3, // Capital A with tilde
	0x00D5, // Capital O with tilde
	0x0152, // Capital letter OE
	0x0153, // Small letter OE
	0x2013, // En dash (short hyphen)
	0x2014, // Em dash (long hyphen)
	0x201C, // Left double quotation mark
	0x201D, // Right double quotation mark
	0x2018,	// Left single quotation mark
	0x2019, // Right single quotation mark
	0x00F7, // Division symbol (bar and 2 dots)
	0x25C7, // White diamond
	0x00FF, // Small Y with Diaeresis
	0x0178, // Capital Y with Diaeresis
	0x2215, // Division slash
	0x20AC,	// Euro symbol
	0x2039, // Single left-pointing angle quotation mark
	0x203A, // Single right-pointing angle quotation mark
	0x0000, // FIXME - I can't be found in Unicode database - Small letter FI
	0x0000, // FIXME - I can't be found in Unicode database - Small letter FL
	0x2021, // Double dagger
	0x2219, // Middle dot
	0x201A,	// Comma (?)
	0x201E, // Right low double comma quotation mark
	0x2030, // Per Mille
	0x00C2, // Capital A with cap
	0x00CA, // Capital E with cap
	0x00C1, // Capital A with acute accent
	0x00CB, // Capital E with diaeresis
	0x00C8, // Capital E with grave
	0x00CD, // Capital I with acute
	0x00CE, // Capital I with cap
	0x00CF, // Capital I with diaeresis
	0x00CC, // Capital I with grave
	0x00D3, // Capital O with acute accent
	0x00D4, // Capital O with cap
	0x2620, // Skull & Crossbones (replaces an Apple symbol)
	0x00D2, // Capital O with grave
	0x00DA, // Capital U with acute
	0x00DB, // Capital U with cap
	0x00D9, // Capital U with grave
	0x257B, // Box drawings vertical down (double check this)
	0x005E, // Spacing Circumflex
	0x007E, // Tilde
	0x203E, // Overline
	0x02D8, // Breve
	0x02D9, // Dot above
	0x02DA, // Ring above
	0x00B8, // Cedilla
	0x2033, // Double prime
	0x02DB, // Ogonek
	0x02C7, // Caron
};

// This table represents the mapping to the Unicode equivalent 
// of the extended ASCII characters (128->255) on the PC. It starts
// from PC ASCII character 128, and goes to character 255. 
static const UInt16 PCAsciiToUnicodeTable[ kAsciiTableSize ] =
{	
	0x20AC,	// Euro symbol
	0x0000,	// FIXME
	0x201A,	// Comma (?)
	0x0192, // Curly F (function symbol)
	0x201E, // Double comma (?)
	0x2026, // Ellipsis
	0x2020, // Dagger
	0x2021, // Double Dagger
	0x02C6, // Caret
	0x2030, // Per Mille
	0x0160, // Capital S with caron
	0x2039, // Left Angle
	0x0152, // Capital letter OE
	0x0000, // Not mapped
	0x017D, // Capital Z with caron
	0x0000, // Not mapped

	0x0000, // Not mapped
	0x2018, // `
	0x2019, // '
	0x201C, // ``
	0x201D, // ''
	0x2022, // Bullet
	0x2013, // Short hyphen
	0x2014, // Long hyphen
	0x02DC,	// Tilde
	0x2122, // Trademark symbol
	0x0161, // Small S with caron
	0x203A, // Right Angle
	0x0153, // Small letter OE
	0x0000, // Not mapped
	0x017E, // Small Z with caron
	0x0178, // Capital Y with Diaeresis
	0x00A0, // Non-breaking space
	0x00A1, // Small I
	0x00A2, // Cent symbol
	0x00A3, // Pound (currency) symbol
	0x00A4, // Currency symbol
	0x00A5, // Yen symbol
	0x00A6, // Pipe symbol
	0x00A7, // Section symbol
	0x00A8, // Umlaut
	0x00A9, // Copyright symbol
	0x00AA, // Feminine ordinal indicator
	0x00AB, // Left double-angle quotation
	0x00AC, // Negation symbol
	0x00AD, // Hyphen
	0x00AE, 
	0x00AF,
	
	0x00B0,
	0x00B1,
	0x00B2,
	0x00B3,
	0x00B4,
	0x00B5,
	0x00B6,
	0x00B7,
	0x00B8,
	0x00B9,
	0x00BA,
	0x00BB,
	0x00BC,
	0x00BD,
	0x00BE,
	0x00BF,
	
	0x00C0,
	0x00C1,
	0x00C2,
	0x00C3,
	0x00C4,
	0x00C5,
	0x00C6,
	0x00C7,
	0x00C8,
	0x00C9,
	0x00CA,
	0x00CB,
	0x00CC,
	0x00CD,
	0x00CE,
	0x00CF,

	0x00D0,
	0x00D1,
	0x00D2,
	0x00D3,
	0x00D4,
	0x00D5,
	0x00D6,
	0x00D7,
	0x00D8,	// Nought
	0x00D9,
	0x00DA,
	0x00DB,
	0x00DC,
	0x00DD,
	0x00DE,
	0x00DF,

	0x00E0,
	0x00E1,
	0x00E2,
	0x00E3,
	0x00E4,
	0x00E5,
	0x00E6,
	0x00E7,
	0x00E8,
	0x00E9,
	0x00EA,
	0x00EB,
	0x00EC,
	0x00ED,
	0x00EE,
	0x00EF,

	0x00F0,
	0x00F1,
	0x00F2,
	0x00F3,
	0x00F4,
	0x00F5,
	0x00F6,
	0x00F7,
	0x00F8,
	0x00F9,
	0x00FA,
	0x00FB,
	0x00FC,
	0x00FD,
	0x00FE,
	0x00FF
};

// This table represents a mapping from Unicode to the U.S.
// English extended ASCII codepage on the PC. Unicode characters 
// in the range 0x0000 -> 0x007F map directly to their ASCII
// equivalents.
//
// * Note that the second row contains 16 bit values, but the M.S.B.
// * is always 0 to pad out each array element, and can safely be
// * discarded in order to get the desired single byte ASCII character.
static const UInt16 UnicodeToPCAsciiTable[2][ kAsciiTableSize ] =
{
	// These are the Unicode values we map to 8 bit characters
	{
		0x0000,	// NULL
		0x0000, // FIXME
		0x0000, // FIXME
		0x0000, // FIXME
		0x0000, // FIXME
		0x00A0,
		0x00A1,
		0x00A2,
		0x00A3,
		0x00A4,
		0x00A5,
		0x00A6,
		0x00A7,
		0x00A8,
		0x00A9,
		0x00AA,
		0x00AB,
		0x00AC,
		0x00AD,
		0x00AE,
		0x00AF,
		0x00B0,
		0x00B1,
		0x00B2,
		0x00B3,
		0x00B4,
		0x00B5,
		0x00B6,
		0x00B7,
		0x00B8,
		0x00B9,
		0x00BA,
		0x00BB,
		0x00BC,
		0x00BD,
		0x00BE,
		0x00BF,
		0x00C0,
		0x00C1,
		0x00C2,
		0x00C3,
		0x00C4,
		0x00C5,
		0x00C6,
		0x00C7,
		0x00C8,
		0x00C9,
		0x00CA,
		0x00CB,
		0x00CC,
		0x00CD,
		0x00CE,
		0x00CF,
		0x00D0,
		0x00D1,
		0x00D2,
		0x00D3,
		0x00D4,
		0x00D5,
		0x00D6,
		0x00D7,
		0x00D8,	// Nought
		0x00D9,
		0x00DA,
		0x00DB,
		0x00DC,
		0x00DD,
		0x00DE,
		0x00DF,
		0x00E0,
		0x00E1,
		0x00E2,
		0x00E3,
		0x00E4,
		0x00E5,
		0x00E6,
		0x00E7,
		0x00E8,
		0x00E9,
		0x00EA,
		0x00EB,
		0x00EC,
		0x00ED,
		0x00EE,
		0x00EF,
		0x00F0,
		0x00F1,
		0x00F2,
		0x00F3,
		0x00F4,
		0x00F5,
		0x00F6,
		0x00F7,
		0x00F8,
		0x00F9,
		0x00FA,
		0x00FB,
		0x00FC,
		0x00FD,
		0x00FE,
		0x00FF,
		0x0152,
		0x0153,
		0x0160,
		0x0161,
		0x0178,
		0x017D,
		0x017E,
		0x0192,
		0x02C6,
		0x02DC,
		0x2013,
		0x2014,
		0x2018,
		0x2019,
		0x201A,
		0x201C,
		0x201D,
		0x201E,
		0x2020,
		0x2021,
		0x2022,	// Bullet
		0x2026,
		0x2030,
		0x2039,
		0x203A,
		0x20AC,
		0x2122
	},

	// These are the 8 bit characters the Unicode is mapped to. We
	// are assuming that the U.S. English code page is in use.
	{
		0x00, // NULL
		0x00, // FIXME
		0x00, // FIXME
		0x00, // FIXME
		0x00, // FIXME
		0xA0,
		0xA1,
		0xA2,
		0xA3,
		0xA4,
		0xA5,
		0xA6,
		0xA7,
		0xA8,
		0xA9,
		0xAA,
		0xAB,
		0xAC,
		0xAD,
		0xAE,
		0xAF,
		0xB0,
		0xB1,
		0xB2,
		0xB3,
		0xB4,
		0xB5,
		0xB6,
		0xB7,
		0xB8,
		0xB9,
		0xBA,
		0xBB,
		0xBC,
		0xBD,
		0xBE,
		0xBF,
		0xC0,
		0xC1,
		0xC2,
		0xC3,
		0xC4,
		0xC5,
		0xC6,
		0xC7,
		0xC8,
		0xC9,
		0xCA,
		0xCB,
		0xCC,
		0xCD,
		0xCE,
		0xCF,
		0xD0,
		0xD1,
		0xD2,
		0xD3,
		0xD4,
		0xD5,
		0xD6,
		0xD7,
		0xD8,
		0xD9,
		0xDA,
		0xDB,
		0xDC,
		0xDD,
		0xDE,
		0xDF,
		0xE0,
		0xE1,
		0xE2,
		0xE3,
		0xE4,
		0xE5,
		0xE6,
		0xE7,
		0xE8,
		0xE9,
		0xEA,
		0xEB,
		0xEC,
		0xED,
		0xEE,
		0xEF,
		0xF0,
		0xF1,
		0xF2,
		0xF3,
		0xF4,
		0xF5,
		0xF6,
		0xF7,
		0xF8,
		0xF9,
		0xFA,
		0xFB,
		0xFC,
		0xFD,
		0xFE,
		0xFF,
		0x8C,	// Big letter OE
		0x9C,	// Small letter oe
		0x8A,	// Capital S with caron
		0x9A,	// Small s with caron
		0x9F,	// Capital Y with Diaeresis
		0x8E,	// Capital Z with caron
		0x9E,	// Small Z with caron
		0x83,	// Small F with hook (function F)
		0x88,	// Caret
		0x98,	// Tilde
		0x96,	// Short bar
		0x97,	// Long bar
		0x91,	// `
		0x92,	// '
		0x82,	// ,
		0x93,	// ``
		0x94,	// ''
		0x84,	// ,,
		0x86,	// Single dagger
		0x87,	// Double dagger
		0x95,	// Bullet
		0x85,	// Ellipsis
		0x89,	// Per Mille
		0x8B,	// Left angle
		0x9B,	// Right angle
		0x80,	// Euro symbol
		0x99	// Trademark symbol
	}
};

// TO DO - Investigate any endianess dependent issues for this table
static UInt16 UnicodeToMacAsciiTable[2][ kAsciiTableSize ] =
{
	// These are the Unicode values we map to 8 bit characters
	{
		0x0000, // FIXME - Small letter FI
		0x0000, // FIXME - Small letter FL
		0x0020, // Space (This is an assumption since the character is non-printing)
		0x005E, // Spacing Circumflex
		0x007E, // Tilde
		0x00A1, // Inverted exclamation mark
		0x00A2, // Cent symbol
		0x00A3, // Pound symbol
		0x00A5, // Yen symbol
		0x00A7, // Section symbol
		0x00A8, // Diaeresis
		0x00A9, // Copyright sign
		0x00AA, // Feminine ordinal indicator
		0x00AB, // Left double-angle quotation mark
		0x00AC, // Negation symbol
		0x00AE, // Registered sign
		0x00B0, // Degree sign
		0x00B1, // Plus-minus
		0x00B5, // Metric "micro" prefix
		0x00B6, // Paragraph symbol (pilcrow)
		0x00B8, // Cedilla
		0x00BA, // Masculine ordinal indicator
		0x00BB, // Right double-angle quotation mark
		0x00BF, // Inverted question mark
		0x00C0, // Capital A with grave
		0x00C1, // Capital A with acute accent
		0x00C2, // Capital A with cap
		0x00C3, // Capital A with tilde
		0x00C4, // Capital A with diaeresis
		0x00C5, // Capital A with ring
		0x00C6, // Capital letter AE
		0x00C7,	// Big C with cedille
		0x00C8, // Capital E with grave
		0x00C9, // Big E with acute accent
		0x00CA, // Capital E with cap
		0x00CB, // Capital E with diaeresis
		0x00CC, // Capital I with grave
		0x00CD, // Capital I with acute accent
		0x00CE, // Capital I with cap
		0x00CF, // Capital I with diaeresis
		0x00D1, // Big N with tilde
		0x00D2, // Capital O with grave
		0x00D3, // Capital O with acute accent
		0x00D4, // Capital O with cap
		0x00D5, // Capital O with tilde
		0x00D6, // Capital O with diaeresis
		0x00D8, // Capital O with stroke
		0x00D9, // Capital U with grave
		0x00DA, // Capital U with acute
		0x00DB, // Capital U with cap
		0x00DC, // Big U with diaeresis
		0x00DF, // Small letter sharp S
		0x00E0, // Small A with grave accent
		0x00E1, // Small A with acute accent
		0x00E2, // Small A with cap
		0x00E3, // Small A with tilde
		0x00E4,	// Small A with diaeresis
		0x00E5,	// Small A with ring
		0x00E6, // Small letter AE
		0x00E7,	// Small C with cedille
		0x00E8, // Small E with grave accent
		0x00E9,	// Small E with acute accent
		0x00EA,	// Small E with cap
		0x00EB,	// Small E with diaeresis
		0x00EC, // Small I with grave
		0x00ED, // Small I with acute accent
		0x00EE, // Small I with cap
		0x00EF,	// Small I with diaeresis
		0x00F1, // Small N with tilde
		0x00F2, // Small O with grave
		0x00F3, // Small O with acute accent
		0x00F4, // Small O with cap
		0x00F5, // Small O with tilde
		0x00F6, // Small O with diaeresis
		0x00F7, // Division symbol (bar and 2 dots)
		0x00F8, // Small O with stroke
		0x00F9, // Small U with grave accent
		0x00FA, // Small U with acute accent
		0x00FB, // Small U with cap
		0x00FC, // Small U with diaeresis
		0x00FF, // Small Y with diaeresis
		0x0152, // Capital letter OE
		0x0153, // Small letter OE
		0x0178, // Capital Y with diaeresis
		0x0192, // Curly F (function symbol)
		0x02C7, // Caron
		0x02D8, // Breve
		0x02D9, // Dot above
		0x02DA, // Ring above
		0x02DB, // Ogonek
		0x03A9, // Capital letter omega
		0x03C0, // Small letter pi
		0x2013, // En dash (short hyphen)
		0x2014, // Em dash (long hyphen)
		0x2018,	// Left single quotation mark
		0x2019, // Right single quotation mark
		0x201A,	// Comma (?)
		0x201C, // Left double quotation mark
		0x201D, // Right double quotation mark
		0x201E, // Right low double comma quotation mark
		0x2020, // Single Dagger
		0x2021, // Double dagger
		0x2022, // Bullet
		0x2026, // Ellipsis
		0x2030, // Per Mille
		0x2032, // Prime
		0x2033, // Double prime
		0x2039, // Single left-pointing angle quotation mark
		0x203A, // Single right-pointing angle quotation mark
		0x203E, // Overline
		0x20AC,	// Euro symbol
		0x2122, // Trademark sign
		0x2202, // Partial derivative symbol
		0x220F, // N-ary product operator
		0x2211, // N-ary summation operator
		0x2215, // Division slash
		0x2219, // Middle dot
		0x221A, // Square root symbol
		0x221E, // Lemniscate
		0x222B, // Integral symbol
		0x2248, // Approximately equal to
		0x2260, // Not equals sign
		0x2264,	// Less-than-or-equals
		0x2265, // Greater-than-or-equals
		0x257B, // Box drawings vertical down (double check this - probably wrong)
		0x25B3, // White up-pointing triangle
		0x25C7, // White diamond
		0x2620, // Skull & Crossbones (replaces an Apple symbol)
	},

	// These are the 8 bit characters the Unicode is mapped to
	{
		0x0000, // FIXME - Small letter FI
		0x0000, // FIXME - Small letter FL
		0x0020, // Space (This is an assumption since the character is non-printing)
		0x00F6, // Spacing Circumflex
		0x00F7, // Tilde
		0x00C1, // Inverted exclamation mark
		0x00A2, // Cent symbol
		0x00A3, // Pound symbol
		0x00B4, // Yen symbol
		0x00A4, // Section symbol
		0x00AC, // Diaeresis
		0x00A9, // Copyright sign
		0x00BB, // Feminine ordinal indicator
		0x00C7, // Left double-angle quotation mark
		0x00C2, // Negation symbol
		0x00A8, // Registered sign
		0x00A1, // Degree sign
		0x00B1, // Plus-minus
		0x00B5, // Metric "micro" prefix
		0x00A6, // Paragraph symbol (pilcrow)
		0x00FC, // Cedilla
		0x00BC, // Masculine ordinal indicator
		0x00C8, // Right double-angle quotation mark
		0x00C0, // Inverted question mark
		0x00CB, // Capital A with grave
		0x00E7, // Capital A with acute accent
		0x00E5, // Capital A with cap
		0x00CC, // Capital A with tilde
		0x0080, // Capital A with diaeresis
		0x0081, // Capital A with ring
		0x00AE, // Capital letter AE
		0x0082, // Big C with cedille
		0x00E9, // Capital E with grave
		0x0083, // Big E with acute accent
		0x00E6, // Capital E with cap
		0x00E8, // Capital E with diaeresis
		0x00ED, // Capital I with grave
		0x00EA, // Capital I with acute accent
		0x00EB, // Capital I with cap
		0x00EC, // Capital I with diaeresis
		0x0084, // Big N with tilde
		0x00F1, // Capital O with grave
		0x00EE, // Capital O with acute accent
		0x00EF, // Capital O with cap
		0x00CD, // Capital O with tilde
		0x0085, // Capital O with diaeresis
		0x00AF, // Capital O with stroke
		0x00F4, // Capital U with grave
		0x00F2, // Capital U with acute
		0x00F3, // Capital U with cap
		0x0086, // Big U with diaeresis
		0x00A7, // Small letter sharp S
		0x0088, // Small A with grave accent
		0x0087, // Small A with acute accent
		0x0089, // Small A with cap
		0x008B, // Small A with tilde
		0x008A, // Small A with diaeresis
		0x008C, // Small A with ring
		0x00BE, // Small letter AE
		0x008D, // Small C with cedille
		0x008F, // Small E with grave accent
		0x008E, // Small E with acute accent
		0x0090, // Small E with cap
		0x0091, // Small E with diaeresis
		0x0093, // Small I with grave
		0x0092, // Small I with acute accent
		0x0094, // Small I with cap
		0x0095, // Small I with diaeresis
		0x0096, // Small N with tilde
		0x0098, // Small O with grave
		0x0097, // Small O with acute accent
		0x0099, // Small O with cap
		0x009B, // Small O with tilde
		0x009A, // Small O with diaeresis
		0x00D6, // Division symbol (bar and 2 dots)
		0x00BF, // Small O with stroke
		0x009D, // Small U with grave accent
		0x009C, // Small U with acute accent
		0x009E, // Small U with cap
		0x009F, // Small U with diaeresis
		0x00D8, // Small Y with diaeresis
		0x00CE, // Capital letter OE
		0x00CF, // Small letter OE
		0x00D9, // Capital Y with diaeresis
		0x00C4, // Curly F (function symbol)
		0x00FF, // Caron
		0x00F9, // Breve
		0x00FA, // Dot above
		0x00FB, // Ring above
		0x00FE, // Ogonek
		0x00BD, // Capital letter omega
		0x00B9, // Small letter pi
		0x00D0, // En dash (short hyphen)
		0x00D1, // Em dash (long hyphen)
		0x00D4, // Left single quotation mark
		0x00D5, // Right single quotation mark
		0x00E2, // Comma (?)
		0x00D2, // Left double quotation mark
		0x00D3, // Right double quotation mark
		0x00E3, // Right low double comma quotation mark
		0x00A0, // Single Dagger
		0x00E0, // Double dagger
		0x00A5, // Bullet
		0x00C9, // Ellipsis
		0x00E4, // Per Mille
		0x00AB, // Prime
		0x00FD, // Double Prime
		0x00DC, // Single left-pointing angle quotation mark
		0x00DD, // Single right-pointing angle quotation mark
		0x00F8, // Overline
		0x00DB, // Euro symbol
		0x00AA, // Trademark sign
		0x00B6, // Partial derivative symbol
		0x00B8, // N-ary product operator
		0x00B7, // N-ary summation operator
		0x00DA, // Division slash
		0x00E1, // Middle dot
		0x00C3, // Square root symbol
		0x00B0, // Lemniscate
		0x00BA, // Integral symbol
		0x00C5, // Approximately equal to
		0x00AD, // Not equals sign
		0x00B2, // Less-than-or-equals
		0x00B3, // Greater-than-or-equals
		0x00F5, // Box drawings vertical down (double check this - probably wrong)
		0x00C6, // White up-pointing triangle
		0x00D7, // White diamond
		0x00F0, // Apple symbol
	}
};

// This table maps standard IBM extended ASCII into Unicode - it is not currently
// needed, but may come in handy in the future and someplace far, far away.
//
//static const UInt16 PCAsciiToUnicodeTable[128] =
//{	
//		0x00C7,	// Big C with cedille
//		0x00FC,	// Small U with Diaeresis		
//		0x00E9,	// Small E with acute accent
//		0x00E2, // Small A with cap
//		0x00E4,	// Small A with Diaeresis
//		0x00E0, // Small A with grave accent
//		0x00E5,	// Small A with ring
//		0x00E7,	// Small C with cedille
//		0x00EA,	// Small E with cap
//		0x00EB,	// Small E with Diaeresis
//		0x00E8, // Small E with grave accent
//		0x00EF,	// Small I with Diaeresis
//		0x00EE, // Small I with cap
//		0x00EC, // Small I with grave
//		0x00C4, // Big A with Diaeresis
//		0x00C5, // Big A with ring
//		0x00C9, // Big E with acute accent
//		0x00E6, // Small letter AE
//		0x00C6, // Big letter AE
//		0x00D4, // Big O with cap
//		0x00F6, // Small O with Diaeresis
//		0x00D2, // Big O with grave accent
//		0x00DB, // Big U with cap
//		0x00F9, // Small U with grave accent
//		0x00FF, // Small Y with Diaeresis
//		0x00D6, // Big O with Diaeresis
//		0x00DC, // Big U with Diaeresis
//		0x00A2, // Cent (currency) symbol
//		0x00A3, // Pound (currency) symbol
//		0x00A5, // Yen (currency) symbol
//		0x0000, // FIXME - What is this symbol? (looks like Pt)
//		0x0192, // Small F with hook
//		0x00E1, // Small A with acute accent
//		0x00ED, // Small I with acute accent
//		0x00F3, // Small O with acute accent
//		0x00FA, // Small U with acute accent
//		0x00F1, // Small N with tilde
//		0x00D1, // Big N with tilde
//		0x00AA, // Feminine ordinal indicator
//		0x00BA, // Masculine ordinal indicator
//		0x00BF, // Inverted question mark
//		0x2310, // Reversed not symbol
//		0x00AC, // Not symbol
//		0x00BD, // Vulgar fraction 1/2
//		0x00BC, // Vulgar fraction 1/4
//		0x00A1, // Inverted exclamation mark
//		0x00AB, // Left double-angle quotation mark
//		0x00BB, // Right double-angle quotation mark
//		0x2591, // Light shade
//		0x2592, // Medium shade
//		0x2593, // Dark shade
//		0x2503, // Box - heavy vertical
//		0x2528, // Box - heavy vertical, light left
//		0x2561, // Box - vertical single, left double
//		0x2562, // Box - double vertical, left single
//		0x2556, // Box - down double, left single
//		0x2555, // Box - down single, left double
//		0x2563, // Box - double vertical and left
//		0x2551, // Box - double vertical
//		0x2557, // Box - double down and left
//		0x255D, // Box - double up and left
//		0x255C, // Box - up double, left single
//		0x255B,	// Box - up single, left double
//		0x2513, // Box - heavy down and left
//		0x2517, // Box - heavy down and right
//		0x2538, // Box - up heavy, horizontal light
//		0x2533, // Box - down heavy, horizontal heavy
//		0x2520, // Box - vertical heavy, right light
//		0x2500, // Box - light horizontal
//		0x2542, // Box - vertical heavy, horizontal light
//		0x255E, // Box - vertical single, right double
//		0x255F, // Box - vertical double, right single
//		0x255A, // Box - double up and right
//		0x2554, // Box - double down and right
//		0x2569, // Box - double up and horizontal
//		0x2566, // Box - double down and horizontal
//		0x2560, // Box - double vertical and right
//		0x2550, // Box - double horizontal
//		0x256C, // Box - double vertical and horizontal
//		0x2567, // Box - up single, horizontal double
//		0x0000, // FIXME - what the hell is this thing?
//		0x2564, // Box - down single, horizontal double
//		0x0000, // FIXME - this is another problem character
//		0x2559, // Box - up double, right single
//		0x2558, // Box - up single, right double
//		0x2552, // Box - down single, right double
//		0x2553, // Box - down double, right single
//		0x256B, // Box - vertical double, horizontal single
//		0x256A, // Box - vertical single, horizontal double
//		0x251A, // Box - up heavy, left light
//		0x250F, // Box - heavy down and right
//		0x2589, // Block - left seven-eigths block
//		0x2584, // Block - lower half-block
//		0x258C, // Bloxk - left half-block
//		0x2590, // Block - right half-block
//		0x2580, // Block - upper half-block
//		0x03B1, // Small letter alpha
//		0x03B2, // Small letter beta
//		0x0393, // Big letter gamma
//		0x03C0, // Small letter pi
//		0x03A3, // Big letter sigma
//		0x03C3, // Small letter sigma
//		0x03BC, // Small letter mu
//		0x03B3, // Small letter gamma
//		0x03A6, // Big letter phi
//		0x0398, // Big letter theta
//		0x038F, // Big letter omega
//		0x03B4, // Small letter delta
//		0x221E, // Lemniscate
//		0x222E, // Line integral
//		0x2208, // Belongs to set
//		0x2229, // Intersection
//		0x2261, // Equivalent
//		0x00B1, // Plus-minus
//		0x2265, // Greater-than or equal
//		0x2264, // Less-than or equal
//		0x2320, // Integral - upper half
//		0x2321, // Integral - lower half
//		0x00F7, // division symbol (bar and 2 dots)
//		0x2248, // Approximately equal to
//		0x2218, // Composite operator (or is it degree symbol?)
//		0x2219, // Dot operator (inner product)
//		0x221A, // Square root
//		0x207F, // Exponent (superscript) n
//		0x00B2, // Squared (superscript 2)
//		0x220E, // QED
//		0x0000,	// These last 2 values are fillers
//		0x0000
//};


// ---------------------------------------------------------------------
// CStrToUnistring            		                            [public]
// ---------------------------------------------------------------------
// creates a CString from a c string

CString
UStringConverter::CStrToUnistring(
		const char* inCStr,
		EStringEncoding inStringEncoding )
{
	ASSERT( inCStr != NULL );

	// This is the character count, NULL excluded, of
	// the input string
	UInt32 numChars = strlen( inCStr );

	// This is our constructed output string
	CString convertedString;

	// We only need to look up a character if the ASCII code 
	// is greater than 127, since low ASCII is the same across 
	// platforms.
	const UInt16* pLookupTable;
	
	if( inStringEncoding == eMacAscii )
		pLookupTable = MacAsciiToUnicodeTable;
	else
		pLookupTable = PCAsciiToUnicodeTable;

	// Examine each character in the input string and
	// perform a table lookup in either the Mac or PC ASCII
	// table to convert from ASCII to Unicode, i.e. wide characters.
	// * We are assuming that a US-English code page is in use
	// * on the system running the application
	for( UInt32 index = 0; index < numChars; index++ )
	{
		// Get the 8-bit character from input string
		UInt8 narrowChar = (UInt8)inCStr[ index ];

		// This is the input character we're examining, and
		// the converter we use to widen characters
		wchar_t wideChar;

		// Check if it is extended ASCII
		if( (UInt32)narrowChar > kAsciiTableSize - 1 )
			// Do our lookup conversion for the Unicode symbol if it is
			wideChar = pLookupTable[ narrowChar - kAsciiTableSize ];
		else
			wideChar = (wchar_t)narrowChar;
					
		convertedString.append( 1, wideChar );
	}
	
	return convertedString;
}

// ---------------------------------------------------------------------
//	PStrToUnistring          		                            	 [public]
// ---------------------------------------------------------------------
// creates a CString from a p string

CString
UStringConverter::PStrToUnistring(
		const UInt8* inPStr,
		EStringEncoding inStringEncoding )
{
	ASSERT( inPStr != NULL );

	// This is the character count, NULL excluded, of
	// the input string
	UInt32 numChars = inPStr[0];

	// This is our constructed output string
	CString convertedString;

	// We only need to look up a character if the ASCII code 
	// is greater than 127, since low ASCII is the same across 
	// platforms.
	const UInt16* pLookupTable;

	if( inStringEncoding == eMacAscii )
		pLookupTable = MacAsciiToUnicodeTable;
	else
		pLookupTable = PCAsciiToUnicodeTable;


	// Examine each character in the input string and
	// perform a table lookup in either the Mac or PC ASCII
	// table to convert from ASCII to Unicode, i.e. wide characters.
	// * We are assuming that a US-English code page is in use
	// * on the system running the application
	for( UInt32 index = 1; index <= numChars; index++ )
	{
		// Retrieve the current character
		UInt8 narrowChar = inPStr[ index ];
	
		// This is the input character we're examining, and
		// the converter we use to widen characters
		wchar_t wideChar;

		// Check if it is extended ASCII
		if( (UInt32)narrowChar > kAsciiTableSize - 1 )
			// Do our lookup conversion for the Unicode symbol if it is
			wideChar = pLookupTable[ narrowChar - kAsciiTableSize ];
		else
			wideChar = (wchar_t)narrowChar;
		
		convertedString.append( 1, wideChar );
	}
	
	return convertedString;
}

// ---------------------------------------------------------------------
//	StringDataToUnistring      		                          	 [public]
// ---------------------------------------------------------------------
// creates a CString from raw string data

CString
UStringConverter::StringDataToUnistring(
		const void* inCStr,
		const UInt32 inSize,
		EStringEncoding inStringEncoding )
{
	ASSERT( inCStr != NULL );

	if( inSize == 0 )
		return L"";

	// This is the character count, NULL excluded, of
	// the input string
	UInt32 numChars = inSize;

	// This is our constructed output string
	CString convertedString;

	// We only need to look up a character if the ASCII code 
	// is greater than 127, since low ASCII is the same across 
	// platforms.
	const UInt16* pLookupTable;

	if( inStringEncoding == eMacAscii )
		pLookupTable = MacAsciiToUnicodeTable;
	else
		pLookupTable = PCAsciiToUnicodeTable;


	// Examine each character in the input string and
	// perform a table lookup in either the Mac or PC ASCII
	// table to convert from ASCII to Unicode, i.e. wide characters.
	// * We are assuming that a US-English code page is in use
	// * on the system running the application
	for( UInt32 index = 0; index < numChars; index++ )
	{
		// Retrieve the current character
		char narrowChar = ((char*)inCStr)[ index ];
	
		// This is the input character we're examining, and
		// the converter we use to widen characters
		wchar_t wideChar;

		// Check if it is extended ASCII
		if( (UInt32)narrowChar > kAsciiTableSize - 1 )
			// Do our lookup conversion for the Unicode symbol if it is
			wideChar = pLookupTable[ narrowChar - kAsciiTableSize ];
		else
			wideChar = (wchar_t)narrowChar;
		
		convertedString.append( 1, wideChar );
	}
	
	return convertedString;
}

// ---------------------------------------------------------------------
//	UnistringToCStr      		                                  [public]
// ---------------------------------------------------------------------
// creates a cstring from a CString (allocates memory for it)

char*
UStringConverter::UnistringToCStr(
		const CString& inStr,
		const EStringEncoding inEncoding,
		const bool inAllowUndefined,
		const char inReplaceCharacter )
{
	// We have to convert inStr into a c-string having the desired
	// encoding, using characters from the standard (in the
	// Western world, anyways) US-English codepage. If there is no
	// mapping from a given Unicode character to the U.S. English
	// codepage, the character is replaced in the output string with
	// the character given by inReplaceCharacter. If we do not allow
	// undefined characters, but one is detected in the input string
	// then an exception is thrown.

	// Allocate space for our c-string, including a terminating null character
	UInt32 stringLength = inStr.length();
	UInt8* pNewString = new UInt8[stringLength + 1];

	/*if( pNewString == NULL )
		THROW_RESOURCE_( eCreatingMemory, eMemory, kNoOSError );
*/
	// Look at each character in the input string and convert
	// it to an 8 bit character; write it to the output buffer
	// in the process.
	for( UInt32 index = 0; index < stringLength; index++ )
	{
		// Retrieve the current character from input string
		wchar_t inputChar = inStr[index];

		if( ( inputChar >= (wchar_t)0 ) && ( inputChar <= (wchar_t)127 ) )
		{
			// If the value of the unicode character is between 0x0000 and 0x007F 
			// inclusive, we have to truncate it down to 8-bits
			pNewString[index] = (UInt8)inputChar;
		}
		else
		{
			char outputChar;

			// Do the search of the lookup table
			if( ConvertUnicodeToAscii( inputChar, inEncoding, outputChar ) )
				pNewString[index] = (UInt8)outputChar;
			else
			{
				if( inAllowUndefined )
					pNewString[index] = (UInt8)inReplaceCharacter;
				else
				{
					delete [] pNewString;
					// We don't allow undefined unicode input characters,
					// so bitch and moan because we found one
//					THROW_RESOURCE_( eCreatingMemory, eMemory, kNoOSError );
//					THROW UNDEFINED CHARACTERS;
				}
			}

		}

	}

	// Make sure to NULL terminate the c-string
	pNewString[ stringLength ] = '\0';

	return (char *)pNewString;
}

// ---------------------------------------------------------------------
//	UnistringToPStr                                              [public]
// ---------------------------------------------------------------------
// creates a p-string from a CString

UInt8*
UStringConverter::UnistringToPStr(
		const CString& inStr,
		const EStringEncoding inEncoding,
		const bool inAllowUndefined ,
		const char inReplaceCharacter )
{
	// We have to convert inStr into a p-string having the desired
	// encoding, using characters from the standard (in the
	// Western world, anyways) US-English codepage. If there is no
	// mapping from a given Unicode character to the U.S. English
	// codepage, the character is replaced in the output string with
	// the character given by inReplaceCharacter. If we do not allow
	// undefined characters, but one is detected in the input string
	// then an exception is thrown.

	// Allocate space for our p-string, including a leading byte count
	UInt32 stringLength = inStr.length();

	if( stringLength > kMaxPascalStrLen )
		stringLength = kMaxPascalStrLen;

	UInt8* pNewString = new UInt8[stringLength + 1];

	/*if( pNewString == NULL )
		THROW_RESOURCE_( eCreatingMemory, eMemory, kNoOSError );
*/
	pNewString[0] = (UInt8)stringLength;

	// Look at each character in the input string and convert
	// it to an 8 bit character; write it to the output buffer
	// in the process.
	for( UInt32 index = 0; index < stringLength; index++ )
	{
		// Retrieve the current character from input string
		wchar_t inputChar = inStr[index];

		if( ( inputChar >= (wchar_t)0 ) && ( inputChar <= (wchar_t)127 ) )
		{
			// If the value of the unicode character is between 0x0000 and 0x007F
			// inclusive, we have to truncate it down to 8-bits
			pNewString[index + 1] = (UInt8)inputChar;
		}
		else
		{
			char outputChar;

			// Do the search of the lookup table
			if( ConvertUnicodeToAscii( inputChar, inEncoding, outputChar ) )
				pNewString[index + 1] = (UInt8)outputChar;
			else
			{
				if( inAllowUndefined )
					pNewString[index + 1] = (UInt8)inReplaceCharacter;
				else
				{
					delete [] pNewString;
					// We don't allow undefined unicode input characters,
					// so bitch and moan because we found one
//					THROW_RESOURCE_( eCreatingMemory, eMemory, kNoOSError );
//					THROW UNDEFINED CHARACTERS;
				}
			}

		}

	}

	return pNewString;
}

// ---------------------------------------------------------------------
// UnistringToStringData                                        [public]
// ---------------------------------------------------------------------
// creates string data given a unicode string (allocates memory for it)

void
UStringConverter::UnistringToStringData(
		const CString& inStr,
		void* &outData,
		UInt32& ioSize,
		const EStringEncoding inEncoding,
		const bool inAllowUndefined,
		const char inReplaceCharacter )
{
	UInt32 stringLength = inStr.length();

	UInt8* pNewString = new UInt8[stringLength];
	/*if( pNewString == NULL )
		THROW_RESOURCE_( eCreatingMemory, eMemory, kNoOSError );
*/
	pNewString[0] = (UInt8)stringLength;

	for( UInt32 index = 0; index < stringLength; index++ )
	{
		wchar_t inputChar = inStr[index];

		if( ( inputChar >= (wchar_t)0 ) && ( inputChar <= (wchar_t)127 ) )
			pNewString[index] = (UInt8)inputChar;
		else
		{
			char outputChar;

			// Do the search of the lookup table
			if( ConvertUnicodeToAscii( inputChar, inEncoding, outputChar ) )
				pNewString[index] = (UInt8)outputChar;
			else
			{
				if( inAllowUndefined )
					pNewString[index] = (UInt8)inReplaceCharacter;
				else
				{
					delete [] pNewString;
					// We don't allow undefined unicode input characters,
					// so bitch and moan because we found one
//					THROW_RESOURCE_( eCreatingMemory, eMemory, kNoOSError );
//					THROW UNDEFINED CHARACTERS;
				}
			}

		}

	}

	outData = (void *)pNewString;
}

// ---------------------------------------------------------------------
// CharToUnichar                                                [public]
// ---------------------------------------------------------------------
// converts an ascii character in a given encoding into a unicode
// character

wchar_t
UStringConverter::CharToUnichar(
		const char inChar,
		const EStringEncoding inEncoding )
{
	const UInt16* pLookupTable;

	if( inEncoding == eMacAscii )
		pLookupTable = MacAsciiToUnicodeTable;
	else
		pLookupTable = PCAsciiToUnicodeTable;

	if( (UInt32)inChar > kAsciiTableSize - 1 )
		return pLookupTable[ inChar - kAsciiTableSize ];
	return (wchar_t)inChar;
}

// ---------------------------------------------------------------------
// UnicharToChar                                                [public]
// ---------------------------------------------------------------------
// converts a unicode character in a given encoding into a unicode
// character

char
UStringConverter::UnicharToChar(
		const wchar_t inChar,
		const EStringEncoding inEncoding,
		const bool inAllowUndefined,
		const char inReplaceCharaceter )
{
	if( ( inChar >= (wchar_t)0 ) && ( inChar <= (wchar_t)127 ) )
		return (char)inChar;

	char outputChar;
	if( ConvertUnicodeToAscii( inChar, inEncoding, outputChar ) )
		return outputChar;
	if( !inAllowUndefined )
//		THROW_RESOURCE_( eCreatingMemory, eMemory, kNoOSError );
//		THROW UNDEFINED CHARACTERS;
		;
	return outputChar;
}

// ---------------------------------------------------------------------
// compare
// ---------------------------------------------------------------------
// bsearch helper function (see next function). Compares 16-bit values.

static int compare( const void* elem1, const void* elem2 )
{
	if ( *(UInt16 *)elem1 > *(UInt16 *)elem2 )
		return 1;
	if ( *(UInt16 *)elem1 < *(UInt16 *)elem2 )
		return -1;
	return 0;
}

// ---------------------------------------------------------------------
// ConvertUnicodeToAscii                                        [public]
// ---------------------------------------------------------------------
// Uses binary search to convert unicode character to ASCII. There sould
// not be any duplicate entries in the lookup table.

#include <stdlib.h>		// bsearch

bool
UStringConverter::ConvertUnicodeToAscii(
		wchar_t inChar,
		const EStringEncoding inEncoding,
		char& outChar )
{
	const UInt16(* pLookupTable)[ kAsciiTableSize ];

	if( inEncoding == eMacAscii )
		pLookupTable = UnicodeToMacAsciiTable;
	else
		pLookupTable = UnicodeToPCAsciiTable;
	UInt16* found = (UInt16 *)bsearch( (const void *)&inChar, (void *)pLookupTable[0], kAsciiTableSize, sizeof( UInt16 ), &compare );
	int index = found - pLookupTable[0];
	if ( found == NULL || index < 0 || index > 128 )
		return false;
	outChar = (char)pLookupTable[1][index];
	return true;
}

HL_End_Namespace_BigRedH
