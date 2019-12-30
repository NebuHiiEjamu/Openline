/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UMime.h"


// conversion from file extension to MIME format
const Int8 *UMime::ConvertExtension_Mime(const Int8 *inExtension)
{
	if (!inExtension)
		return "data/unknown";
	
	Uint32 nExtensionSize = strlen(inExtension);
	if (!nExtensionSize)
		return "data/unknown";
		
// application
	if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".exe", 4))
		return "application/x-msdownload";
// text
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".txt", 4))
		return "text/plain";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".htm", 4))
		return "text/html";	
// images	
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".gif", 4))
		return "image/gif";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".jpg", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".jpeg", 5))
		return "image/jpeg";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".bmp", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".dib", 4))
		return "image/bmp";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".pct", 4))
		return "image/pict";		// ???
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".fpx", 4))
		return "image/vnd.fpx";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".pnt", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".pntg", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".mac", 4))
		return "image/x-macpaint";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".psd", 4))
		return "image/x-photoshop";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".png", 4))
		return "image/png";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".sgi", 4))
		return "image/x-sgi";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".tga", 4))
		return "image/x-targa";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".tif", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".pntg", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".tiff", 5))
		return "image/tiff";
// video
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".mov", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".qt", 3))
		return "video/quicktime";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".mpg", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".mpeg", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".mpa", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".mp2", 4))
		return "video/mpeg";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".avi", 4))
		return "video/x-msvideo";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".dif", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".dv", 3))
		return "video/x-dv";
// 3D
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".swf", 4))
		return "application/x-shockwave-flash";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".flc", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".fli", 4))
		return "video/flc";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".3dm", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".pntg", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".3dmf", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".gd3d", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".qd3", 4))
		return "x-world/x-3dmf";
// audio
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".mp3", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".swa", 4))
		return "audio/mpeg";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".ram", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".rm", 3) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".rae", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".ra", 3))
		return "audio/x-pn-realaudio";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".aif", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".aiff", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".aifc", 5))
		return "audio/aiff";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".au", 3) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".snd", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".ulw", 4))
		return "audio/basic";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".wav", 4))
		return "audio/wav";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".sd2", 4))
		return "audio/x-sd2";
	else if (!UText::CompareInsensitive(inExtension, nExtensionSize, ".mid", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".mifi", 5) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".smf", 4) || !UText::CompareInsensitive(inExtension, nExtensionSize, ".kar", 4))
		return "audio/midi";
 	
	return "data/unknown";
}

// conversion from MIME format to file extension
const Int8 *UMime::ConvertMime_Extension(const Int8 *inMime)
{
	if (!inMime)
		return ".dat";

	Uint32 nMimeSize = strlen(inMime);
	if (!nMimeSize)
		return ".dat";
	
// application
	if (!UText::CompareInsensitive(inMime, nMimeSize, "application/x-msdownload", 24))
		return ".exe";
// text
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "text/plain", 10))
		return ".txt";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "text/html", 9))
		return ".htm";
// images
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/gif", 9))
		return ".gif";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/jpeg", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "image/pjpeg", 11))
		return ".jpg";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/bmp", 9) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-bmp",11) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-xbitmap", 15))
		return ".bmp";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/pict", 10))	// ???
		return ".pct";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/vnd.fpx", 13))
		return ".fpx";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-macpaint", 16))
		return ".pnt";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-photoshop", 17))
		return ".psd";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/png", 9) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-png", 11))
		return ".png";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-sgi", 11))
		return ".sgi";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-targa", 13))
		return ".tga";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/tiff", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-tiff", 12))
		return ".tif";
//video
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/quicktime", 15))
		return ".mov";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/mpeg", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "video/x-mpeg", 12))
		return ".mpg";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/x-msvideo", 15))
		return ".avi";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/x-dv", 10))
		return ".dif";
// 3D
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "application/x-shockwave-flash", 29))
		return ".swf";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/flc", 9))
		return ".flc";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "x-world/x-3dmf", 14))
		return ".3dm";
// audio	
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/mpeg", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-mpeg", 12))
		return ".mp3";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/x-pn-realaudio", 20))
		return ".ram";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/aiff", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-aiff", 12))
		return ".aif";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/basic", 11))
		return ".au";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/wav", 9) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-wav", 11))
		return ".wav";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/x-sd2", 11))
		return ".sd2";
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/midi", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-midi", 12))
		return ".mid";
					
	return ".dat";
}

// conversion from file name with extension to MIME format
const Int8 *UMime::ConvertNameExtension_Mime(const Uint8 *inName)
{
	Int8 csExtension[8];
	csExtension[0] = 0;
	
	if (inName && inName[0])
	{
		// search extension
		const Uint8 *pExtension = inName + inName[0];
		while (*pExtension != '.' && pExtension > inName + 1)
			pExtension--;
		
		if (*pExtension == '.')
		{
			Uint32 nExtensionSize = inName + inName[0] + 1 - pExtension;
			nExtensionSize = UMemory::Copy(csExtension, pExtension, nExtensionSize > sizeof(csExtension) - 1 ? sizeof(csExtension) - 1 : nExtensionSize);
			csExtension[nExtensionSize] = 0;
		}
	}
	
	return UMime::ConvertExtension_Mime(csExtension);
}

