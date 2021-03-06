/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * SDL screen functions header file.
 *
 * @version $Revision$
 */
#include "SDL.h"
#include "vector.h"

#if !PSP_SCREEN
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define LAYER_WIDTH 320
#define LAYER_HEIGHT 480
#define PANEL_WIDTH 160
#define PANEL_HEIGHT 480
#else
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define LAYER_WIDTH 320
#define LAYER_HEIGHT 272
#define PANEL_WIDTH 80
#define PANEL_HEIGHT 272
#endif

#define SCAN_WIDTH 320
#if !PSP_SCREEN
#define SCAN_HEIGHT 480
#else
#define SCAN_HEIGHT 272
#endif
#define SCAN_WIDTH_8 (SCAN_WIDTH<<8)
#define SCAN_HEIGHT_8 (SCAN_HEIGHT<<8)

#define BPP 8
#define LayerBit Uint8

#define PAD_UP 1
#define PAD_DOWN 2
#define PAD_LEFT 4
#define PAD_RIGHT 8
#define PAD_BUTTON1 16
#define PAD_BUTTON2 32
#define PAD_START 64

#define DEFAULT_BRIGHTNESS 224

extern int windowMode;
extern LayerBit *l1buf, *l2buf;
extern LayerBit *buf;
extern SDL_Surface *layer, *lpanel, *rpanel, *video, *l1, *l2, *p;
extern SDL_Rect lpanelRect, rpanelRect;
extern LayerBit *lpbuf, *rpbuf;
extern Uint8 *keys;
extern SDL_Joystick *stick;
extern int buttonReversed;
extern int brightness;

void initSDL(int window);
void closeSDL();
void blendScreen();
void flipScreen();
void clearScreen();
void clearLPanel();
void clearRPanel();
void smokeScreen();
void drawThickLine(int x1, int y1, int x2, int y2, LayerBit color1, LayerBit color2, int width);
void drawLine(int x1, int y1, int x2, int y2, LayerBit color, int width, LayerBit *buf);
void drawBox(int x, int y, int width, int height, 
	     LayerBit color1, LayerBit color2, SDL_Surface *dst);
void drawBoxPanel(int x, int y, int width, int height, 
		  LayerBit color1, LayerBit color2, LayerBit *buf);
int drawNum(int n, int x ,int y, int s, int c1, int c2);
int drawNumRight(int n, int x ,int y, int s, int c1, int c2);
int drawNumCenter(int n, int x ,int y, int s, int c1, int c2);
void drawSprite(int n, int x, int y);

int getPadState();
int getButtonState();
