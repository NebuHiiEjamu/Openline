#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UMath.h"

//#undef cos
//#undef sin
//#undef tan
//#undef acos
//#undef asin
//#undef atan
//#undef sqrt
#include <math.h>
//#include <fp.h>
#include <OSUtils.h>

/* -------------------------------------------------------------------------- */

Int32 UMath::CalcRandomSeed()
{
	UnsignedWide microsecs;
	::Microseconds(&microsecs);
	
	return (microsecs.hi*4294967L + microsecs.lo/1000);		// 4294967L = 2^32 / 1000
}

fast_float UMath::Cosine(fast_float x)
{
	return cos(x);
}

fast_float UMath::Sine(fast_float x)
{
	return sin(x);
}

fast_float UMath::Tangent(fast_float x)
{
	return tan(x);
}

fast_float UMath::ArcCosine(fast_float x)
{
	return acos(x);
}

fast_float UMath::ArcSine(fast_float x)
{
	return asin(x);
}

fast_float UMath::ArcTangent(fast_float x)
{
	return atan(x);
}

fast_float UMath::SquareRoot(fast_float x)
{
	return sqrt(x);
}




#endif /* MACINTOSH */
