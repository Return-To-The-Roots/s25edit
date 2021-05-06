// Copyright (C) 2000 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	SGE shape
 *
 *	Started 000430
 */

#include "sge_shape.h"
#include "sge_primitives.h"
#include "sge_surface.h"

#ifndef _SGE_NO_CLASSES

using namespace std;

//==================================================================================
// sge_surface (derived from sge_shape)
// A class for moving/blitting surfaces
//==================================================================================
sge_surface::sge_surface(SDL_Surface* dest, SDL_Surface* src, Sint16 x, Sint16 y)
{
    surface = src;
    sge_surface::dest = dest;

    current_pos.x = x;
    current_pos.y = y;
    current_pos.w = src->w;
    current_pos.h = src->h;
    last_pos.x = last_pos.y = last_pos.w = last_pos.h = 0;
    prev_pos = last_pos;

    border.x = border.y = 0;
    border.w = dest->w;
    border.h = dest->h;
    warp_border = false;
}

sge_surface::~sge_surface() = default;

bool sge_surface::check_warp()
{
    bool flag = false;

    if(warp_border)
    {
        if(current_pos.x + current_pos.w < border.x)
        {
            current_pos.x = border.x + border.w - current_pos.w;
            flag = true;
        } else if(current_pos.x > border.x + border.w)
        {
            current_pos.x = border.x;
            flag = true;
        }
        if(current_pos.y + current_pos.h < border.y)
        {
            current_pos.y = border.y + border.h - current_pos.h;
            flag = true;
        } else if(current_pos.y > border.y + border.h)
        {
            current_pos.y = border.y;
            flag = true;
        }
    }
    return flag;
}

int sge_surface::get_warp(SDL_Rect rec, SDL_Rect& r1, SDL_Rect& r2, SDL_Rect& r3, SDL_Rect& r4) const
{
    // We want to decode the pos rectangle into two or four rectangles.

    r1.x = r2.x = r3.x = r4.x = rec.x, r1.y = r2.y = r3.y = r4.y = rec.y;
    r1.w = r2.w = r3.w = r4.w = rec.w, r1.h = r2.h = r3.h = r4.h = rec.h;

    int rects = 0;

    if(warp_border)
    {
        if(rec.x < border.x)
        {
            r1.w = border.x - rec.x;
            r1.x = border.x + border.w - r1.w;
            r2.w = abs(rec.w - r1.w); // SDL_Rect w/h is unsigned
            r2.x = border.x;
            rects = 2;
        } else if(rec.x + rec.w > border.x + border.w)
        {
            r1.x = rec.x;
            r1.w = border.x + border.w - rec.x;
            r2.x = border.x;
            r2.w = abs(rec.w - r1.w);
            rects = 2;
        }

        r3.x = r1.x;
        r3.w = r1.w;
        r4.x = r2.x;
        r4.w = r2.w;

        if(rec.y < border.y)
        {
            if(rects == 0)
            {
                r1.h = border.y - rec.y;
                r1.y = border.y + border.h - r1.h;
                r2.h = abs(rec.h - r1.h);
                r2.y = border.y;
                rects = 2;
            } else
            {
                r2.h = r1.h = border.y - rec.y;
                r2.y = r1.y = border.y + border.h - r1.h;
                r4.h = r3.h = abs(rec.h - r1.h);
                r4.y = r3.y = border.y;
                rects = 4;
            }
        } else if(rec.y + rec.h > border.y + border.h)
        {
            if(rects == 0)
            {
                r1.y = rec.y;
                r1.h = border.y + border.h - rec.y;
                r2.y = border.y;
                r2.h = abs(rec.h - r1.h);
                rects = 2;
            } else
            {
                r2.y = r1.y = rec.y;
                r2.h = r1.h = border.y + border.h - rec.y;
                r4.y = r3.y = border.y;
                r4.h = r3.h = abs(rec.h - r1.h);
                rects = 4;
            }
        }
    }
    return rects;
}

