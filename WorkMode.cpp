#include "Lumon.h"
#include "Logo.h"
#include "FixedMath.h"
#include "MathTables.h"

#define NUM_ELEMS(arr) (sizeof(arr) / sizeof((arr)[0]))

enum {
	kMaxNumbersToAnimate = 12,
	kGridCols            = 16,
	kGridRows            = 7,
	kBoxFlapsOpen        = 22
};

typedef struct {
	char digit;
	char size;
	short res;
	Fixed x, y;
	long res1;
} Number;

typedef struct {
	unsigned long tStart, tDelta;
	Number *number[kMaxNumbersToAnimate];
	Fixed xStart[kMaxNumbersToAnimate], yStart[kMaxNumbersToAnimate];
	Fixed xDelta[kMaxNumbersToAnimate], yDelta[kMaxNumbersToAnimate];
	char whichBin;
} Animation;

static Boolean isAnimating = false;
static short headerFontSize, footerFontSize, unselectedNumberSize, selectedNumberSize;

static Rect gridRect, binRects[5], progRects[5], logoRect, titleRect;
static short cellWidth, cellHeight, progPadding, titlePadding, boxFlaps, pendingBin, level = 1;
static char binProgress[5] = {0};

static Number grid[kGridCols][kGridRows];
static Animation animation;

static void ResetNumber (short x, short y, short size) {
	Number *number = &grid[x][y];

	FontInfo fi;

	TextFont (helvetica);
	TextFace (normal);
	TextSize (size);

	GetFontInfo (&fi);

	const short charWidth  = CharWidth (number->digit);
	const short charHeight = fi.ascent + fi.descent;

	number->x     = long2Fix(gridRect.left + cellWidth  / 2 + cellWidth  * x - charWidth  / 2);
	number->y     = long2Fix(gridRect.top  + cellHeight / 2 + cellHeight * y - charHeight / 2 + fi.ascent);
	number->size  = size;
}

static void ResetNumber (Number *number, short size) {
	for (short y = 0; y <  kGridRows; y++)
	for (short x = 0; x <  kGridCols; x++) {
		if (number == &grid[x][y]) {
			ResetNumber (x, y, size);
		}
	}
}

static void ChooseNumber (Number *number) {
	number->digit = '0' + ((unsigned short)Random() % 10);
}

static void DrawNumber (Number *number) {
	TextFont (helvetica);
	TextFace (number->size == unselectedNumberSize ? normal : bold);
	TextSize (number->size);
	MoveTo (
		fix2Long (number->x),
		fix2Long (number->y)
	);
	DrawChar (number->digit);
}

static Boolean DrawTotalProgress () {
	Str255 progressStr, theStr;
	GetIndString (progressStr, 128, 1);
	short totalScore = (binProgress[0] + binProgress[1] + binProgress[2] + binProgress[3] + binProgress[4]) / 5;

	NumToString (totalScore, theStr);

	TextFont (helvetica);
	TextSize (headerFontSize);
	TextFace (bold + outline);
	TextMode (srcXor);

	FontInfo fi;
	GetFontInfo (&fi);

	MoveTo (logoRect.left - StringWidth(progressStr) - StringWidth(theStr) - CharWidth(' '), titleRect.top + titlePadding + fi.ascent);
	DrawString (theStr);
	DrawString (progressStr);
	return totalScore == 100;
}

static void DrawBinProgress (short which) {
	Str255 theStr;
	FontInfo fi;

	TextFont (helvetica);
	TextSize (footerFontSize);
	TextFace (normal);

	PenMode (patCopy);
	TextMode (srcXor);

	Rect myRect = progRects[which];
	EraseRect (&myRect);
	FrameRect (&myRect);

	myRect.right = myRect.left + (myRect.right - myRect.left) * binProgress[which] / 100;
	PaintRect (&myRect);

	GetFontInfo (&fi);
	MoveTo (myRect.left + progPadding, myRect.bottom - progPadding - fi.descent);
	NumToString (binProgress[which], theStr);

	DrawString (theStr);
	DrawChar('%');
}

static void DrawSeparator(short y, short separation) {
	MoveTo (0, y - separation);
	LineTo(qd.screenBits.bounds.right, y - separation);
	MoveTo (0, y + separation);
	LineTo(qd.screenBits.bounds.right, y + separation);
}


static void FinishedLevel () {
	binProgress[0] = binProgress[1] = binProgress[2] = binProgress[3] = binProgress[4] = 0;
	level++;
	DrawWorkMode ();
}

// Draws the box flaps, the easing ranges from 0 to 255, zero being fully closed,
// 255 fully open.

