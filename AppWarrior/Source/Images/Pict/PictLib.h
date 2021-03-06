/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "ImageTypes.h"

//#ifdef PPM_PACKCOLORS
//#define ppm_hashpixel(p) ( ( (Int16) (p) & 0x7fffffff ) % HASH_SIZE )
//#else /*PPM_PACKCOLORS*/
#define ppm_hashpixel(p) ( ( ( (Int32) PPM_GETR(p) * 33023 + (Int32) PPM_GETG(p) * 30013 + (Int32) PPM_GETB(p) * 27011 ) & 0x7fffffff ) % HASH_SIZE )
//#endif /*PPM_PACKCOLORS*/

#define		runtochar(c)	(257-(c))
#define		counttochar(c)	((c)-1)
#define PPM_EQUAL(p,q) ((p) == (q))

#define PPM_GETR(p) (((p) & 0x3ff00000) >> 20)
#define PPM_GETG(p) (((p) & 0xffc00) >> 10)
#define PPM_GETB(p) ((p) & 0x3ff)

typedef struct colorhist_item* colorhist_vector;
struct colorhist_item
{
    Uint32 color;
    Int16 value;
};

typedef struct colorhist_list_item *colorhist_list;
struct colorhist_list_item
{
    struct colorhist_item ch;
    colorhist_list next;
};

typedef colorhist_list *colorhash_table;


colorhist_vector ppm_computecolorhist(Uint32 **pixels, Uint16 cols, Uint16 rows, Uint16 maxcolors, Uint16 *colorsP);
void ppm_addtocolorhist(colorhist_vector chv, Uint16 *colorsP, Uint16 maxcolors, Uint32 *colorP, Uint16 value, Uint16 position);
colorhash_table ppm_computecolorhash(Uint32 **pixels, Uint16 cols, Uint16 rows, Uint16 maxcolors, Uint16* colorsP);
colorhash_table ppm_alloccolorhash();
Int16 ppm_addtocolorhash(colorhash_table cht, Uint32* colorP, Int16 value);
colorhist_vector ppm_colorhashtocolorhist(colorhash_table cht, Uint16 maxcolors);
colorhash_table ppm_colorhisttocolorhash(colorhist_vector chv, Uint16 colors);
Int16 ppm_lookupcolor(colorhash_table cht,Uint32 *colorP);
void ppm_freecolorhist(colorhist_vector chv);
void ppm_freecolorhash(colorhash_table cht);