void sge_surface::warp_draw()
{
    SDL_Rect r1, r2, r3, r4;
    int rects = get_warp(current_pos, r1, r2, r3, r4);

    if(rects == 2)
    {
        sge_Blit(surface, dest, 0, 0, r1.x, r1.y, r1.w, r1.h);
        sge_Blit(surface, dest, surface->w - r2.w, surface->h - r2.h, r2.x, r2.y, r2.w, r2.h);
    } else if(rects == 4)
    {
        sge_Blit(surface, dest, 0, 0, r1.x, r1.y, r1.w, r1.h);
        sge_Blit(surface, dest, surface->w - r2.w, 0, r2.x, r2.y, r2.w, r2.h);
        sge_Blit(surface, dest, 0, surface->h - r3.h, r3.x, r3.y, r3.w, r3.h);
        sge_Blit(surface, dest, surface->w - r4.w, surface->h - r4.h, r4.x, r4.y, r4.w, r4.h);
    } else
        sge_Blit(surface, dest, 0, 0, current_pos.x, current_pos.y, surface->w, surface->h);
}

void sge_surface::warp_clear(Uint32 color)
{
    SDL_Rect r1, r2, r3, r4;
    int rects = get_warp(last_pos, r1, r2, r3, r4);

    if(rects > 0)
    {
        sge_FilledRect(dest, r1.x, r1.y, r1.x + r1.w - 1, r1.y + r1.h - 1, color);
        sge_FilledRect(dest, r2.x, r2.y, r2.x + r2.w - 1, r2.y + r2.h - 1, color);
        if(rects > 2)
        {
            sge_FilledRect(dest, r3.x, r3.y, r3.x + r3.w - 1, r3.y + r3.h - 1, color);
            sge_FilledRect(dest, r4.x, r4.y, r4.x + r4.w - 1, r4.y + r4.h - 1, color);
        }
    } else
        sge_FilledRect(dest, last_pos.x, last_pos.y, last_pos.x + last_pos.w - 1, last_pos.y + last_pos.h - 1, color);
}

void sge_surface::warp_clear(SDL_Surface* src, Sint16 srcX, Sint16 srcY)
{
    SDL_Rect r1, r2, r3, r4;
    int rects = get_warp(current_pos, r1, r2, r3, r4);

    if(rects > 0)
    {
        sge_Blit(src, dest, r1.x, r1.y, r1.x, r1.y, r1.w, r1.h);
        sge_Blit(src, dest, r2.x, r2.y, r2.x, r2.y, r2.w, r2.h);
        if(rects > 2)
        {
            sge_Blit(src, dest, r3.x, r3.y, r3.x, r3.y, r3.w, r3.h);
            sge_Blit(src, dest, r4.x, r4.y, r4.x, r4.y, r4.w, r4.h);
        }
    } else
        sge_Blit(src, dest, srcX, srcY, last_pos.x, last_pos.y, last_pos.w, last_pos.h);
}

// Draws the surface
void sge_surface::draw()
{
    if(!surface)
        return;

    current_pos.w = surface->w;
    current_pos.h = surface->h;

    if(warp_border)
        warp_draw();
    else
        sge_Blit(surface, dest, 0, 0, current_pos.x, current_pos.y, surface->w, surface->h);

    prev_pos = last_pos;
    last_pos = current_pos;
}

void sge_surface::clear(Uint32 color)
{
    if(warp_border)
        warp_clear(color);
    else
        sge_FilledRect(dest, last_pos.x, last_pos.y, last_pos.x + last_pos.w - 1, last_pos.y + last_pos.h - 1, color);
}

void sge_surface::clear(SDL_Surface* src, Sint16 srcX, Sint16 srcY)
{
    if(warp_border)
        warp_clear(src, srcX, srcY);
    else
        sge_Blit(src, dest, srcX, srcY, last_pos.x, last_pos.y, last_pos.w, last_pos.h);
}