static void DrawBoxFlaps (short whichBox, unsigned char easing) {
	Rect boxRect = binRects[whichBox];

	if (easing == 0) return;

	const long angle = (unsigned long)easing * kBoxFlapsOpen >> easeFuncBits;

	const short flapLength = (boxRect.right - boxRect.left) / 2;
	const short angleShifted = (angle + NUM_ELEMS(sineTable) / 4) % NUM_ELEMS(sineTable);
	const long   sine = ((long)sineTable[ angle        ] * flapLength) >> trigFuncBits;
	const long cosine = ((long)sineTable[ angleShifted ] * flapLength) >> trigFuncBits;

	const short depthEffect = flapLength / 4 * easing >> easeFuncBits;

	MoveTo (boxRect.left           , boxRect.top);
	LineTo (boxRect.left  + cosine , boxRect.top - sine);
	LineTo (boxRect.left  + cosine , boxRect.top - sine - depthEffect);
	LineTo (boxRect.left           , boxRect.top        - depthEffect);
	LineTo (boxRect.left           , boxRect.top);

	MoveTo (boxRect.right          , boxRect.top);
	LineTo (boxRect.right - cosine , boxRect.top - sine);
	LineTo (boxRect.right - cosine , boxRect.top - sine - depthEffect);
	LineTo (boxRect.right          , boxRect.top        - depthEffect);
	LineTo (boxRect.right          , boxRect.top);

	MoveTo (boxRect.left           , boxRect.top        - depthEffect);
	LineTo (boxRect.right          , boxRect.top        - depthEffect);
}

void StartWorkMode () {
	DrawWorkMode (true);
}