// conversion from type code to MIME format
const Int8 *UMime::ConvertTypeCode_Mime(Uint32 inTypeCode)
{
	if (!inTypeCode)
		return "data/unknown";

// application
	if (inTypeCode == TB('APPL') || inTypeCode == TB('DEXE'))
		return "application/x-msdownload";
// text
	else if (inTypeCode == TB('TEXT') || inTypeCode == TB('ttro'))
		return "text/plain";
	else if (inTypeCode == TB('WAFF'))
		return "text/html";
// images
	else if (inTypeCode == TB('GIFf') || inTypeCode == TB('GIF '))
		return "image/gif";
	else if (inTypeCode == TB('JPEG'))
		return "image/jpeg";
	else if (inTypeCode == TB('BMP ') || inTypeCode == TB('BMPf'))
		return "image/bmp";
	else if (inTypeCode == TB('PICT'))
		return "image/pict";		// ???
	else if (inTypeCode == TB('FPix'))
		return "image/vnd.fpx";
	else if (inTypeCode == TB('PNTG'))
		return "image/x-macpaint";
	else if (inTypeCode == TB('8BPS'))
		return "image/x-photoshop";
	else if (inTypeCode == TB('PNGf') || inTypeCode == TB('PNG '))
		return "image/png";
	else if (inTypeCode == TB('SGI '))
		return "image/x-sgi";
	else if (inTypeCode == TB('TPIC'))
		return "image/x-targa";
	else if (inTypeCode == TB('TIFF'))
		return "image/tiff";
// video
	else if (inTypeCode == TB('MooV') || inTypeCode == TB('sooV'))
		return "video/quicktime";
	else if (inTypeCode == TB('MPEG') || inTypeCode == TB('Mpeg') || inTypeCode == TB('mpeg') || inTypeCode == TB('MPG ') || inTypeCode == TB('MPGa') || inTypeCode == TB('MPGv') || inTypeCode == TB('MPGx') || inTypeCode == TB('MPG2') || inTypeCode == TB('MPG3'))
		return "video/mpeg";
	else if (inTypeCode == TB('AVI ') || inTypeCode == TB('VfW '))
		return "video/x-msvideo";
	else if (inTypeCode == TB('dvc!'))
		return "video/x-dv";
// 3D
	else if (inTypeCode == TB('SWFL') || inTypeCode == TB('SWF '))
		return "application/x-shockwave-flash";
	else if (inTypeCode == TB('FLI '))
		return "video/flc";
	else if (inTypeCode == TB('3DMF'))
		return "x-world/x-3dmf";
// audio	
	else if (inTypeCode == TB('MP3 ') || inTypeCode == TB('MPG3') || inTypeCode == TB('SwaT'))
		return "audio/mpeg";
	else if (inTypeCode == TB('RAE '))
		return "audio/x-pn-realaudio";
	else if (inTypeCode == TB('AIFF') || inTypeCode == TB('AIFC'))
		return "audio/aiff";
	else if (inTypeCode == TB('ULAW'))
		return "audio/basic";
	else if (inTypeCode == TB('WAVE') || inTypeCode == TB('.WAV'))
		return "audio/wav";
	else if (inTypeCode == TB('Sd2f') || inTypeCode == TB('SD2 '))
		return "audio/x-sd2";
	else if (inTypeCode == TB('Midi'))
		return "audio/midi";
		
	return "data/unknown";
}

// conversion from MIME format to type code
Uint32 UMime::ConvertMime_TypeCode(const Int8 *inMime)
{
	if (!inMime)
		return TB('BINA');;

	Uint32 nMimeSize = strlen(inMime);
	if (!nMimeSize)
		return TB('BINA');;
	
// application
	if (!UText::CompareInsensitive(inMime, nMimeSize, "application/x-msdownload", 24))
		return TB('APPL');
// text
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "text/plain", 10))
		return TB('TEXT');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "text/html", 9))
		return TB('WAFF');
// images
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/gif", 9))
		return TB('GIFf');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/jpeg", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "image/pjpeg", 11))
		return TB('JPEG');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/bmp", 9) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-bmp",11) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-xbitmap", 15))
		return TB('BMP ');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/pict", 10))	// ???
		return TB('PICT');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/vnd.fpx", 13))
		return TB('FPix');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-macpaint", 16))
		return TB('PNTG');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-photoshop", 17))
		return TB('8BPS');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/png", 9) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-png", 11))
		return TB('PNGf');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-sgi", 11))
		return TB('SGI ');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/x-targa", 13))
		return TB('TPIC');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "image/tiff", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "image/x-tiff", 12))
		return TB('TIFF');
// video
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/quicktime", 15))
		return TB('MooV');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/mpeg", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "video/x-mpeg", 12))
		return TB('MPEG');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/x-msvideo", 15))
		return TB('VfW ');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/x-dv", 10))
		return TB('dvc!');
// 3D
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "application/x-shockwave-flash", 29))
		return TB('SWFL');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "video/flc", 9))
		return TB('FLI ');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "x-world/x-3dmf", 14))
		return TB('3DMF');
// audio
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/mpeg", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-mpeg", 12))
		return TB('MP3 ');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/x-pn-realaudio", 20))
		return TB('RAE ');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/aiff", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-aiff", 12))
		return TB('AIFF');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/basic", 11))
		return TB('ULAW');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/wav", 9) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-wav", 11))
		return TB('WAVE');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/x-sd2", 11))
		return TB('Sd2f');
	else if (!UText::CompareInsensitive(inMime, nMimeSize, "audio/midi", 10) || !UText::CompareInsensitive(inMime, nMimeSize, "audio/x-midi", 12))
		return TB('Midi');

	return TB('BINA');;
}

