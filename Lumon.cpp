#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <SegLoad.h>

#include "MathTables.h"

#define NUM_ELEMS(arr) (sizeof(arr) / sizeof((arr)[0]))

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

enum {
	mApple         = 32000,
	mFile          = 32001,
	mEdit          = 32002,

	// Items in mFile
	mQuit          = 9
};

Boolean  gQuit = false;

enum {
	kQuitTries       = 3,
	kBootTicks       = 120,
	kIdleTicks       = 3600,
	kIdleBounceSpeed = 1,

	kGridCols        = 16,
	kGridRows        = 7,
	kBoxFlapsOpen    = 22
};

short headerFontSize, footerFontSize, unselectedNumberSize, selectedNumberSize;

typedef struct {
	char digit;
	char size;
	short res;
	Fixed x, y;
	long res1;
} Number;

const short maxNumbersToAnimate = 12;

typedef struct {
	unsigned long tStart, tDelta;
	Number *number[maxNumbersToAnimate];
	Fixed xStart[maxNumbersToAnimate], yStart[maxNumbersToAnimate];
	Fixed xDelta[maxNumbersToAnimate], yDelta[maxNumbersToAnimate];
	char whichBin;
} Animation;

enum {bootMode, workMode, idleMode} state;
Rect gridRect, binRects[5], progRects[5], logoRect, titleRect;
short cellWidth, cellHeight, progPadding, titlePadding, boxFlaps, pendingBin, level = 1, dxBounce, dyBounce, savedMenuBarHeight;
char binProgress[5] = {0};
Number grid[kGridCols][kGridRows];
Animation animation;
unsigned long gTimer;
WindowPtr mainWindow;
PicHandle gLumonIcon;

Boolean isAnimating = false;

PicHandle GetLumonIcon ();
void GetLogoSize (short *width, short *height);

void DrawLumonStr   (PicHandle lumonIcon);
void DrawLumonGlobe (PicHandle lumonIcon, Rect &logoRect);
void DrawLumonGlobe (PicHandle lumonIcon, short x, short y, Rect *logoRect = 0);

void Initialize ();
void ShowBootLogo ();
void StartWorkMode ();
void StartIdleMode ();
void AwakeFromIdleMode ();
void AnimateIdleMode ();
void DoEvent (EventRecord *event);
void DoMenuSelection (long choice);
void DoMouseAction ();
void AutoHideMenuBar ();
void AddMenuToWindowVisibilityRegion ();

void DrawAll ();
void DrawWorkMode (Boolean resetNumbers = false);
void DrawSeparator (short y, short separation);
void DrawNumber (Number *);
void DrawBinProgress (short which);
Boolean DrawTotalProgress ();
void ResetNumber (Number *number, short size);
void ResetNumber (short x, short y, short size);
void ChooseNumber (Number *);
void BinNumbers (Animation* animation, short whichBin);
void DrawBoxFlaps (short whichBox, unsigned char factor);
void AnimateNumbers (Animation *animation);
void FinishedLevel ();
void DoAnimation ();

void main(void) {
	Point currentMousePoint, lastMousePoint;

	Initialize ();

	gLumonIcon = GetLumonIcon();
	gTimer = TickCount();

	ShowBootLogo ();

	while (!gQuit) {
		AutoHideMenuBar ();

		EventRecord event;
		SystemTask();
		if (GetNextEvent(everyEvent, &event)) {
			DoEvent(&event);
		}

		switch (state) {
			case bootMode:
				if ((TickCount() - gTimer) > kBootTicks) {
					StartWorkMode ();
				}
				break;
			case workMode:
						DoMouseAction();
				DoAnimation();
				if (
					(FrontWindow() == mainWindow) &&
					((TickCount() - gTimer) > kIdleTicks)
				) {
					HideCursor ();
					StartIdleMode ();
					GetMouse (&lastMousePoint);
				}
				break;
			case idleMode: {
				AnimateIdleMode ();
				GetMouse (&currentMousePoint);
				if (!EqualPt(currentMousePoint, lastMousePoint)) {
					ShowCursor ();
					StartWorkMode ();
					gTimer = TickCount();
				}
				break;
			}
		}
	}

	if (state == idleMode) {
		ShowCursor ();
	}

	KillPicture (gLumonIcon);
}

