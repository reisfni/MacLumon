#pragma once

#define USE_TOOLBOX_FIXED_ROUTINES 0

#if USE_TOOLBOX_FIXED_ROUTINES
	// Use the toolbox functions introducted in the 128K ROMs.
	// This generates code that is incompatible with 64k ROMs,
	// is larger and runs slower. There is probably no reason
	// to do so, unless you want the additional error checking
	// and rounding.

	#define fixRatio      FixRatio
	#define fixMul        FixMul
	#define long2Fix      Long2Fix
	#define fix2Long      Fix2Long
#else
	// Use our own functions for compatibility with 64k ROMs.
	// By using 8 bits rather than 16 for the decimals, we also
	// end up with tighter code.

	#define kFixShift     8

	#define fixRatio(A,B) (((A) << kFixShift) / (B))
	#define fixMul(A,B)   ((A) * (B) >> kFixShift)
	#define long2Fix(A)   (((long)A) << kFixShift)
	#define fix2Long(A)   ((A) >> kFixShift)
#endif

#define float2Fix(A)    ((Fixed)(long2Fix (1) * A))
#define scaleBy(A,B)    fixMul (A,float2Fix(B))

extern const unsigned char easeInQuad[];
extern const unsigned char easeOutQuad[];
extern const signed char sineTable[];