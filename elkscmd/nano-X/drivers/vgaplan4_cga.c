/*
 * Copyright (c) 1999 Greg Haerr <greg@censoft.com>
 *
 * 16 color 4 planes EGA/VGA Planar Video Driver for MicroWindows
 * Portable C version
 * Blitting enabled with #define HAVEBLIT in vgaplan4.h
 *
 * Based on BOGL - Ben's Own Graphics Library.
 *   Written by Ben Pfaff <pfaffben@debian.org>.
 *	 BOGL is licensed under the terms of the GNU General Public License
 *
 * In this driver, psd->linelen is line byte length, not line pixel length
 *
 */

/* Modified for CGA
 * T. Yamada 2024
 */

/*#define NDEBUG*/
#include <assert.h>
#include "../device.h"
#include "vgaplan4.h"
#include "fb.h"

/* assumptions for speed: NOTE: psd is ignored in these routines*/
#define SCREENBASE0 	MK_FP(0xb800, 0)
#define SCREENBASE1 	MK_FP(0xba00, 0)
#define BYTESPERLINE		80

static FARADDR screenbase_table[2] = {
	SCREENBASE0, SCREENBASE1
};

/* extern data*/
extern MODE gr_mode;	/* temp kluge*/

/* precalculated mask bits*/
static unsigned char mask[8] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

/* Init VGA controller, calc linelen and mmap size, return 0 on fail*/
int
ega_init(PSD psd)
{
	psd->addr = (char *)SCREENBASE0;		/* long ptr -> short on 16bit sys*/
	psd->linelen = BYTESPERLINE;
	psd->size = 65535;

	return 1;
}

/* draw a pixel at x,y of color c*/
void
ega_drawpixel(PSD psd,unsigned int x, unsigned int y, PIXELVAL c)
{
	FARADDR dst;
	assert (x >= 0 && x < psd->xres);
	assert (y >= 0 && y < psd->yres);
	assert (c >= 0 && c < psd->ncolors);

	DRAWON;
	if (y < psd->yres) {
		dst = screenbase_table[y&1] + x / 8 + y / 2 * BYTESPERLINE;
		if(gr_mode == MODE_XOR) {
			if  (c) {
				PUTBYTE_FP(dst,(GETBYTE_FP(dst) ^ mask[x&7]));
			}
		} else {
			if  (c) {
				ORBYTE_FP (dst,mask[x&7]);
			}
		else {
				ANDBYTE_FP (dst,~mask[x&7]);
			}
		}
	}
	DRAWOFF;
}

/* Return pixel value at x,y*/
PIXELVAL
ega_readpixel(PSD psd,unsigned int x,unsigned int y)
{
	FARADDR		src;
	PIXELVAL	c = 0;

	assert (x >= 0 && x < psd->xres);
	assert (y >= 0 && y < psd->yres);

	DRAWON;
	src = screenbase_table[y&1] + x / 8 + y / 2 * BYTESPERLINE;
	if(GETBYTE_FP(src) & mask[x&7])
		c = 1;
	DRAWOFF;
	return c;
}

