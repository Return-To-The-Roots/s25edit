// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CGame.h"
#include "WindowManager.h"
#include "drivers/VideoDriverWrapper.h"
#include "enum_cast.hpp"
#include "globals.h"
#include "s25util/utf8.h"

void CGame::ForwardEventToWindowManager(SDL_Event* ev)
{
    static MouseCoords mouse_xy;

    switch(ev->type)
    {
        case SDL_WINDOWEVENT:
            if(ev->window.event == SDL_WINDOWEVENT_RESIZED)
                WINDOWMANAGER.WindowResized();
            break;

        case SDL_KEYDOWN:
        {
            KeyEvent ke;
            switch(ev->key.keysym.sym)
            {
                case SDLK_RETURN: ke.kt = KeyType::Return; break;
                case SDLK_SPACE: ke.kt = KeyType::Space; break;
                case SDLK_LEFT: ke.kt = KeyType::Left; break;
                case SDLK_RIGHT: ke.kt = KeyType::Right; break;
                case SDLK_UP: ke.kt = KeyType::Up; break;
                case SDLK_DOWN: ke.kt = KeyType::Down; break;
                case SDLK_BACKSPACE: ke.kt = KeyType::Backspace; break;
                case SDLK_DELETE: ke.kt = KeyType::Delete; break;
                case SDLK_TAB: ke.kt = KeyType::Tab; break;
                case SDLK_HOME: ke.kt = KeyType::Home; break;
                case SDLK_END: ke.kt = KeyType::End; break;
                case SDLK_ESCAPE: ke.kt = KeyType::Escape; break;
                case SDLK_PRINTSCREEN: ke.kt = KeyType::Print; break;
                default:
                    if(ev->key.keysym.sym >= SDLK_F1 && ev->key.keysym.sym <= SDLK_F12)
                        ke.kt = static_cast<KeyType>(rttr::enum_cast(KeyType::F1) + ev->key.keysym.sym - SDLK_F1);
                    break;
            }
            ke.alt = (ev->key.keysym.mod & KMOD_ALT) != 0;
            ke.ctrl = (ev->key.keysym.mod & KMOD_CTRL) != 0;
            ke.shift = (ev->key.keysym.mod & KMOD_SHIFT) != 0;
            if(ke.kt != KeyType::Invalid)
                WINDOWMANAGER.Msg_KeyDown(ke);
            break;
        }

        case SDL_TEXTINPUT:
        {
            const std::u32string text = s25util::utf8to32(ev->text.text);
            KeyEvent ke;
            for(char32_t c : text)
            {
                ke.c = c;
                WINDOWMANAGER.Msg_KeyDown(ke);
            }
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
            mouse_xy.pos = Position(ev->button.x, ev->button.y);
            VIDEODRIVER.SetMousePos(mouse_xy.pos);
            switch(ev->button.button)
            {
                case SDL_BUTTON_LEFT:
                    mouse_xy.ldown = true;
                    WINDOWMANAGER.Msg_LeftDown(mouse_xy);
                    break;
                case SDL_BUTTON_RIGHT:
                    mouse_xy.rdown = true;
                    WINDOWMANAGER.Msg_RightDown(mouse_xy);
                    break;
                case SDL_BUTTON_MIDDLE:
                    mouse_xy.mdown = true;
                    WINDOWMANAGER.Msg_MiddleDown(mouse_xy);
                    break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            mouse_xy.pos = Position(ev->button.x, ev->button.y);
            VIDEODRIVER.SetMousePos(mouse_xy.pos);
            switch(ev->button.button)
            {
                case SDL_BUTTON_LEFT:
                    mouse_xy.ldown = false;
                    WINDOWMANAGER.Msg_LeftUp(mouse_xy);
                    break;
                case SDL_BUTTON_RIGHT:
                    mouse_xy.rdown = false;
                    WINDOWMANAGER.Msg_RightUp(mouse_xy);
                    break;
                case SDL_BUTTON_MIDDLE:
                    mouse_xy.mdown = false;
                    WINDOWMANAGER.Msg_MiddleUp(mouse_xy);
                    break;
            }
            break;

        case SDL_MOUSEWHEEL:
        {
            int y = ev->wheel.y;
            if(ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
                y = -y;
            if(y > 0)
                WINDOWMANAGER.Msg_WheelUp(mouse_xy);
            else if(y < 0)
                WINDOWMANAGER.Msg_WheelDown(mouse_xy);
            break;
        }

        case SDL_MOUSEMOTION:
        {
            const Position newPos(ev->motion.x, ev->motion.y);
            if(newPos != mouse_xy.pos)
            {
                mouse_xy.pos = newPos;
                VIDEODRIVER.SetMousePos(newPos);
                WINDOWMANAGER.Msg_MouseMove(mouse_xy);
            }
            break;
        }
    }
}
