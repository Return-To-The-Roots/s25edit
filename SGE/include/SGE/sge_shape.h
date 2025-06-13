// Copyright (C) 2000 - 2003 Anders Lindström
// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 *	SDL Graphics Extension
 *	SGE shape (header)
 *
 *	Started 000430
 */

#pragma once

#include "sge_internal.h"
#include <SDL.h>

#ifndef _SGE_NO_CLASSES

// Remove "class 'std::list<>' needs to have dll-interface to be used" warnings
// from MS VisualC++
#    ifdef _MSC_VER
#        pragma warning(push)
#        pragma warning(disable : 4251)
#    endif

#    include <list>
#    include <vector>

struct sge_cdata;
class DECLSPEC sge_shape;

//==================================================================================
// sge_shape
// Abstract base class for different shapes (surfaces, sprites, ...)
//==================================================================================
class sge_shape
{
protected:
    SDL_Rect current_pos; // The *current* (maybe undrawn) position of the shape
    SDL_Rect last_pos;    // The *last* drawn position of shape
    SDL_Rect prev_pos;    // The previous drawn position of shape (used to update a cleared area)

    SDL_Surface* dest; // The surface to perform operations on

public:
    virtual ~sge_shape() = default; // Destructor
    virtual void draw() = 0;        // Draws the shape - prev_pos = last_pos; last_pos = the new position of shape

    // Some functions to clear (remove) shape
    virtual void clear(Uint32 color) = 0;                               // Clears to color
    virtual void clear(SDL_Surface* src, Sint16 srcX, Sint16 srcY) = 0; // Clears by blitting src

    SDL_Rect get_pos() const { return current_pos; }   // Returns the current position
    SDL_Rect get_last_pos() const { return last_pos; } // Returns the last updated position

    SDL_Surface* get_dest() const { return dest; }

    /*
    //NW N NE
    //  \|/
    // W-C-E
    //  /|\
    //SW S SE
    //
    //Returns some usefull coords in shape (current)
    */
    Sint16 c_x() const { return current_pos.x + current_pos.w / 2; }
    Sint16 c_y() const { return current_pos.y + current_pos.h / 2; }

    Sint16 nw_x() const { return current_pos.x; }
    Sint16 nw_y() const { return current_pos.y; }

    Sint16 n_x() const { return current_pos.x + current_pos.w / 2; }
    Sint16 n_y() const { return current_pos.y; }

    Sint16 ne_x() const { return current_pos.x + current_pos.w - 1; }
    Sint16 ne_y() const { return current_pos.y; }

    Sint16 e_x() const { return current_pos.x + current_pos.w - 1; }
    Sint16 e_y() const { return current_pos.y + current_pos.h / 2; }

    Sint16 se_x() const { return current_pos.x + current_pos.w - 1; }
    Sint16 se_y() const { return current_pos.y + current_pos.h - 1; }

    Sint16 s_x() const { return current_pos.x + current_pos.w / 2; }
    Sint16 s_y() const { return current_pos.y + current_pos.h - 1; }

    Sint16 sw_x() const { return current_pos.x; }
    Sint16 sw_y() const { return current_pos.y + current_pos.h - 1; }

    Sint16 w_x() const { return current_pos.x; }
    Sint16 w_y() const { return current_pos.y + current_pos.h / 2; }

    Sint16 get_xpos() const { return current_pos.x; }
    Sint16 get_ypos() const { return current_pos.y; }
    Uint16 get_w() const { return current_pos.w; }
    Uint16 get_h() const { return current_pos.h; }
};

//==================================================================================
// sge_surface (derived from sge_shape)
// A class for moving/blitting surfaces
//==================================================================================
class DECLSPEC sge_surface : public sge_shape
{
protected:
    SDL_Surface* surface; // The source surface *NOT COPIED*

    // Do warp logic
    bool check_warp();

    // Handles outside screen problems (But not in this class)
    virtual bool check_border() { return check_warp(); }

    // The border box (default: the screen size)
    SDL_Rect border;

    // Should we warp around the border box? (not in this class
    // but some methods here must know)
    bool warp_border;

    // Decode a warp pos rectangle
    int get_warp(SDL_Rect rec, SDL_Rect& r1, SDL_Rect& r2, SDL_Rect& r3, SDL_Rect& r4) const;

    // Helper functions
    void warp_draw();
    void warp_clear(Uint32 color);
    void warp_clear(SDL_Surface* src, Sint16 srcX, Sint16 srcY);

public:
    sge_surface(SDL_Surface* dest, SDL_Surface* src, Sint16 x = 0, Sint16 y = 0);
    ~sge_surface();

    // Draws the surface
    virtual void draw() override;

    virtual void clear(Uint32 color) override;
    virtual void clear(SDL_Surface* src, Sint16 srcX, Sint16 srcY) override;
    // virtual void clear(SDL_Surface *src){clear(src,last_pos.x,last_pos.y);}

    // Move the surface
    virtual void move_to(Sint16 x, Sint16 y)
    {
        current_pos.x = x;
        current_pos.y = y;
        check_border();
    }
    virtual void move(Sint16 x_step, Sint16 y_step)
    {
        current_pos.x += x_step;
        current_pos.y += y_step;
        check_border();
    }

    // Get pointer to surface
    SDL_Surface* get_img() const { return surface; }

    // Sets the border box
    void set_border(SDL_Rect box) { border = box; }
};

//==================================================================================
// The frame struct (for sge_ssprite)
//==================================================================================
struct sge_frame
{
    // The image
    SDL_Surface* img;

    // Collision data
    sge_cdata* cdata;
};