/* Draw horizontal line from x1,y to x2,y not including final point*/
void
ega_drawhorzline(PSD psd, unsigned int x1, unsigned int x2, unsigned int y,
	PIXELVAL c)
{
	FARADDR dst, last;
	unsigned int x1_ini;

	x1_ini = x1;

	assert (x1 >= 0 && x1 < psd->xres);
	assert (x2 >= 0 && x2 < psd->xres);
	assert (x2 >= x1);
	assert (y >= 0 && y < psd->yres);
	assert (c >= 0 && c < psd->ncolors);

	DRAWON;
	if (y < psd->yres) {
		/* OR/AND mode is not supported for CGA for now */
		if(gr_mode == MODE_SET) {
			x1 = x1_ini;
			dst = screenbase_table[y&1] + x1 / 8 + y / 2 * BYTESPERLINE;
			if (x1 / 8 == x2 / 8) {
				while(x1 < x2) {
					if  (c) {
						ORBYTE_FP (dst,mask[x1&7]);
					}
					else {
						ANDBYTE_FP (dst,~mask[x1&7]);
					}
					x1++;
				}
			} else {

				while (x1 % 8) {
					if  (c) {
						ORBYTE_FP (dst,mask[x1&7]);
					}
					else {
						ANDBYTE_FP (dst,~mask[x1&7]);
					}
					x1++;
				}
				if (x1_ini % 8)
					dst++;

				last = screenbase_table[y&1] + x2 / 8 + y / 2 * BYTESPERLINE;
				while (dst < last) {
					if  (c) {
						PUTBYTE_FP(dst++, 255);
					}
					else {
						PUTBYTE_FP(dst++, 0);
					}
				}

				x1 = ((x2 >> 3) << 3);
				while (x1 < x2) {
					if  (c) {
						ORBYTE_FP (dst,mask[x1&7]);
					}
					else {
						ANDBYTE_FP (dst,~mask[x1&7]);
					}
					x1++;
				}
			}
		} else if(gr_mode == MODE_XOR) {
			x1 = x1_ini;
			dst = screenbase_table[y&1] + x1 / 8 + y / 2 * BYTESPERLINE;
			if (x1 / 8 == x2 / 8) {
				while(x1 < x2) {
					if  (c) {
						PUTBYTE_FP(dst,(GETBYTE_FP(dst) ^ mask[x1&7]));
					}
					x1++;
				}
			} else {

				while (x1 % 8) {
					if  (c) {
						PUTBYTE_FP(dst,(GETBYTE_FP(dst) ^ mask[x1&7]));
					}
					x1++;
				}
				if (x1_ini % 8)
				dst++;

				last = screenbase_table[y&1] + x2 / 8 + y / 2 * BYTESPERLINE;
				while (dst < last) {
					if  (c) {
						PUTBYTE_FP(dst,~GETBYTE_FP(dst));
						dst++;
					}
				}

				x1 = ((x2 >> 3) << 3);
				while (x1 < x2) {
					if  (c) {
						PUTBYTE_FP(dst,(GETBYTE_FP(dst) ^ mask[x1&7]));
					}
					x1++;
				}
			}
		} else {
			/* slower method, draw pixel by pixel*/
			while(x1 < x2) {
				if  (c) {
					ORBYTE_FP (screenbase_table[y&1] + x1 / 8 + y / 2 * BYTESPERLINE,mask[x1&7]);
				}
				else {
					ANDBYTE_FP (screenbase_table[y&1] + x1 / 8 + y / 2 * BYTESPERLINE,~mask[x1&7]);
				}
				x1++;
			}
		}
	}
	DRAWOFF;
}

/* Draw a vertical line from x,y1 to x,y2 not including final point*/
void
ega_drawvertline(PSD psd,unsigned int x, unsigned int y1, unsigned int y2,
	PIXELVAL c)
{
	FARADDR dst;
	unsigned int y;

	assert (x >= 0 && x < psd->xres);
	assert (y1 >= 0 && y1 < psd->yres);
	assert (y2 >= 0 && y2 < psd->yres);
	assert (y2 >= y1);
	assert (c >= 0 && c < psd->ncolors);

	y = y1;

	DRAWON;
	if(gr_mode == MODE_XOR) {
		while (y < y2 && y < psd->yres) {
			dst = screenbase_table[y&1] + x / 8 + y / 2 * BYTESPERLINE;
			if  (c) {
				PUTBYTE_FP(dst,(GETBYTE_FP(dst) ^ mask[x&7]));
			}
			y++;
		}
	} else {
		while (y < y2 && y < psd->yres) {
			dst = screenbase_table[y&1] + x / 8 + y / 2 * BYTESPERLINE;
			if  (c) {
				ORBYTE_FP (dst,mask[x&7]);
			}
			else {
				ANDBYTE_FP (dst,~mask[x&7]);
			}
			y++;
		}
	}
	DRAWOFF;
}

void
ega_blit(PSD dstpsd, COORD dstx, COORD dsty, COORD w, COORD h,
	PSD srcpsd, COORD srcx, COORD srcy, int op)
{
#if HAVEBLIT
	BOOL	srcvga, dstvga;

    /* decide which blit algorithm to use*/
	srcvga = srcpsd->flags & PSF_SCREEN;
	dstvga = dstpsd->flags & PSF_SCREEN;

	if(srcvga) {
		if(dstvga)
			vga_to_vga_blit(dstpsd, dstx, dsty, w, h,
				srcpsd, srcx, srcy, op);
		else
			vga_to_mempl4_blit(dstpsd, dstx, dsty, w, h,
				srcpsd, srcx, srcy, op);
	} else {
		if(dstvga)
			mempl4_to_vga_blit(dstpsd, dstx, dsty, w, h,
				srcpsd, srcx, srcy, op);
		else
			mempl4_to_mempl4_blit(dstpsd, dstx, dsty, w, h,
				srcpsd, srcx, srcy, op);
	}
#endif /* HAVEBLIT*/
}