//==================================================================================
// sge_ssprite (derived from sge_surface)
// A simple sprite class
//==================================================================================
sge_ssprite::sge_ssprite(SDL_Surface* screen, SDL_Surface* img, Sint16 x, Sint16 y) : sge_surface(screen, img, x, y)
{
    // Create the first frame
    current_frame = new sge_frame; // User has to catch bad_alloc himself
    current_frame->img = img;
    current_frame->cdata = nullptr;
    frames.push_back(current_frame);

    current_fi = frames.begin();
    fi_start = current_fi;
    fi_stop = frames.end();

    // Default
    xvel = yvel = 0;
    seq_mode = stop;

    bounce_border = true;
}

sge_ssprite::sge_ssprite(SDL_Surface* screen, SDL_Surface* img, sge_cdata* cdata, Sint16 x, Sint16 y)
    : sge_surface(screen, img, x, y)
{
    // Create the first frame
    current_frame = new sge_frame; // User has to catch bad_alloc himself
    current_frame->img = img;
    current_frame->cdata = cdata;
    frames.push_back(current_frame);

    current_fi = frames.begin();
    fi_start = current_fi;
    fi_stop = frames.end();

    // Default
    xvel = yvel = 0;
    seq_mode = stop;

    bounce_border = true;
}

sge_ssprite::~sge_ssprite()
{
    // Empty the list
    for(auto* frame : frames)
        delete frame;

    frames.clear();
}

bool sge_ssprite::check_border()
{
    if(!bounce_border)
        return sge_surface::check_border();

    bool flag = false;

    if(current_pos.x < border.x)
    {
        current_pos.x = border.x;
        xvel = -xvel;
        flag = true;
    }
    if(current_pos.x + current_pos.w > border.x + border.w)
    {
        current_pos.x = border.x + border.w - current_pos.w;
        xvel = -xvel;
        flag = true;
    }
    if(current_pos.y < border.y)
    {
        current_pos.y = border.y;
        yvel = -yvel;
        flag = true;
    }
    if(current_pos.y + current_pos.h > border.y + border.h)
    {
        current_pos.y = border.y + border.h - current_pos.h;
        yvel = -yvel;
        flag = true;
    }
    return flag;
}

void sge_ssprite::add_frame(SDL_Surface* img)
{
    add_frame(img, nullptr);
}

void sge_ssprite::add_frame(SDL_Surface* img, sge_cdata* cdata)
{
    // Create a new frame
    auto* frame = new sge_frame; // User has to catch bad_alloc himself
    frame->img = img;
    frame->cdata = cdata;
    frames.push_back(frame);

    fi_start = frames.begin();
    fi_stop = frames.end();

    seq_mode = loop;
}

void sge_ssprite::skip_frame(int skips)
{
    if(skips > 0)
    {
        for(int i = 0; i < skips; i++)
        {
            ++current_fi;
            if(current_fi == fi_stop)
            {
                if(seq_mode != play_once)
                    current_fi = fi_start; // loop
                else
                { // stop
                    seq_mode = stop;
                    --current_fi; // current_fi = fi_stop -1
                    fi_start = current_fi;
                }
            }
        }
    } else if(skips < 0)
    {
        for(int i = 0; i > skips; i--)
        {
            if(current_fi == fi_start)
            {
                if(seq_mode != play_once)
                    current_fi = fi_stop; // loop
                else
                { // stop
                    seq_mode = stop;
                    ++current_fi; //+1
                    fi_stop = current_fi;
                }
            }
            --current_fi;
        }
    } else
        return;

    current_frame = *current_fi;
    surface = current_frame->img;
    current_pos.w = surface->w;
    current_pos.h = surface->h;
}

bool sge_ssprite::update()
{
    move(xvel, yvel);
    return !((xvel == 0) && (yvel == 0));
}

