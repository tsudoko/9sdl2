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
void eresized(int) {} /* required for successful linking, will implement later */

/* TODO: remove unused headers */
#define _PLAN9_SOURCE
#include <u.h>
#include <lib9.h>
#include <draw.h>
#include <event.h>

/* open(), close() */
#include <fcntl.h>
#include <unistd.h>
#define ORDWR O_RDWR

#include "../SDL_sysvideo.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_keyboard_c.h"

#include "SDL_devdraw_c.h"
#include "SDL_devdrawevents_c.h"

int
DEVDRAW_videoinit(_THIS)
{
    SDL_VideoDisplay d;
    SDL_DisplayMode m;

    SDL_zero(m);
    if (initdraw(nil, nil, nil) < 0) {
        char err[256];
        errstr(err, sizeof err);
        return SDL_SetError("initdraw: %s", err);
    }

    einit(Emouse|Ekeyboard);

    m.w = Dx(screen->r);
    m.h = Dy(screen->r);
    m.refresh_rate = 60; /* TODO */
    m.format = SDL_PIXELFORMAT_RGBA32; /* TODO */
    m.driverdata = NULL;

    SDL_zero(display);
    d.desktop_mode = m;
    d.current_mode = m;
    d.driverdata = NULL;

    SDL_AddVideoDisplay(&d);

    return 1;
}

void
DEVDRAW_videoquit(_THIS)
{

}

void
DEVDRAW_getdisplaymodes(_THIS, SDL_VideoDisplay * display)
{

}

int
DEVDRAW_setdisplaymode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

int
DEVDRAW_createwindow(_THIS, SDL_Window * w)
{
    SDL_VideoData *phdata = (SDL_VideoData *) _this->driverdata;
    SDL_WindowData *wdata;

    wdata = (SDL_WindowData *) SDL_calloc(1, sizeof(SDL_WindowData));
    if (wdata == NULL) {
        return SDL_OutOfMemory();
    }
    w->driverdata = wdata;

    if ((w->flags & SDL_WINDOW_OPENGL) == SDL_WINDOW_OPENGL) {
        return SDL_Unsupported();
    }

    return 0;
}

int
DEVDRAW_createwindowfrom(_THIS, SDL_Window * w, const void *data)
{
    return -1;
}

void
DEVDRAW_setwindowtitle(_THIS, SDL_Window * w)
{
    int label = open("/dev/label", ORDWR);
    if (label < 0) {
        return;
    }

    fprint(label, w->title);
    close(label);
}
void
DEVDRAW_setwindowicon(_THIS, SDL_Window * w, SDL_Surface * icon)
{
}
/* TODO: resizes might not work when the window is not current */
void
DEVDRAW_setwindowposition(_THIS, SDL_Window * w)
{
    int wctl = open("/dev/wctl", ORDWR);
    if (wctl < 0) {
        return;
    }

    fprint(wctl, "move -minx %d -miny %d", w->x, w->y);
    close(wctl);
}
void
DEVDRAW_setwindowsize(_THIS, SDL_Window * w)
{
    int wctl = open("/dev/wctl", ORDWR);
    if (wctl < 0) {
        return;
    }

    fprint(wctl, "resize -dx %d -dy %d", w->w, w->h);
    close(wctl);
}
/* as far as we understand there's no way to implement these with rio, since
   "hidden" in this context means "not visible on the window list" as well */
void
DEVDRAW_showwindow(_THIS, SDL_Window * w)
{
}
void
DEVDRAW_hidewindow(_THIS, SDL_Window * w)
{
}
void
DEVDRAW_raisewindow(_THIS, SDL_Window * w)
{
    int wctl = open("/dev/wctl", ORDWR);
    if (wctl < 0) {
        return;
    }

    fprint(wctl, "current");
    close(wctl);
}
void
DEVDRAW_maximizewindow(_THIS, SDL_Window * w)
{
}
void
DEVDRAW_minimizewindow(_THIS, SDL_Window * w)
{
    int wctl = open("/dev/wctl", ORDWR);
    if (wctl < 0) {
        return;
    }

    fprint(wctl, "hide");
    close(wctl);
}
void
DEVDRAW_restorewindow(_THIS, SDL_Window * w)
{
    int wctl = open("/dev/wctl", ORDWR);
    if (wctl < 0) {
        return;
    }

    fprint(wctl, "unhide");
    close(wctl);
}
void
DEVDRAW_setwindowgrab(_THIS, SDL_Window * w, SDL_bool grabbed)
{
}
void
DEVDRAW_destroywindow(_THIS, SDL_Window * w)
{
    SDL_VideoData *phdata = (SDL_VideoData *) _this->driverdata;
    /* TODO */
}

/* TODO maybe implement this */
SDL_bool
DEVDRAW_getwindowwminfo(_THIS, SDL_Window * w, struct SDL_SysWMinfo *info)
{
    return SDL_FALSE;
}

static int
DEVDRAW_available(void)
{
    return 1;
}

static void
DEVDRAW_destroy(SDL_VideoDevice * device)
{
    if (device->driverdata != NULL) {
        SDL_free(device->driverdata);
        device->driverdata = NULL;
    }
    SDL_free(device);
}

static SDL_VideoDevice *
DEVDRAW_create()
{
    SDL_VideoDevice *device;
    SDL_VideoData *phdata;
    int status;

    if (!DEVDRAW_available()) {
        return NULL;
    }

    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    phdata = (SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    if (phdata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }

    device->driverdata = phdata;

    /* phdata->egl_initialized = SDL_TRUE; */


    /* Setup amount of available displays */
    device->num_displays = 0;

    /* Set device free function */
    device->free = DEVDRAW_destroy;

    /* Setup all functions which we can handle */
    device->VideoInit = DEVDRAW_videoinit;
    device->VideoQuit = DEVDRAW_videoquit;
    device->GetDisplayModes = DEVDRAW_getdisplaymodes;
    device->SetDisplayMode = DEVDRAW_setdisplaymode;
    device->CreateSDLWindow = DEVDRAW_createwindow;
    device->CreateSDLWindowFrom = DEVDRAW_createwindowfrom;
    device->SetWindowTitle = DEVDRAW_setwindowtitle;
    device->SetWindowIcon = DEVDRAW_setwindowicon;
    device->SetWindowPosition = DEVDRAW_setwindowposition;
    device->SetWindowSize = DEVDRAW_setwindowsize;
    device->ShowWindow = DEVDRAW_showwindow;
    device->HideWindow = DEVDRAW_hidewindow;
    device->RaiseWindow = DEVDRAW_raisewindow;
    device->MaximizeWindow = DEVDRAW_maximizewindow;
    device->MinimizeWindow = DEVDRAW_minimizewindow;
    device->RestoreWindow = DEVDRAW_restorewindow;
    device->SetWindowGrab = DEVDRAW_setwindowgrab;
    device->DestroyWindow = DEVDRAW_destroywindow;
#if 0
    device->GetWindowWMInfo = DEVDRAW_getwindowwminfo;
#endif
    device->CreateWindowFramebuffer = DEVDRAW_createwindowframebuffer;

    /* !!! FIXME: implement SetWindowBordered */

    return device;
}

VideoBootStrap DEVDRAW_bootstrap = {
    "devdraw",
    "SDL /dev/draw Video Driver",
    DEVDRAW_available,
    DEVDRAW_create
};

#endif /* SDL_VIDEO_DRIVER_DEVDRAW */

/* vi: set ts=4 sw=4 expandtab: */
