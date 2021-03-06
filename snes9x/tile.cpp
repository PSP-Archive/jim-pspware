/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman
 
  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se) 

 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/
#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "display.h"
#include "gfx.h"
#include "tile.h"

#include "profiler.h"

#ifdef USE_GLIDE
#include "3d.h"
#endif

#ifdef PSP
extern "C" {
void debug_log( const char* message );
};
#endif // PSP

extern uint32 HeadMask [4];
extern uint32 TailMask [5];

uint8 ConvertTile (uint8 *pCache, uint32 TileAddr)
{
    register uint8 *tp = &Memory.VRAM[TileAddr];
    uint32 *p = (uint32 *) pCache;
    uint32 non_zero = 0;
    uint8 line;

    switch (BG.BitShift)
    {
    case 8:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;

	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 32)))
	    {
		p1 |= odd_high[2][pix >> 4];
		p2 |= odd_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 33)))
	    {
		p1 |= even_high[2][pix >> 4];
		p2 |= even_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 48)))
	    {
		p1 |= odd_high[3][pix >> 4];
		p2 |= odd_low[3][pix & 0xf];
	    }
	    if ((pix = *(tp + 49)))
	    {
		p1 |= even_high[3][pix >> 4];
		p2 |= even_low[3][pix & 0xf];
	    }
	    *p++ = p1;
	    *p++ = p2;
	    non_zero |= p1 | p2;
	}
	break;

    case 4:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    *p++ = p1;
	    *p++ = p2;
	    non_zero |= p1 | p2;
	}
	break;

    case 2:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    *p++ = p1;
	    *p++ = p2;
	    non_zero |= p1 | p2;
	}
	break;
    }
    return (non_zero ? TRUE : BLANK_TILE);
}

#ifndef OPTI
inline void WRITE_4PIXELS (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS_FLIPPED (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELSx2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel(N*2+1, Pixel); \
	TILE_SetPixel(N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS_FLIPPEDx2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel(N*2+1, Pixel); \
	TILE_SetPixel(N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELSx2x2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel(N*2, Pixel); \
	TILE_SetPixel(N*2+1, Pixel); \
	TILE_SetPixel(GFX.RealPitch+N*2, Pixel); \
	TILE_SetPixel(GFX.RealPitch+N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS_FLIPPEDx2x2 (uint32 Offset, uint8 *Pixels)
{
    uint8 Pixel;
    uint8 *Screen = GFX.S + Offset;
    uint8 *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel(N*2, Pixel); \
	TILE_SetPixel(N*2+1, Pixel); \
	TILE_SetPixel(GFX.RealPitch+N*2, Pixel); \
	TILE_SetPixel(GFX.RealPitch+N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

void DrawTile (uint32 Tile, uint32 Offset, uint32 StartLine,
	       uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile);

    TILE_PREAMBLE

    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS, WRITE_4PIXELS_FLIPPED, 4)

    FINISH_PROFILE_TILE_FUNC (DrawTile);
}

void DrawClippedTile (uint32 Tile, uint32 Offset,
		      uint32 StartPixel, uint32 Width,
		      uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile);

    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS, WRITE_4PIXELS_FLIPPED, 4)

    FINISH_PROFILE_TILE_FUNC (DrawClippedTile);
}

void DrawTilex2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		 uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile);

    TILE_PREAMBLE

    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELSx2, WRITE_4PIXELS_FLIPPEDx2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawTilex2);
}

void DrawClippedTilex2 (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawClippedTilex2);

    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELSx2, WRITE_4PIXELS_FLIPPEDx2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawClippedTilex2);
}

void DrawTilex2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTilex2x2);

    TILE_PREAMBLE

    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELSx2x2, WRITE_4PIXELS_FLIPPEDx2x2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawTilex2x2);
}

void DrawClippedTilex2x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawClippedTilex2x2);

    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELSx2x2, WRITE_4PIXELS_FLIPPEDx2x2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawClippedTilex2x2);
}
#endif // OPTI