void DoEvent(EventRecord *event) {
	GrafPtr savedPort;
	WindowPtr window;   /* for handling window events */

	switch (event->what) {
		case updateEvt:
			window = (WindowPtr) event->message;
			if (window && (window == mainWindow)) {
				BeginUpdate (window);
				AddMenuToWindowVisibilityRegion ();
				DrawAll ();
				EndUpdate (window);
				AddMenuToWindowVisibilityRegion ();
			}
			break;
		case mouseDown:
			switch (FindWindow(event->where, &window)) {
				case inDrag:
					AwakeFromIdleMode ();
					Rect dragRect = (**GetGrayRgn()).rgnBBox;
					DragWindow(window, event->where, &dragRect);
					break;
				case inSysWindow:
					AwakeFromIdleMode ();
					SystemClick(event, window);
					break;
				case inGoAway:
					break;
				case inMenuBar:
					AwakeFromIdleMode ();
					DrawMenuBar();
					DoMenuSelection(MenuSelect(event->where));
					gTimer = TickCount();
					break;
				case inContent:
					/*if (window != FrontWindow()) {
						SelectWindow(window);
						event->what = nullEvent;
					}*/
					break;
			}
			break;
		case keyDown:
		case autoKey:
			AwakeFromIdleMode ();
			if ((event->modifiers & cmdKey) != 0) {
				const char key = event->message & charCodeMask;
				const long menuResult = MenuKey (key);
				if (menuResult) {
					DoMenuSelection (menuResult);
				}
			}
			break;
	}
}

void DoMenuSelection(long choice) {
	Str255 daName;
	const int        menuId  = HiWord(choice);
	const int        itemNum = LoWord(choice);
	const MenuHandle hMenu   = GetMenuHandle(menuId);

	switch (menuId)  {
		case mApple:
			switch (itemNum) {
				case 1:
					Alert (128, NULL);
					break;
				default:
					GetMenuItemText( hMenu, itemNum, daName );
					OpenDeskAcc( daName );
					break;
			}
			break;
		case mFile:
			static triesRemaining = kQuitTries;
			if (itemNum == mQuit) {
				if (triesRemaining--) {
					Alert (129, NULL);
				} else {
					gQuit = true;
				}
			}
			break;
	}
	HiliteMenu(0);
}

void Initialize () {
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	// Initialize the random number generator

	GetDateTime((unsigned long*) &qd.randSeed);

	// Create our main window. Although we won't be drawing to it, it is
	// still necessary to ensure the desktop gets painted correctly in
	// a MultiFinder environment.

	Rect windRect = qd.screenBits.bounds;
	mainWindow = NewWindow( nil, &windRect, "\pLumon Industries", true, plainDBox,
							(WindowPtr) -1, false, 0 );

	SetPort (mainWindow);

	savedMenuBarHeight = LMGetMBarHeight();

	AddMenuToWindowVisibilityRegion ();

	// Set up the menu bar

	Handle ourMenuBar = GetNewMBar( 128 );
	SetMenuBar( ourMenuBar );
	AppendResMenu( GetMenuHandle( mApple ), 'DRVR' );
	ReleaseResource( ourMenuBar );
	DrawMenuBar();
}

// Add the menu bar to the window visibility region

void AddMenuToWindowVisibilityRegion () {
	RgnHandle menuBarRgn;
	Rect menuRect = qd.screenBits.bounds;
	menuRect.bottom = menuRect.top + savedMenuBarHeight;

	menuBarRgn = NewRgn();
	RectRgn (menuBarRgn, &menuRect);

	UnionRgn (mainWindow->visRgn, menuBarRgn, mainWindow->visRgn);
	DisposeRgn (menuBarRgn);
}

void AutoHideMenuBar () {
	static Boolean menuBarVisible = true;

	Point mouseLoc;
	GetMouse (&mouseLoc);

	Boolean cursorInMenuBar = mouseLoc.v < savedMenuBarHeight;

	if (menuBarVisible != cursorInMenuBar) {
		if (mouseLoc.v < savedMenuBarHeight) {
			LMSetMBarHeight (savedMenuBarHeight);
			menuBarVisible = true;
		} else {
			LMSetMBarHeight (0);
			menuBarVisible = false;
		}
	}
}

