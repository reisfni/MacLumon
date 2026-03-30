#pragma once

extern PicHandle gLumonIcon;

void StartWorkMode ();
void StartIdleMode ();

void DrawBootLogo ();
void DrawIdleMode ();
void DrawWorkMode (Boolean resetNumbers = false);

void AnimateWorkMode ();
void AnimateIdleMode ();