inline void WRITE_4PIXELS16 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS16_FLIPPED (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS16x2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16(N*2, Pixel); \
	TILE_SetPixel16(N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS16_FLIPPEDx2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel16(N*2, Pixel); \
	TILE_SetPixel16(N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS16x2x2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[N])) \
    { \
	TILE_SetPixel16(N*2, Pixel); \
	TILE_SetPixel16(N*2+1, Pixel); \
	TILE_SetPixel16((GFX.RealPitch>>1)+N*2, Pixel); \
	TILE_SetPixel16((GFX.RealPitch>>1)+N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

inline void WRITE_4PIXELS16_FLIPPEDx2x2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SetPixel16(N*2, Pixel); \
	TILE_SetPixel16(N*2+1, Pixel); \
	TILE_SetPixel16((GFX.RealPitch>>1)+N*2, Pixel); \
	TILE_SetPixel16((GFX.RealPitch>>1)+N*2+1, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)
#undef FN
}

#ifdef OPTI
void DrawTile16_OBJ (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile16_OBJ);
    
//	TILE_PREAMBLE

    uint8 *pCache;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << 5);
    if ((Tile & 0x1ff) >= 256){
		TileAddr += BG.NameSelect;
	}

    TileAddr &= 0xffff;

    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> 5)) << 6];

    if (!BG.Buffered [TileNumber]){
		BG.Buffered[TileNumber] = ConvertTile (pCache, TileAddr);
	}

    if (BG.Buffered [TileNumber] == BLANK_TILE){
		TileBlank = Tile & 0xFFFFFFFF;
		return;
	}

	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & 7) << 4) + 128];

    register uint8*	bp;
	register int	inc;

    if (Tile & V_FLIP){
		bp  = pCache + 56 - StartLine;
		inc = -8;
	} else {
		bp  = pCache + StartLine;
		inc = 8;
	}

    uint16*	Screen = (uint16 *) GFX.S + Offset;
	uint16* Colors =  GFX.ScreenColors;
    uint8*	Depth = GFX.DB + Offset;
	int		GFX_PPL = GFX.PPL;
	int		GFX_Z1  = GFX.Z1;
	int		GFX_Z2  = GFX.Z2;

    if (!(Tile & H_FLIP)){
#define FN(N) \
    if (GFX_Z1 > Depth[N] && bp[N]){ \
		Screen[N] = Colors[bp[N]]; \
		Depth[N]  = GFX_Z2; \
	}
		while ( LineCount-- ){
		    if ( *(uint32*)bp ){
				FN(0); FN(1); FN(2); FN(3);
			}

		    if ( *(uint32 *)(bp + 4) ){
				FN(4); FN(5); FN(6); FN(7);
			}
			bp += inc;
			Screen += GFX_PPL;
			Depth  += GFX_PPL;
		}
#undef FN
	} else {
#define FN(N, B) \
	if (GFX_Z1 > Depth[N] && bp[B]){ \
		Screen[N] = Colors[bp[B]]; \
		Depth[N]  = GFX_Z2; \
    }
		while ( LineCount-- ){
		    if ( *(uint32 *)(bp + 4) ){
				FN(0, 7); FN(1, 6); FN(2, 5); FN(3, 4);
			}

		    if ( *(uint32*)bp ){
				FN(4, 3); FN(5, 2); FN(6, 1); FN(7, 0);
			}
			bp += inc;
			Screen += GFX_PPL;
			Depth  += GFX_PPL;
		}
#undef FN
	}

     FINISH_PROFILE_TILE_FUNC (DrawTile16_OBJ);
}

void DrawTile16 (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile16);
    