void ShowBootLogo () {
	Rect myRect = qd.screenBits.bounds;

	BackColor( blackColor );
	ForeColor( whiteColor );

	EraseRect (&myRect);
	TextSize (48);
	DrawLumonGlobe (gLumonIcon, myRect.right / 2, myRect.bottom / 2);
	state = bootMode;
}

void StartWorkMode () {
	BackColor( blackColor );
	ForeColor( whiteColor );
	EraseRect (&qd.screenBits.bounds);

	DrawWorkMode (true);

	state = workMode;
}

void StartIdleMode () {
	SelectWindow (mainWindow);

	BackColor( blackColor );
	ForeColor( whiteColor );

	EraseRect (&qd.screenBits.bounds);

	TextSize (48);

	short logoWidth, logoHeight;
	GetLogoSize (&logoWidth, &logoHeight);

	// Choose a random initial position of the logo which is fully inside the screen

	short centerX = kIdleBounceSpeed + logoWidth  / 2 + (unsigned short)Random() % (qd.screenBits.bounds.right  - logoWidth  - kIdleBounceSpeed * 2);
	short centerY = kIdleBounceSpeed + logoHeight / 2 + (unsigned short)Random() % (qd.screenBits.bounds.bottom - logoHeight - kIdleBounceSpeed * 2);
	dxBounce = kIdleBounceSpeed;
	dyBounce = kIdleBounceSpeed;

	DrawLumonGlobe (gLumonIcon, centerX, centerY, &logoRect);

	// Give the logo region a black border so CopyBits doesn't leave a trail

	InsetRect (&logoRect, -kIdleBounceSpeed, -kIdleBounceSpeed);

	// Reset these colors to avoid artifacting with CopyBits on color Macs

	BackColor( whiteColor );
	ForeColor( blackColor );

	state = idleMode;
}

void AnimateIdleMode () {
	Rect newRect = logoRect;

	OffsetRect (&newRect, dxBounce, dyBounce);

	if ((newRect.right > qd.screenBits.bounds.right) || (newRect.left < qd.screenBits.bounds.left)) {
		dxBounce = -dxBounce;
		OffsetRect (&newRect, dxBounce * 2, 0);
	}
	if ((newRect.bottom > qd.screenBits.bounds.bottom) || (newRect.top < qd.screenBits.bounds.top)) {
		dyBounce = -dyBounce;
		OffsetRect (&newRect, 0, dyBounce * 2);
	}

	CopyBits (&qd.screenBits, &qd.screenBits, &logoRect, &newRect, srcCopy, NULL);
	logoRect = newRect;
}

void AwakeFromIdleMode () {
	if (state == idleMode) {
		ShowCursor();
		StartWorkMode ();
	}
}

void DrawSeparator(short y, short separation) {
	MoveTo (0, y - separation);
	LineTo(qd.screenBits.bounds.right, y - separation);
	MoveTo (0, y + separation);
	LineTo(qd.screenBits.bounds.right, y + separation);
}

const unsigned char *logoStr = "\pLUMON";

// Returns a QuickDraw picture with the Lumon icon

PicHandle GetLumonIcon() {
	Rect myRect;

	BackColor( blackColor );
	ForeColor( whiteColor );

	RgnHandle rgn1 = NewRgn();
	OpenRgn();
		SetRect (&myRect, 47, 12, 53, 18);
		FrameOval(&myRect);

		SetRect (&myRect, 35, 45, 65, 85);
		FrameOval(&myRect);
	CloseRgn (rgn1);

	Point savedPenLoc;
	GetPen (&savedPenLoc);

	RgnHandle rgn2 = NewRgn();
	OpenRgn();
		MoveTo (47, 15);
		LineTo (35, 65);
		LineTo (65, 65);
		LineTo (53, 15);
		LineTo (47, 15);
	CloseRgn (rgn2);
	UnionRgn (rgn1, rgn2, rgn1);

	MoveTo (savedPenLoc.h, savedPenLoc.v);

	SetRect (&myRect, 0, 0, 100, 100);
	ClipRect (&myRect);

	PicHandle myIcon = OpenPicture (&myRect);
	PaintRoundRect(&myRect, 50, 50);
	EraseRgn (rgn1);
	ClosePicture();

	DisposeRgn (rgn1);
	DisposeRgn (rgn2);
	ClipRect (&qd.screenBits.bounds);

	return myIcon;
}

