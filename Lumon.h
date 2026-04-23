#pragma once

#include <Types.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Windows.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>

//#define ACCENT_COLOR yellowColor

extern short globeFontSize;
extern short gPenSize;
extern Rect  gScreenBounds;

void SetupFonts();

void StartWorkMode ();
void StartIdleMode ();

void DrawBootLogo ();
void DrawIdleMode ();
void DrawWorkMode (Boolean resetNumbers = false);

Boolean AnimateWorkMode ();
void AnimateIdleMode ();