//	TILE_PREAMBLE

    uint8 *pCache;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);

    TileAddr &= 0xffff;

    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) << 6];

    if (!BG.Buffered [TileNumber]){
		BG.Buffered[TileNumber] = ConvertTile (pCache, TileAddr);
	}

    if (BG.Buffered [TileNumber] == BLANK_TILE){
		TileBlank = Tile & 0xFFFFFFFF;
		return;
	}

    if (BG.DirectColourMode){
		if (IPPU.DirectColourMapsNeedRebuild){
			S9xBuildDirectColourMaps ();
		}
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
    } else {
		GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
	}

    register uint8*	bp;
	register int	inc;

    if (Tile & V_FLIP){
		bp  = pCache + 56 - StartLine;
		inc = -8;
	} else {
		bp  = pCache + StartLine;
		inc = 8;
	}

    uint16*	Screen = (uint16 *) GFX.S + Offset;
	uint16* Colors =  GFX.ScreenColors;
    uint8*	Depth = GFX.DB + Offset;
	int		GFX_PPL = GFX.PPL;
	int		GFX_Z1  = GFX.Z1;
//	int		GFX_Z2  = GFX.Z2;

    if (!(Tile & H_FLIP)){
#define FN(N) \
    if (GFX_Z1 > Depth[N] && bp[N]){ \
		Screen[N] = Colors[bp[N]]; \
		Depth[N]  = GFX_Z1; \
	}
		while ( LineCount-- ){
		    if ( *(uint32*)bp ){
				FN(0); FN(1); FN(2); FN(3);
			}

		    if ( *(uint32 *)(bp + 4) ){
				FN(4); FN(5); FN(6); FN(7);
			}
			bp += inc;
			Screen += GFX_PPL;
			Depth  += GFX_PPL;
		}
#undef FN
	} else {
#define FN(N, B) \
	if (GFX_Z1 > Depth[N] && bp[B]){ \
		Screen[N] = Colors[bp[B]]; \
		Depth[N]  = GFX_Z1; \
    }
		while ( LineCount-- ){
		    if ( *(uint32 *)(bp + 4) ){
				FN(0, 7); FN(1, 6); FN(2, 5); FN(3, 4);
			}

		    if ( *(uint32*)bp ){
				FN(4, 3); FN(5, 2); FN(6, 1); FN(7, 0);
			}
			bp += inc;
			Screen += GFX_PPL;
			Depth  += GFX_PPL;
		}
#undef FN
	}

    FINISH_PROFILE_TILE_FUNC (DrawTile16);
}

#else
void DrawTile16 (uint32 Tile, uint32 Offset, uint32 StartLine,
	         uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile16);
    
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16, WRITE_4PIXELS16_FLIPPED, 4)

    FINISH_PROFILE_TILE_FUNC (DrawTile16);
}
#endif // OPTI

void DrawClippedTile16 (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawClippedTile16);
    
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16, WRITE_4PIXELS16_FLIPPED, 4)

    FINISH_PROFILE_TILE_FUNC (DrawClippedTile16);
}

void DrawTile16x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile16x2);
    
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16x2, WRITE_4PIXELS16_FLIPPEDx2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawTile16x2);
}

void DrawClippedTile16x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawClippedTile16x2);
    
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16x2, WRITE_4PIXELS16_FLIPPEDx2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawClippedTile16x2);
}

void DrawTile16x2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		     uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile16x2x2);
    
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16x2x2, WRITE_4PIXELS16_FLIPPEDx2x2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawTile16x2x2);
}

void DrawClippedTile16x2x2 (uint32 Tile, uint32 Offset,
			    uint32 StartPixel, uint32 Width,
			    uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawClippedTile16x2x2);
    
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16x2x2, WRITE_4PIXELS16_FLIPPEDx2x2, 8)

    FINISH_PROFILE_TILE_FUNC (DrawClippedTile16x2x2);
}

inline void WRITE_4PIXELS16_ADD (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectAddPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_FLIPPED_ADD (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectAddPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_ADD1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_FLIPPED_ADD1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_SUB (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectSubPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_FLIPPED_SUB (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectSubPixel16(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_SUB1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_FLIPPED_SUB1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}


void DrawTile16Add (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile16Add);
    
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_ADD, WRITE_4PIXELS16_FLIPPED_ADD, 4)

    FINISH_PROFILE_TILE_FUNC (DrawTile16Add);
}

void DrawClippedTile16Add (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawClippedTile16Add);
    
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_ADD, WRITE_4PIXELS16_FLIPPED_ADD, 4)

    FINISH_PROFILE_TILE_FUNC (DrawClippedTile16Add);
}