// Draws the string "LUMON", but substitutes the 'O' with the Lumon icon, which
// should be provided to this function

void DrawLumonStr (PicHandle lumonIcon) {
	TextFont (helvetica);
	TextFace (bold);

	FontInfo fi;
	GetFontInfo (&fi);

	DrawChar ('L');
	DrawChar ('U');
	DrawChar ('M');
	const oWidth = CharWidth('O');

	Point penPos;
	Rect iconRect;
	GetPen (&penPos);
	SetRect (&iconRect, penPos.h, penPos.v - fi.ascent * 0.9, penPos.h + oWidth, penPos.v);

	DrawPicture (lumonIcon, &iconRect);

	Move(oWidth,0);
	DrawChar ('N');
}

void GetLogoSize (short *width, short *height) {
	TextFont (helvetica);
	TextFace (bold);

	FontInfo fi;
	GetFontInfo (&fi);

	const short logoLineHeight = fi.ascent + fi.descent;
	const short logoPadding    = logoLineHeight * 0.25;

	*width  = (StringWidth (logoStr) - 2) / 4 * 6;
	*height = (logoLineHeight + logoPadding * 2) * 2;
}

void DrawLumonGlobe (PicHandle lumonIcon, Rect &logoRect) {
	EraseOval (&logoRect);
	FrameOval (&logoRect);

	// Draw latitude lines

	const short ph = qd.thePort->pnSize.h;
	const short pv = qd.thePort->pnSize.v;

	const short logoWidth  = logoRect.right  - logoRect.left;
	const short logoHeight = logoRect.bottom - logoRect.top;

	// Foreshortening of latitude lines halfway up from equator to pole
	const short f = logoWidth * 0.06698; // logoWidth / 2 * (1 - sqrt(0.75))

	MoveTo (logoRect.left  + f,      logoRect.top + logoHeight / 4);
	LineTo (logoRect.right - f - ph, logoRect.top + logoHeight / 4);

	MoveTo (logoRect.left  + f,      logoRect.top + logoHeight / 4 * 3 - pv);
	LineTo (logoRect.right - f - ph, logoRect.top + logoHeight / 4 * 3 - pv);

	// Draw longitude lines

	const short step = logoWidth / 6;

	Rect tmp = logoRect;
	InsetRect (&tmp, step, 0);
	FrameOval (&tmp);

	InsetRect (&tmp, step, 0);
	FrameOval (&tmp);

	// Draw the text
	TextFont (helvetica);
	TextFace (bold);

	FontInfo fi;
	GetFontInfo (&fi);

	const short lineHeight  = fi.ascent + fi.descent;
	const short strWidth    = StringWidth (logoStr);

	MoveTo (
		logoRect.left + logoWidth/2 - strWidth/2,
		logoRect.top  + logoHeight/2 + fi.ascent/2
	);

	TextMode (srcCopy);
	DrawLumonStr (lumonIcon);
}

void DrawLumonGlobe (PicHandle lumonIcon, short x, short y, Rect *logoRect) {
	short logoWidth, logoHeight;
	Rect myRect;

	TextFont (helvetica);
	TextFace (bold);

	GetLogoSize (&logoWidth, &logoHeight);

	SetRect (&myRect,
		x - logoWidth  / 2,
		y - logoHeight / 2,
		x + logoWidth / 2,
		y + logoHeight / 2
	);
	DrawLumonGlobe (lumonIcon, myRect);

	if (logoRect) {
		*logoRect = myRect;
	}
}

