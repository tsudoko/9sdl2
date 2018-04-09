/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_DEVDRAW

#include <assert.h>
#include <stdlib.h>
#define _PLAN9_SOURCE
#include <limits.h>
#include <draw.h>

#include "../SDL_sysvideo.h"
#include "SDL_devdrawframebuffer_c.h"

#define Dimg "_devdrawimg"
#define Dnimg "_devdrawnimg"

int
DEVDRAW_CreateWindowFramebuffer(_THIS, SDL_Window * win, Uint32 * format, void ** pixels, int *pitch)
{
    unsigned char *img;
    int w, h;
    int bpp;

    img = SDL_GetWindowData(win, Dimg);
    if (img) {
        free(img);
    }

    bpp = screen->depth;
#if 0
    print("bpp %d\n", bpp);
    char chan[10];
    chantostr((char *)chan, screen->chan);
    print("chan %s\n", chan);
#endif
    assert(bpp == 4*CHAR_BIT);
    SDL_GetWindowSize(win, &w, &h);
    assert(sizeof (void *) >= sizeof (size_t));
    size_t size = w*h*(bpp/CHAR_BIT);
    *pixels = malloc(size);
    if (!*pixels) {
        return -1;
    }
    *format = SDL_PIXELFORMAT_BGRA32;
    *pitch = w*(bpp/CHAR_BIT);

    SDL_SetWindowData(win, Dimg, (void *)*pixels);
    SDL_SetWindowData(win, Dnimg, (void *)size);
    return 0;
}

int
DEVDRAW_UpdateWindowFramebuffer(_THIS, SDL_Window * w, const SDL_Rect * rects, int numrects)
{
    unsigned char *img = (unsigned char *)SDL_GetWindowData(w, Dimg);
    size_t nimg = (size_t)SDL_GetWindowData(w, Dnimg);
    int r;
    if (!img) {
        return SDL_SetError("Couldn't find window image data");
    }

    if ((r = loadimage(screen, screen->r, img, nimg)) != nimg) {
        /* TODO: use %r */
        return SDL_SetError("Updating failed (updated %d/%d)", r, nimg);
    }

    return 0;
}

void
DEVDRAW_DestroyWindowFramebuffer(_THIS, SDL_Window * w)
{
    SDL_SetWindowData(w, Dnimg, (void *)0);
    unsigned char *img = (unsigned char *)SDL_SetWindowData(w, Dimg, NULL);
    if (img) {
        free(img);
    }
}

#endif /* SDL_VIDEO_DRIVER_DEVDRAW */

/* vi: set ts=4 sw=4 expandtab: */
