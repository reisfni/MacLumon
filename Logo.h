#pragma once

PicHandle GetLumonIcon();

void GetGlobeSize (short *width, short *height);
void GetGlobeRect (Rect *logoRect, short x, short y);

void DrawLumonStr (PicHandle lumonIcon);
void DrawLumonGlobe (const Rect &logoRect, PicHandle lumonIcon);
