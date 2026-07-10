// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ArchiveID.h"
#include "CControlContainer.h"

enum class WindowPos
{
    Center
};

class CWindow : public CControlContainer
{
    friend class CDebug;

    // if active is false, the window will be render behind the active windows within the game loop
    bool active = true;

protected:
    Position pos_;
    Extent size_;

private:
    const char* title;
    bool marked = true;
    bool clicked = false;
    bool canMove;
    bool canClose;
    bool canClose_marked = false;
    bool canClose_clicked = false;
    bool canMinimize;
    bool canMinimize_marked = false;
    bool canMinimize_clicked = false;
    bool canResize;
    bool canResize_marked = false;
    bool canResize_clicked = false;
    bool minimized = false;
    bool moving = false;
    bool resizing = false;
    int priority = 0; // for register, blit, event
    void (*callback_)(int);
    int callbackQuitMessage;

public:
    /// Draw the window (frame and children).
    void draw(Position parentOrigin) override;

    CWindow(void callback(int), int callbackQuitMessage, Position pos, Extent size, const char* title = nullptr,
            int color = WINDOW_GREEN1, Uint8 flags = 0, ArchiveID bgArchive = ArchiveID::EDITRES);
    CWindow(void callback(int), int callbackQuitMessage, WindowPos pos, Extent size, const char* title = nullptr,
            int color = WINDOW_GREEN1, Uint8 flags = 0, ArchiveID bgArchive = ArchiveID::EDITRES);
    // Access
    Position getPos() const { return pos_; }
    Extent getSize() const { return size_; }
    Rect getRect() const { return Rect(pos_, size_); }
    int getPriority() const { return priority; }
    void setPriority(int priority) { this->priority = priority; }
    void setTitle(const char* title);
    void setMouseData(SDL_MouseMotionEvent motion) final;
    void setMouseData(SDL_MouseButtonEvent button) final;
    void setActive()
    {
        active = true;
        marked = true;
    }
    void setInactive();
    bool isActive() const { return active; }
    bool isMoving() const { return moving; }
    bool isResizing() const { return resizing; }
    bool isMarked() const { return marked; }
    void setDirty() {}
    // we can not trust this information, cause if minimized is false, it is possible, that we still have the old
    // minimized surface bool isMinimized() { return minimized; }; we need an information if a input-element (textfield
    // etc.) is active to not deliver the input to other gui-element in the event system
    bool hasActiveInputElement();
};