void DrawTile16Add1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawTile1Add1_2);
    
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_ADD1_2, WRITE_4PIXELS16_FLIPPED_ADD1_2, 4)

    FINISH_PROFILE_TILE_FUNC (DrawTile16Add1_2);
}

void DrawClippedTile16Add1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_ADD1_2, WRITE_4PIXELS16_FLIPPED_ADD1_2, 4)
}

void DrawTile16Sub (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4)
}

void DrawClippedTile16Sub (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4)
}

void DrawTile16Sub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_SUB1_2, WRITE_4PIXELS16_FLIPPED_SUB1_2, 4)
}

void DrawClippedTile16Sub1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUB1_2, WRITE_4PIXELS16_FLIPPED_SUB1_2, 4)
}

inline void WRITE_4PIXELS16_ADDF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectFAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_FLIPPED_ADDF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectFAddPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_SUBF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N])) \
    { \
	TILE_SelectFSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

inline void WRITE_4PIXELS16_FLIPPED_SUBF1_2 (uint32 Offset, uint8 *Pixels)
{
    uint32 Pixel;
    uint16 *Screen = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint8  *SubDepth = GFX.SubZBuffer + Offset;

#define FN(N) \
    if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N])) \
    { \
	TILE_SelectFSubPixel16Half(N, Pixel); \
    }

    FN(0)
    FN(1)
    FN(2)
    FN(3)

#undef FN
}

void DrawTile16FixedAdd1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_ADDF1_2, WRITE_4PIXELS16_FLIPPED_ADDF1_2, 4)
}

void DrawClippedTile16FixedAdd1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_ADDF1_2, 
			WRITE_4PIXELS16_FLIPPED_ADDF1_2, 4)
}

void DrawTile16FixedSub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    RENDER_TILE(WRITE_4PIXELS16_SUBF1_2, WRITE_4PIXELS16_FLIPPED_SUBF1_2, 4)
}

void DrawClippedTile16FixedSub1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUBF1_2, 
			WRITE_4PIXELS16_FLIPPED_SUBF1_2, 4)
}

void DrawLargePixel (uint32 Tile, uint32 Offset,
		     uint32 StartPixel, uint32 Pixels,
		     uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawLargePixel);

    TILE_PREAMBLE

    register uint8 *sp = GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
    uint8 pixel;
#define PLOT_PIXEL(screen, pixel) (pixel)

    RENDER_TILE_LARGE (((uint8) GFX.ScreenColors [pixel]), PLOT_PIXEL)

    FINISH_PROFILE_TILE_FUNC (DrawLargePixel);
}

void DrawLargePixel16 (uint32 Tile, uint32 Offset,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount)
{
    START_PROFILE_TILE_FUNC (DrawLargePixel16);
    
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.DB + Offset;
    uint16 pixel;

    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], PLOT_PIXEL)

    FINISH_PROFILE_TILE_FUNC (DrawLargePixel16);
}

void DrawLargePixel16Add (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_ADD_PIXEL(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_ADD (p, *(s + GFX.Delta))    : \
			       COLOR_ADD (p, GFX.FixedColour)) \
			    : p)
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_ADD_PIXEL)
}

void DrawLargePixel16Add1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_ADD_PIXEL1_2(s, p) \
((uint16) (Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_ADD1_2 (p, *(s + GFX.Delta))    : \
			       COLOR_ADD (p, GFX.FixedColour)) \
			    : p))
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_ADD_PIXEL1_2)
}

void DrawLargePixel16Sub (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_SUB_PIXEL(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_SUB (p, *(s + GFX.Delta))    : \
			       COLOR_SUB (p, GFX.FixedColour)) \
			    : p)
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_SUB_PIXEL)
}