void sge_ssprite::set_seq(int start, int stop, playing_mode mode)
{
    // Handle stupid user errors
    if(start < 0 || start > int(frames.size()) - 1)
        return;
    if(stop < start || stop > int(frames.size()) - 1)
        return;

    seq_mode = loop;
    if(mode == play_once)
        seq_mode = play_once;
    if(start == stop)
        seq_mode = sge_ssprite::stop;

    fi_start = fi_stop = frames.begin();

    for(int i = 0; i <= stop; i++)
    {
        if(i < start)
            ++fi_start;

        ++fi_stop;

        if(fi_stop == frames.end())
        {
            if(fi_start == frames.end())
                --fi_start;
            break;
        }
    }

    current_fi = fi_start;

    current_frame = *current_fi;
    surface = current_frame->img;
    current_pos.w = surface->w;
    current_pos.h = surface->h;
}

void sge_ssprite::reset_seq()
{
    fi_start = frames.begin();
    fi_stop = frames.end();

    current_fi = fi_start;

    current_frame = *current_fi;
    surface = current_frame->img;
    current_pos.w = surface->w;
    current_pos.h = surface->h;

    if(frames.size() > 1)
        seq_mode = loop;
    else
        seq_mode = stop;
}

void sge_ssprite::first_frame()
{
    current_fi = fi_start;

    current_frame = *current_fi;
    surface = current_frame->img;
    current_pos.w = surface->w;
    current_pos.h = surface->h;
}

void sge_ssprite::last_frame()
{
    current_fi = fi_stop;
    --current_fi;

    current_frame = *current_fi;
    surface = current_frame->img;
    current_pos.w = surface->w;
    current_pos.h = surface->h;
}

//==================================================================================
// sge_sprite (derived from sge_ssprite)
// A timed sprite class
//==================================================================================
bool sge_sprite::update(Uint32 ticks)
{
    if(tlast == 0)
    {
        tlast = ticks;
        return false;
    }

    Sint16 tmp;
    Uint32 time = ticks - tlast;
    tlast = ticks; // Reset time

    bool ret = false;

    // Calculate new pos
    if(xppms != 0)
    {
        xpos += time * xppms;
        tmp = int(xpos);
        if(current_pos.x != tmp)
        {
            current_pos.x = tmp;
            ret = true;
        }
    }
    if(yppms != 0)
    {
        ypos += time * yppms;
        tmp = int(ypos);
        if(current_pos.y != tmp)
        {
            current_pos.y = tmp;
            ret = true;
        }
    }

    if(ret) // Are we off-screen?
        check_border();

    // Calculate new frame
    if(fpms != 0)
    {
        fpos += time * fpms;
        tmp = int(fpos);
        if(tmp != 0)
        {
            skip_frame(tmp);
            fpos -= tmp;
            ret = true;
        }
    }

    return ret;
}

bool sge_sprite::check_border()
{
    if(warp_border)
    {
        if(sge_surface::check_warp())
        {
            xpos = current_pos.x;
            ypos = current_pos.y;
            return true;
        }
        return false;
    }
    if(!bounce_border)
        return false;

    bool flag = false;

    if(current_pos.x < border.x)
    {
        current_pos.x = border.x;
        xpos = current_pos.x;
        xppms = -xppms;
        flag = true;
    } else if(current_pos.x + current_pos.w > border.x + border.w)
    {
        current_pos.x = border.x + border.w - current_pos.w;
        xpos = current_pos.x;
        xppms = -xppms;
        flag = true;
    }
    if(current_pos.y < border.y)
    {
        current_pos.y = border.y;
        ypos = current_pos.y;
        yppms = -yppms;
        flag = true;
    } else if(current_pos.y + current_pos.h > border.y + border.h)
    {
        current_pos.y = border.y + border.h - current_pos.h;
        ypos = current_pos.y;
        yppms = -yppms;
        flag = true;
    }
    return flag;
}

#endif /* _SGE_NO_CLASSES */