void DrawWorkMode (Boolean resetNumbers) {
	BackColor( blackColor );
	ForeColor( whiteColor );
	EraseRect (&qd.screenBits.bounds);

	// Adjust the font sizes for small screens

	if ((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) < 500) {
		headerFontSize       = 18;
		footerFontSize       = 12;
		unselectedNumberSize = 12;
		selectedNumberSize   = 24;
		PenSize (1,1);
	} else {
		headerFontSize       = 24;
		footerFontSize       = 18;
		unselectedNumberSize = 18;
		selectedNumberSize   = 36;
		PenSize (2,2);
	}

	Rect binRect, progRect;

	// Clear the screen

	EraseRect( &qd.screenBits.bounds );

	// Draw the header

	TextFont (helvetica);
	TextSize (headerFontSize);
	TextFace (bold);

	FontInfo fi;
	GetFontInfo (&fi);
	const short titleLineHeight = fi.ascent + fi.descent;
	const short titleMargin     = scaleBy (titleLineHeight, 1.25);
	titlePadding                = scaleBy (titleLineHeight, 0.25);
	short logoWidth, logoHeight;

	GetGlobeSize (&logoWidth, &logoHeight);

	SetRect (
		&titleRect,
		titleMargin,
		titleMargin,
		qd.screenBits.bounds.right - titleMargin - titlePadding * 2 - logoWidth/2,
		titleMargin + titleLineHeight + titlePadding * 2);
	FrameRect(&titleRect);

	Str255 myStr;
	MoveTo (titleMargin + titlePadding, titleMargin + titlePadding + fi.ascent);
	GetIndString (myStr, 129, level);
	DrawString (myStr);

	const short headerHeight = titleRect.bottom + titleMargin;

	DrawSeparator (headerHeight, 2);

	logoRect         = titleRect;
	logoRect.top    -= (titleRect.bottom - titleRect.top) / 2;
	logoRect.bottom += (titleRect.bottom - titleRect.top) / 2;
	logoRect.left    = titleRect.right - logoWidth/2;
	logoRect.right   = titleRect.right + logoWidth/2;

	DrawLumonGlobe (logoRect, gLumonIcon);
	DrawTotalProgress ();

	// Draw the footer

	TextFont (helvetica);
	TextSize (footerFontSize);
	TextFace (bold);

	GetFontInfo (&fi);
	GetIndString (myStr, 128, 2);
	const short footerLineHeight    = fi.ascent + fi.descent;
	const short footerPadding       = scaleBy (footerLineHeight, 0.2);
	const short footerWidth         = StringWidth (myStr);
	const short footerTop           = qd.screenBits.bounds.bottom - footerLineHeight - footerPadding * 2;
	MoveTo (
		qd.screenBits.bounds.right/2 - footerWidth/2,
		qd.screenBits.bounds.bottom - footerPadding - fi.descent
	);
	DrawString    (myStr);
	DrawSeparator (footerTop, 0);

	// Draw the progress bars

	TextFont (helvetica);
	TextSize (footerFontSize);
	TextFace (normal);

	GetFontInfo (&fi);

	const short progLineHeight         = fi.ascent + fi.descent;
	const short columnMarginLeftRight  = qd.screenBits.bounds.right / 38;
	const short progMarginTopBot       = scaleBy (progLineHeight, 0.25);
	progPadding                        = scaleBy (progLineHeight, 0.25);
	const short columnWidth            = (qd.screenBits.bounds.right - columnMarginLeftRight * 8) / 5;
	const short progHeight             = fi.ascent + fi.descent + progPadding * 2;

	progRect.bottom = footerTop - progMarginTopBot;
	progRect.top    = progRect.bottom - progPadding * 2 - progLineHeight;

	for (int i = 0; i < 5; i++) {
		progRect.left  = columnMarginLeftRight * (2 + i) + columnWidth * i;
		progRect.right = progRect.left + columnWidth;
		progRects[i] = progRect;
		DrawBinProgress (i);
	}

	// Draw the bins

	TextFont (helvetica);
	TextSize (footerFontSize);
	TextFace (bold);

	GetFontInfo (&fi);

	const short binLineHeight       = fi.ascent + fi.descent;
	const short binMarginTopBot     = scaleBy (binLineHeight, 0.25);
	const short binPaddingTopBot    = scaleBy (binLineHeight, 0.4);

	binRect.bottom  = progRect.top   - binMarginTopBot;
	binRect.top     = binRect.bottom - binPaddingTopBot * 2 - binLineHeight;

	for (int i = 0; i < NUM_ELEMS(binRects); i++) {
		unsigned char *binStr = "\p01";
		binStr[2] = '1' + i;

		binRect.left  = columnMarginLeftRight * (2 + i) + columnWidth * i;
		binRect.right = binRect.left + columnWidth;
		FrameRect (&binRect);
		MoveTo(binRect.left + columnWidth/2 - StringWidth(binStr)/2, binRect.bottom - binPaddingTopBot - fi.descent);
		DrawString(binStr);

		binRects[i] = binRect;
	}

	// Draw the grid

	TextFont (helvetica);
	TextSize (18);
	TextFace (bold);

	GetFontInfo (&fi);
	SetRect (
		&gridRect,
		0,
		headerHeight,
		qd.screenBits.bounds.right,
		binRect.top - binMarginTopBot - 4
	);

	const short gridWidth  = gridRect.right  - gridRect.left;
	const short gridHeight = gridRect.bottom - gridRect.top;

	cellWidth  = gridWidth  / kGridCols;
	cellHeight = gridHeight / kGridRows;

	for (short y = 0; y <  kGridRows; y++)
	for (short x = 0; x <  kGridCols; x++) {
		if (resetNumbers) {
			ChooseNumber (&grid[x][y]);
			ResetNumber (x, y, unselectedNumberSize);
		}
		DrawNumber (&grid[x][y]);
	}

	DrawSeparator (gridRect.bottom, 2);
}

static short SelectNumber (short x, short y) {
	Number *number = &grid[x][y];

	ClipRect(&gridRect);

	TextMode (srcXor);
	DrawNumber (number);
	ResetNumber (number, selectedNumberSize);
	DrawNumber (number);

	ClipRect(&qd.screenBits.bounds);

	return number->digit - '0';
}

static Boolean BinNumbers (Animation* animation, short whichBin) {

	// Search the grid for numbers which are selected and copy them into
	// the animation queue

	short n = 0;

	for (short y = 0; y <  kGridRows; y++)
	for (short x = 0; x <  kGridCols; x++) {
		if (grid[x][y].size != unselectedNumberSize) {
			Number *number = &grid[x][y];

			isAnimating = true;
			animation->number[n++] = number;

			if (n == NUM_ELEMS(animation->number)) {
				goto loopExit;
			}
		}
	}

loopExit:

	if (n) {
		Rect *r = &binRects[whichBin];
		const long xEnd = (r->right  + r->left) / 2;
		const long yEnd = r->bottom + 10;

		animation->tStart = TickCount();
		animation->tDelta = 60;
		animation->whichBin = whichBin;

		for (int i = 0; i < NUM_ELEMS(animation->number); i++) {
			Number *number = animation->number[i];
			if (number) {
				animation->xStart[i] = number->x;
				animation->yStart[i] = number->y;

				animation->xDelta[i] = long2Fix(xEnd) - number->x;
				animation->yDelta[i] = long2Fix(yEnd) - number->y;
			}
		}

		return true;
	}
	return false;
}