//==================================================================================
// sge_ssprite (derived from sge_surface)
// A simple sprite class
//==================================================================================
class DECLSPEC sge_ssprite : public sge_surface
{
public:
    enum playing_mode
    {
        loop,
        play_once,
        stop
    }; // This must be public

protected:
    // Linked list with the frames
    // Obs! 'surface' always points to the current frame image
    std::list<sge_frame*> frames;
    using FI = std::list<sge_frame*>::const_iterator; // List iterator (for frames)

    FI current_fi;
    FI fi_start; // first frame in the playing sequence loop
    FI fi_stop;  // last frame + 1

    // The pointer to the current frame
    sge_frame* current_frame; // Should at all times be *current_fi

    // The speed of the sprite (pixels/update)
    Sint16 xvel, yvel;

    bool bounce_border; // Do we want the sprite to bounce at the border?
    virtual bool check_border() override;

    // Playing sequence mode
    playing_mode seq_mode;

public:
    // Constructor and destructor
    sge_ssprite(SDL_Surface* screen, SDL_Surface* img, Sint16 x = 0, Sint16 y = 0);
    sge_ssprite(SDL_Surface* screen, SDL_Surface* img, sge_cdata* cdata, Sint16 x = 0, Sint16 y = 0);
    ~sge_ssprite();

    // Updates the internal status
    // Returns true if the sprite moved
    virtual bool update();

    // Sets the speed
    void set_vel(Sint16 x, Sint16 y)
    {
        xvel = x;
        yvel = y;
    }
    void set_xvel(Sint16 x) { xvel = x; }
    void set_yvel(Sint16 y) { yvel = y; }

    // Gets the speed
    Sint16 get_xvel() const { return xvel; }
    Sint16 get_yvel() const { return yvel; }

    // Add a frame
    // Obs! Resets playing sequence
    void add_frame(SDL_Surface* img);
    void add_frame(SDL_Surface* img, sge_cdata* cdata);

    // Change frame
    void skip_frame(int skips); // A negative 'skips' indicates backwards
    void next_frame() { skip_frame(1); }
    void prev_frame() { skip_frame(-1); }
    void first_frame(); // Does NOT change the sequence
    void last_frame();  //             "

    // Changes playing sequence (0- first frame)
    // playing_mode:
    // sge_ssprite::loop - loops forever
    // sge_ssprite::play_once - just once then stops
    // sge_ssprite::stop - is returned by get_PlayingMode() if stoped
    void set_seq(int start, int stop, playing_mode mode = loop);
    void reset_seq();
    playing_mode get_PlayingMode() { return seq_mode; }

    // Get cdata for current frame
    sge_cdata* get_cdata() { return current_frame->cdata; }

    // Get the current frame
    sge_frame* get_frame() { return current_frame; }

    // Get the frame list
    // DO NOT ADD OR REMOVE ELEMENTS without using
    // reset_seq() when done!!
    std::list<sge_frame*>* get_list() { return &frames; }

    // Set border mode:
    // Bounce - sprite bounces (default)
    // Warp - sprite warps at border
    void border_bounce(bool mode)
    {
        bounce_border = mode;
        if(warp_border)
        {
            warp_border = false;
        }
    }
    void border_warp(bool mode)
    {
        warp_border = mode;
        if(bounce_border)
        {
            bounce_border = false;
        }
    }
};

//==================================================================================
// sge_sprite (derived from sge_ssprite)
// A timed sprite class
//==================================================================================
class DECLSPEC sge_sprite : public sge_ssprite
{
protected:
    // Pixels/ms
    double xppms, yppms;

    // Frames/ms
    double fpms;

    // The float pos
    double xpos, ypos, fpos;

    // Ticks since last pos update
    Uint32 tlast;

    virtual bool check_border() override;

public:
    // Constructor
    sge_sprite(SDL_Surface* screen, SDL_Surface* img, Sint16 x = 0, Sint16 y = 0) : sge_ssprite(screen, img, x, y)
    {
        xppms = yppms = fpms = 0;
        tlast = 0;
        xpos = x;
        ypos = y;
        fpos = 0;
    }

    sge_sprite(SDL_Surface* screen, SDL_Surface* img, sge_cdata* cdata, Sint16 x = 0, Sint16 y = 0)
        : sge_ssprite(screen, img, cdata, x, y)
    {
        xppms = yppms = fpms = 0;
        tlast = 0;
        xpos = x;
        ypos = y;
        fpos = 0;
    }

    // Change the speeds
    void set_pps(Sint16 x, Sint16 y)
    {
        xppms = x / 1000.0;
        yppms = y / 1000.0;
    }
    void set_xpps(Sint16 x) { xppms = x / 1000.0; }
    void set_ypps(Sint16 y) { yppms = y / 1000.0; }
    void set_fps(Sint16 f) { fpms = f / 1000.0; }

    // Get the speeds
    Sint16 get_xpps() const { return Sint16(xppms * 1000); }
    Sint16 get_ypps() const { return Sint16(yppms * 1000); }
    Sint16 get_fps() const { return Sint16(fpms * 1000); }

    // Update position and frame
    // Returns true if something changed
    bool update(Uint32 ticks);
    bool update() override { return update(SDL_GetTicks()); }

    // Correct move members for this class
    void move_to(Sint16 x, Sint16 y) override
    {
        sge_surface::move_to(x, y);
        xpos = current_pos.x;
        ypos = current_pos.y;
    }
    void move(Sint16 x_step, Sint16 y_step) override
    {
        sge_surface::move(x_step, y_step);
        xpos = current_pos.x;
        ypos = current_pos.y;
    }

    // Freeze time until next update
    void pause() { tlast = 0; }
};

#    ifdef _MSC_VER
#        pragma warning(pop)
#    endif

#endif /* _SGE_NO_CLASSES */