void DrawLargePixel16Sub1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE

    register uint16 *sp = (uint16 *) GFX.S + Offset;
    uint8  *Depth = GFX.ZBuffer + Offset;
    uint16 pixel;

#define LARGE_SUB_PIXEL1_2(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
			       COLOR_SUB1_2 (p, *(s + GFX.Delta))    : \
			       COLOR_SUB (p, GFX.FixedColour)) \
			    : p)
			      
    RENDER_TILE_LARGE (GFX.ScreenColors [pixel], LARGE_SUB_PIXEL1_2)
}

#ifdef USE_GLIDE
#if 0
void DrawTile3dfx (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount)
{
    TILE_PREAMBLE

    float x = Offset % GFX.Pitch;
    float y = Offset / GFX.Pitch;

    Glide.sq [0].x = Glide.x_offset + x * Glide.x_scale;
    Glide.sq [0].y = Glide.y_offset + y * Glide.y_scale;
    Glide.sq [1].x = Glide.x_offset + (x + 8.0) * Glide.x_scale;
    Glide.sq [1].y = Glide.y_offset + y * Glide.y_scale;
    Glide.sq [2].x = Glide.x_offset + (x + 8.0) * Glide.x_scale;
    Glide.sq [2].y = Glide.y_offset + (y + LineCount) * Glide.y_scale;
    Glide.sq [3].x = Glide.x_offset + x * Glide.x_scale;
    Glide.sq [3].y = Glide.y_offset + (y + LineCount) * Glide.y_scale;

    if (!(Tile & (V_FLIP | H_FLIP)))
    {
	// Normal
	Glide.sq [0].tmuvtx [0].sow = 0.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine;
	Glide.sq [1].tmuvtx [0].sow = 8.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine;
	Glide.sq [2].tmuvtx [0].sow = 8.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [3].tmuvtx [0].sow = 0.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine + LineCount;
    }
    else
    if (!(Tile & V_FLIP))
    {
	// Flipped
	Glide.sq [0].tmuvtx [0].sow = 8.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine;
	Glide.sq [1].tmuvtx [0].sow = 0.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine;
	Glide.sq [2].tmuvtx [0].sow = 0.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [3].tmuvtx [0].sow = 8.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine + LineCount;
    }
    else
    if (Tile & H_FLIP)
    {
	// Horizontal and vertical flip
	Glide.sq [0].tmuvtx [0].sow = 8.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [1].tmuvtx [0].sow = 0.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [2].tmuvtx [0].sow = 0.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine;
	Glide.sq [3].tmuvtx [0].sow = 8.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine;
    }
    else
    {
	// Vertical flip only
	Glide.sq [0].tmuvtx [0].sow = 0.0;
	Glide.sq [0].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [1].tmuvtx [0].sow = 8.0;
	Glide.sq [1].tmuvtx [0].tow = StartLine + LineCount;
	Glide.sq [2].tmuvtx [0].sow = 8.0;
	Glide.sq [2].tmuvtx [0].tow = StartLine;
	Glide.sq [3].tmuvtx [0].sow = 0.0;
	Glide.sq [3].tmuvtx [0].tow = StartLine;
    }
    grTexDownloadMipMapLevel (GR_TMU0, Glide.texture_mem_start,
			      GR_LOD_8, GR_LOD_8, GR_ASPECT_1x1,
			      GR_TEXFMT_RGB_565,
			      GR_MIPMAPLEVELMASK_BOTH,
			      (void *) pCache);
    grTexSource (GR_TMU0, Glide.texture_mem_start,
		 GR_MIPMAPLEVELMASK_BOTH, &Glide.texture);
    grDrawTriangle (&Glide.sq [0], &Glide.sq [3], &Glide.sq [2]);
    grDrawTriangle (&Glide.sq [0], &Glide.sq [1], &Glide.sq [2]);
}

void DrawClippedTile3dfx (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount)
{
    TILE_PREAMBLE
    register uint8 *bp;

    TILE_CLIP_PREAMBLE
    RENDER_CLIPPED_TILE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4)
}
#endif
#endif