static void AnimateNumbers (Animation *animation) {
	unsigned long t = TickCount();

	if (t < animation->tStart) {
		return;
	}

	const Boolean animationInProgress = t < (animation->tStart + animation->tDelta);

	const Fixed f =
		animationInProgress ?
		fixRatio (t - animation->tStart, animation->tDelta) :
		long2Fix (1);

	const unsigned char fy = easeInQuad        [fixMul (f,NUM_ELEMS(easeInQuad    )  - 1)];
	const unsigned char fx = easeOutQuad       [fixMul (f,NUM_ELEMS(easeOutQuad   )  - 1)];
	const unsigned char fb = easeBounceQuint   [fixMul (f,NUM_ELEMS(easeBounceQuint) - 1)];

	Rect clipRect;
	clipRect = qd.screenBits.bounds;
	clipRect.bottom = binRects[0].top;
	ClipRect (&clipRect);

	TextFont (helvetica);
	TextFace (normal);
	TextMode (srcXor);

	short lastSize = 0;
	for (int i = 0; i < NUM_ELEMS(animation->number); i++) {
		Number *number = animation->number[i];
		if (number) {
			// Set the font for the digit
			if (lastSize != number->size) {
				TextSize (number->size);
				TextFace (number->size == unselectedNumberSize ? normal : bold);
				lastSize = number->size;
			}

			// XOR out the old number
			MoveTo (
				fix2Long (number->x),
				fix2Long (number->y)
			);
			DrawChar (number->digit);

			// Update the location
			number->x = animation->xStart[i] + ((animation->xDelta[i] >> easeFuncBits) * fx);
			number->y = animation->yStart[i] + ((animation->yDelta[i] >> easeFuncBits) * fy);

			// XOR in the new number
			MoveTo (
				fix2Long (number->x),
				fix2Long (number->y)
			);
			DrawChar (number->digit);
		}
	}

	// Draw the box flaps

	ClipRect (&qd.screenBits.bounds);
	PenMode (patXor);

	DrawBoxFlaps (animation->whichBin, boxFlaps);
	boxFlaps = fb;
	DrawBoxFlaps (animation->whichBin, boxFlaps);

	if (!animationInProgress) {
		isAnimating = false;

		// Sum the value of the numbers
		short numberSum = 0;
		for (int i = 0; i < NUM_ELEMS(animation->number); i++) {
			if (animation->number[i]) {
				numberSum += animation->number[i]->digit - '0';
			}
		}

		// Reset the numbers
		for (int i = 0; i < NUM_ELEMS(animation->number); i++) {
			if (animation->number[i]) {
				ResetNumber (animation->number[i], unselectedNumberSize);
				ChooseNumber (animation->number[i]);
			}
		}

		// Fade in the numbers. For some reason, grayishTextOr messes with the pen state
		// so it is necessary to save and restore it.

		PenState savedPenState;
		GetPenState( &savedPenState );
		for (int s = 0; s < 2; s++) {
			TextMode (s == 0 ? grayishTextOr : 0);
			for (int i = 0; i < NUM_ELEMS(animation->number); i++) {
				if (animation->number[i]) {
					long tmp;
					DrawNumber (animation->number[i]);
					Delay (2, &tmp);
				}
			}
		}
		SetPenState (&savedPenState);

		// Clear the animation data structure
		for (int i = 0; i < NUM_ELEMS(animation->number); i++) {
			animation->number[i] = NULL;
		}

		// Erase the prior total score (as we are using XOR)
		DrawTotalProgress ();

		// Score the numbers
		short newScore = ((short) binProgress[animation->whichBin]) + numberSum / 30;
		if (newScore < 100) {
			binProgress[animation->whichBin] = newScore + 1;
		} else {
			binProgress[animation->whichBin] = 100;
		}

		// Draw the new scores

		DrawBinProgress (animation->whichBin);
		if (DrawTotalProgress ()) {
			FinishedLevel ();
		}
	}
}

void AnimateWorkMode () {
	if (isAnimating) {
		AnimateNumbers (&animation);
	}

	if (Button()) {
		Point mouseLoc;
		GetMouse (&mouseLoc);

		const short x = (mouseLoc.h - gridRect.left) / cellWidth;
		const short y = (mouseLoc.v - gridRect.top)  / cellHeight;

		if ((x >= 0) && (y >= 0) && (x < kGridCols) && (y < kGridRows)) {
			pendingBin = SelectNumber (x, y) / 2;
		}
	} else if (!isAnimating) {
		isAnimating = BinNumbers (&animation, pendingBin);
	}
}