void DrawBinProgress (short which) {
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

Boolean DrawTotalProgress () {
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

void DrawAll () {
	switch (state) {
		case workMode:
			DrawWorkMode ();
			break;
	}
}

void DrawWorkMode (Boolean resetNumbers) {
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
	const short titleMargin     = titleLineHeight * 1.25;
	titlePadding                = titleLineHeight * 0.25;
	short logoWidth, logoHeight;

	GetLogoSize (&logoWidth, &logoHeight);

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

	DrawLumonGlobe (gLumonIcon, logoRect);
	DrawTotalProgress ();

	// Draw the footer

	TextFont (helvetica);
	TextSize (footerFontSize);
	TextFace (bold);

	GetFontInfo (&fi);
	GetIndString (myStr, 128, 2);
	const short footerLineHeight    = fi.ascent + fi.descent;
	const short footerPadding       = footerLineHeight * 0.2;
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
	const short progMarginTopBot       = progLineHeight * 0.25;
	progPadding                        = progLineHeight * 0.25;
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
	const short binMarginTopBot     = binLineHeight * 0.25;
	const short binPaddingTopBot    = binLineHeight * 0.4;

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

void DrawNumber (Number *number) {
	TextFont (helvetica);
	TextFace (number->size == unselectedNumberSize ? normal : bold);
	TextSize (number->size);
	MoveTo (
		fix2Long (number->x),
		fix2Long (number->y)
	);
	DrawChar (number->digit);
}

void ResetNumber (Number *number, short size) {
	for (short y = 0; y <  kGridRows; y++)
	for (short x = 0; x <  kGridCols; x++) {
		if (number == &grid[x][y]) {
			ResetNumber (x,y, size);
		}
	}
}

void ResetNumber (short x, short y, short size) {
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

void AnimateNumbers (Animation *animation) {
	unsigned long t = TickCount();

	if (t < animation->tStart) {
		return;
	}

	const Boolean animationInProgress = t < (animation->tStart + animation->tDelta);

	const Fixed f =
		animationInProgress ?
		fixRatio (t - animation->tStart, animation->tDelta) :
		long2Fix (1);

	const unsigned char fy = easeInQuad        [fixMul(f,NUM_ELEMS(easeInQuad    )  - 1)];
	const unsigned char fx = easeOutQuad       [fixMul(f,NUM_ELEMS(easeOutQuad   )  - 1)];
	const unsigned char fb = easeBounceQuint   [fixMul(f,NUM_ELEMS(easeBounceQuint) - 1)];

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

		// Fade in the numbers
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
		PenPat (&qd.black);

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

void FinishedLevel () {
	binProgress[0] = binProgress[1] = binProgress[2] = binProgress[3] = binProgress[4] = 0;
	level++;
	DrawWorkMode ();
}

void BinNumbers (Animation* animation, short whichBin) {
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
}

// Draws the box flaps, the easing ranges from 0 to 255, zero being fully closed,
// 255 fully open.

void DrawBoxFlaps (short whichBox, unsigned char easing) {
	Rect boxRect = binRects[whichBox];

	if (easing == 0) return;

	long angle = (unsigned long)easing * kBoxFlapsOpen >> easeFuncBits;

	const short flapLength = (boxRect.right - boxRect.left) / 2;
	const short angleShifted = (angle + NUM_ELEMS(sineTable) / 4) % NUM_ELEMS(sineTable);
	long   sine = ((long)sineTable[ angle        ] * flapLength) >> trigFuncBits;
	long cosine = ((long)sineTable[ angleShifted ] * flapLength) >> trigFuncBits;

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

void ChooseNumber (Number *number) {
	number->digit = '0' + ((unsigned short)Random() % 10);
}

void DoMouseAction () {
	if (Button()) {
		Point mouseLoc;
		GetMouse (&mouseLoc);

		const short x = (mouseLoc.h - gridRect.left) / cellWidth;
		const short y = (mouseLoc.v - gridRect.top)  / cellHeight;

		if ((x >= 0) && (y >= 0) && (x < kGridCols) && (y < kGridRows)) {
			Number *number = &grid[x][y];

			ClipRect(&gridRect);

			TextMode (srcXor);
			DrawNumber (number);
			ResetNumber (number, selectedNumberSize);
			DrawNumber (number);

			ClipRect(&qd.screenBits.bounds);

			pendingBin = (number->digit - '0') / 2;
		}

		gTimer = TickCount(); // Reset the idle timer
	} else {
		if (!isAnimating) {
			short n = 0;

			for (short y = 0; y <  kGridRows; y++)
			for (short x = 0; x <  kGridCols; x++) {
				if (grid[x][y].size != unselectedNumberSize) {
					Number *number = &grid[x][y];

					isAnimating = true;
					animation.number[n++] = number;
					if (n == NUM_ELEMS(animation.number)) {
						goto loopExit;
					}
				}
			}

		loopExit:

			if (isAnimating) {
				BinNumbers (&animation, pendingBin);
			}
		}
	}
}

void DoAnimation() {
	if (isAnimating) {
		AnimateNumbers (&animation);
	}
}