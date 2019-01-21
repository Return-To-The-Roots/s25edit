#include "CWindow.h"
#include "../CGame.h"
#include "../CSurface.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"
#include "CPicture.h"
#include "CSelectBox.h"
#include "CTextfield.h"

CWindow::CWindow(void callback(int), int callbackQuitMessage, Uint16 x, Uint16 y, Uint16 w, Uint16 h, const char* title, int color,
                 Uint8 flags)
{
    marked = true;
    clicked = false;
    this->x_ = x;
    this->y_ = y;
    // ensure window is big enough to take all basic pictures needed
    // if ( w < (global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + global::bmpArray[WINDOW_UPPER_FRAME].w +
    // global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w) )
    //    this->w = global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + global::bmpArray[WINDOW_UPPER_FRAME].w +
    //    global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w;
    // else
    this->w_ = w;
    // if ( h < (global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h) )
    //    this->h = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
    // else
    this->h_ = h;
    canMove = (flags & WINDOW_MOVE) ? true : false;
    canClose = (flags & WINDOW_CLOSE) ? true : false;
    canClose_marked = false;
    canClose_clicked = false;
    canMinimize = (flags & WINDOW_MINIMIZE) ? true : false;
    canMinimize_marked = false;
    canMinimize_clicked = false;
    canResize = (flags & WINDOW_RESIZE) ? true : false;
    canResize_marked = false;
    canResize_clicked = false;
    minimized = false;
    priority = 0;
    pic_background = color;
    for(auto& button : buttons)
        button = nullptr;
    for(auto& text : texts)
        text = nullptr;
    for(auto& picture : pictures)
        picture = nullptr;
    for(auto& static_picture : static_pictures)
    {
        static_picture.x_ = 0;
        static_picture.y_ = 0;
        static_picture.pic = -1;
    }
    for(auto& textfield : textfields)
        textfield = nullptr;
    for(auto& selectboxe : selectboxes)
        selectboxe = nullptr;

    this->title = title;
    this->callback_ = callback;
    this->callbackQuitMessage = callbackQuitMessage;
    Surf_Window = nullptr;
    needSurface = true;
    needRender = true;
    active = true;
    waste = false;
    moving = false;
    resizing = false;
}

CWindow::~CWindow()
{
    for(auto& button : buttons)
        delete button;
    for(auto& text : texts)
        delete text;
    for(auto& picture : pictures)
        delete picture;
    for(auto& textfield : textfields)
        delete textfield;
    for(auto& selectboxe : selectboxes)
        delete selectboxe;
    SDL_FreeSurface(Surf_Window);
}

void CWindow::setTitle(const char* title)
{
    this->title = title;
    needRender = true;
}

void CWindow::setColor(int color)
{
    pic_background = color;
    needRender = true;
}

bool CWindow::hasActiveInputElement()
{
    for(auto& textfield : textfields)
        if(textfield && textfield->isActive())
            return true;
    return false;
}

void CWindow::setMouseData(SDL_MouseMotionEvent motion)
{
    // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
    if((motion.x >= x_ + global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2)
       && (motion.x < x_ + w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2) && (motion.y >= y_ + 4)
       && (motion.y < y_ + +global::bmpArray[WINDOW_UPPER_FRAME].h - 4))
    {
        // left button was pressed while moving
        if(clicked)
            moving = true;
    } else if(!moving)
        clicked = false;

    if(!(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)))
        moving = false;
    if(moving && canMove)
    {
        x_ += motion.xrel;
        y_ += motion.yrel;
        // make sure to not move the window outside the display surface
        if(x_ < 0)
            x_ = 0;
        if(x_ + w_ >= global::s2->getDisplaySurface()->w) //-V807
            x_ = global::s2->getDisplaySurface()->w - w_ - 1;
        if(y_ < 0)
            y_ = 0;
        if(y_ + h_ >= global::s2->getDisplaySurface()->h)
            y_ = global::s2->getDisplaySurface()->h - h_ - 1;
    }

    // check whats happen to the close button
    if(canClose)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= x_ + 2) && (motion.x < x_ + global::bmpArray[WINDOW_BUTTON_CLOSE].w - 2) && (motion.y >= y_ + 2)
           && (motion.y < y_ + global::bmpArray[WINDOW_BUTTON_CLOSE].h - 2))
            canClose_marked = true;
        else
            canClose_marked = false;
    }
    // check whats happen to the minimize button
    if(canMinimize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= x_ + w_ - global::bmpArray[WINDOW_BUTTON_MINIMIZE].w + 2) && (motion.x < x_ + w_ - 2) && (motion.y >= y_ + 2)
           && (motion.y < y_ + global::bmpArray[WINDOW_BUTTON_MINIMIZE].h - 2))
            canMinimize_marked = true;
        else
            canMinimize_marked = false;
    }
    // check whats happen to the resize button
    if(canResize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= x_ + w_ - global::bmpArray[WINDOW_BUTTON_RESIZE].w + 2) && (motion.x < x_ + w_ - 2)
           && (motion.y >= y_ + h_ - global::bmpArray[WINDOW_BUTTON_RESIZE].h + 2) && (motion.y < y_ + h_ - 2))
        {
            // left button was pressed while moving
            if(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT))
                resizing = true;
            canResize_marked = true;
        } else if(!resizing)
            canResize_marked = false;

        if(!(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)))
            resizing = false;
        if(resizing)
        {
            // only resize if not minimized
            if(!minimized)
            {
                w_ += motion.xrel;
                h_ += motion.yrel;

                // MISSING: we have to test if window size is under minimum

                // the window has resized, so we need a new surface
                needSurface = true;
            }
        }
    }

    // deliver mouse data to the content objects of the window (if mouse cursor is inside the window)
    if((motion.x >= x_) && (motion.x < x_ + w_) && (motion.y >= y_) && (motion.y < y_ + h_))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute - y_window
        motion.x -= x_;
        motion.y -= y_;
        for(auto& picture : pictures)
        {
            if(picture)
                picture->setMouseData(motion);
        }
        for(auto& button : buttons)
        {
            if(button)
                button->setMouseData(motion);
        }
        for(auto& selectboxe : selectboxes)
        {
            if(selectboxe)
                selectboxe->setMouseData(motion);
        }
    }

    needRender = true;
}

void CWindow::setMouseData(SDL_MouseButtonEvent button)
{
    // at first check if the right mouse button was pressed, cause in this case we will close the window
    if(button.button == SDL_BUTTON_RIGHT && button.state == SDL_PRESSED)
    {
        callback_(callbackQuitMessage);
        return;
    }

    // save width and height in case we minimize the window (the initializing values are for preventing any mistakes and compilerwarning ---
    // in fact: uninitialized values are only a problem if the window is created minimized, but this will not happen)
    static int maximized_h = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
    if(!minimized)
        maximized_h = h_;

    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
        if((button.x >= x_ + global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2)
           && (button.x < x_ + w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2) && (button.y >= y_ + 4)
           && (button.y < y_ + +global::bmpArray[WINDOW_UPPER_FRAME].h - 4))
        {
            marked = true;
            clicked = true;
        }
        // pressed inside the window
        if((button.state == SDL_PRESSED) && (button.x >= x_) && (button.x <= x_ + w_) && (button.y >= y_) && (button.y <= y_ + h_))
            marked = true;
        // else pressed outside of the window
        else if(button.state == SDL_PRESSED)
            marked = false;

        // check whats happen to the close button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canClose_marked)
            canClose_clicked = true;
        else if(button.state == SDL_RELEASED)
        {
            canClose_clicked = false;
            // if mouse button is released ON the close button (marked = true), then send the quit message to the callback
            if(canClose_marked && callback_)
            {
                callback_(callbackQuitMessage);
                return;
            }
        }
        // check whats happen to the minimize button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canMinimize_marked)
            canMinimize_clicked = true;
        else if(button.state == SDL_RELEASED)
        {
            canMinimize_clicked = false;
            // if mouse button is released ON the BUTTON (marked = true), then minimize or maximize the window
            if(canMinimize_marked)
            {
                if(minimized) // maximize now
                {
                    h_ = maximized_h;
                    // the window has resized, so we need a new surface
                    needSurface = true;
                    minimized = false;
                } else // minimize now
                {
                    h_ = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
                    // the window has resized, so we need a new surface
                    needSurface = true;
                    minimized = true;
                }
            }
        }
        // check whats happen to the resize button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canResize_marked)
            canResize_clicked = true;
        else if(button.state == SDL_RELEASED)
            canResize_clicked = false;
    }

    // deliver mouse data to the content objects of the window (if mouse cursor is inside the window)
    if((button.x >= x_) && (button.x < x_ + w_) && (button.y >= y_) && (button.y < y_ + h_))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute - y_window
        button.x -= x_;
        button.y -= y_;
        for(auto& picture : pictures)
        {
            if(picture)
                picture->setMouseData(button);
        }
        for(auto& i : buttons)
        {
            if(i)
                i->setMouseData(button);
        }
        for(auto& textfield : textfields)
        {
            if(textfield)
                textfield->setMouseData(button);
        }
        for(auto& selectboxe : selectboxes)
        {
            if(selectboxe)
                selectboxe->setMouseData(button);
        }
    }

    // at least call the callback
    callback_(WINDOW_CLICKED_CALL);

    needRender = true;
}

void CWindow::setKeyboardData(const SDL_KeyboardEvent& key)
{
    for(auto& textfield : textfields)
    {
        if(textfield)
            textfield->setKeyboardData(key);
    }
    needRender = true;
}

CButton* CWindow::addButton(void callback(int), int clickedParam, Uint16 x, Uint16 y, Uint16 w, Uint16 h, int color, const char* text,
                            int picture)
{
    // x_abs and y_abs are not the left upper corner of the window, because the left and upper frames are there
    int x_abs = x + global::bmpArray[WINDOW_LEFT_FRAME].w;
    int y_abs = y + global::bmpArray[WINDOW_UPPER_FRAME].h;

    for(auto& button : buttons)
    {
        if(!button)
        {
            button = new CButton(callback, clickedParam, x_abs, y_abs, w, h, color, text, picture);
            needRender = true;
            return button;
        }
    }
    return nullptr;
}

bool CWindow::delButton(CButton* ButtonToDelete)
{
    if(!ButtonToDelete)
        return false;

    for(auto& button : buttons)
    {
        if(button == ButtonToDelete)
        {
            delete button;
            button = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

CFont* CWindow::addText(const char* string, int x, int y, int fontsize, int color)
{
    // x_abs and y_abs are not the left upper corner of the window, because the left and upper frames are there
    int x_abs = x + global::bmpArray[WINDOW_LEFT_FRAME].w;
    int y_abs = y + global::bmpArray[WINDOW_UPPER_FRAME].h;

    for(auto& text : texts)
    {
        if(!text)
        {
            text = new CFont(string, x_abs, y_abs, fontsize, color);
            needRender = true;
            return text;
        }
    }
    return nullptr;
}

bool CWindow::delText(CFont* TextToDelete)
{
    if(!TextToDelete)
        return false;

    for(auto& text : texts)
    {
        if(text == TextToDelete)
        {
            delete text;
            text = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

CPicture* CWindow::addPicture(void callback(int), int clickedParam, Uint16 x, Uint16 y, int picture)
{
    // x_abs and y_abs are not the left upper corner of the window, because the left and upper frames are there
    int x_abs = x + global::bmpArray[WINDOW_LEFT_FRAME].w;
    int y_abs = y + global::bmpArray[WINDOW_UPPER_FRAME].h;

    for(auto& i : pictures)
    {
        if(!i)
        {
            i = new CPicture(callback, clickedParam, x_abs, y_abs, picture);
            needRender = true;
            return i;
        }
    }
    return nullptr;
}

bool CWindow::delPicture(CPicture* PictureToDelete)
{
    if(!PictureToDelete)
        return false;

    for(auto& picture : pictures)
    {
        if(picture == PictureToDelete)
        {
            delete picture;
            picture = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

int CWindow::addStaticPicture(int x, int y, int picture)
{
    // x_abs and y_abs are not the left upper corner of the window, because the left and upper frames are there
    int x_abs = x + global::bmpArray[WINDOW_LEFT_FRAME].w;
    int y_abs = y + global::bmpArray[WINDOW_UPPER_FRAME].h;

    if(picture < 0)
        return -1;

    for(int i = 0; i < MAXPICTURES; i++)
    {
        if(static_pictures[i].pic == -1)
        {
            static_pictures[i].pic = picture;
            static_pictures[i].x_ = x_abs;
            static_pictures[i].y_ = y_abs;
            needRender = true;
            return i;
        }
    }
    return -1;
}

bool CWindow::delStaticPicture(int ArrayIndex)
{
    if(ArrayIndex < 0 || ArrayIndex >= MAXPICTURES)
        return false;

    static_pictures[ArrayIndex].pic = -1;
    static_pictures[ArrayIndex].x_ = 0;
    static_pictures[ArrayIndex].y_ = 0;
    needRender = true;

    return true;
}

CTextfield* CWindow::addTextfield(Uint16 x, Uint16 y, Uint16 cols, Uint16 rows, int fontsize, int text_color, int bg_color,
                                  bool button_style)
{
    // x_abs and y_abs are not the left upper corner of the window, because the left and upper frames are there
    int x_abs = x + global::bmpArray[WINDOW_LEFT_FRAME].w;
    int y_abs = y + global::bmpArray[WINDOW_UPPER_FRAME].h;

    for(auto& textfield : textfields)
    {
        if(!textfield)
        {
            textfield = new CTextfield(x_abs, y_abs, cols, rows, fontsize, text_color, bg_color, button_style);
            needRender = true;
            return textfield;
        }
    }
    return nullptr;
}

bool CWindow::delTextfield(CTextfield* TextfieldToDelete)
{
    if(!TextfieldToDelete)
        return false;

    for(auto& textfield : textfields)
    {
        if(textfield == TextfieldToDelete)
        {
            delete textfield;
            textfield = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

CSelectBox* CWindow::addSelectBox(Uint16 x, Uint16 y, Uint16 w, Uint16 h, int fontsize, int text_color, int bg_color)
{
    if(Surf_Window && (x >= Surf_Window->w || y >= Surf_Window->h))
        return nullptr;

    for(auto& selectboxe : selectboxes)
    {
        if(!selectboxe)
        {
            selectboxe = new CSelectBox(x, y, w, h, fontsize, text_color, bg_color);
            needRender = true;
            return selectboxe;
        }
    }
    return nullptr;
}

bool CWindow::delSelectBox(CSelectBox* SelectBoxToDelete)
{
    if(!SelectBoxToDelete)
        return false;

    for(auto& selectboxe : selectboxes)
    {
        if(selectboxe == SelectBoxToDelete)
        {
            delete selectboxe;
            selectboxe = nullptr;
            needRender = true;
            return true;
        }
    }
    return false;
}

bool CWindow::render()
{
    // position in the Surface 'Surf_Window'
    Uint16 pos_x = 0;
    Uint16 pos_y = 0;
    // width and height of the window background color source picture
    Uint16 pic_w = 0;
    Uint16 pic_h = 0;
    // upper frame (can be marked, clicked or normal)
    int upperframe;
    // close button (can be marked, clicked or normal)
    int closebutton = WINDOW_BUTTON_CLOSE;
    // minimize button (can be marked, clicked or normal)
    int minimizebutton = WINDOW_BUTTON_MINIMIZE;
    // resize button (can be marked, clicked or normal)
    int resizebutton = WINDOW_BUTTON_RESIZE;

    // test if a textfield has changed
    for(auto& textfield : textfields)
    {
        if(textfield)
            if(textfield->hasRendered())
                needRender = true;
    }

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(needSurface)
    {
        SDL_FreeSurface(Surf_Window);
        Surf_Window = nullptr;
        if((Surf_Window = SDL_CreateRGBSurface(SDL_SWSURFACE, w_, h_, 32, 0, 0, 0, 0)) == nullptr)
            return false;
        needSurface = false;
    }

    // at first completly fill the background (not the fastest way, but simplier)
    if(pic_background != WINDOW_NOTHING)
    {
        if(w_ <= global::bmpArray[pic_background].w)
            pic_w = w_;
        else
            pic_w = global::bmpArray[pic_background].w;

        if(h_ <= global::bmpArray[pic_background].h)
            pic_h = h_;
        else
            pic_h = global::bmpArray[pic_background].h;

        while(pos_x + pic_w <= Surf_Window->w)
        {
            while(pos_y + pic_h <= Surf_Window->h)
            {
                CSurface::Draw(Surf_Window, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, pic_w, pic_h);
                pos_y += pic_h;
            }

            if(Surf_Window->h - pos_y > 0)
                CSurface::Draw(Surf_Window, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, pic_w, Surf_Window->h - pos_y);

            pos_y = 0;
            pos_x += pic_w;
        }

        if(Surf_Window->w - pos_x > 0)
        {
            while(pos_y + pic_h <= Surf_Window->h)
            {
                CSurface::Draw(Surf_Window, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, Surf_Window->w - pos_x, pic_h);
                pos_y += pic_h;
            }

            if(Surf_Window->h - pos_y > 0)
                CSurface::Draw(Surf_Window, global::bmpArray[pic_background].surface, pos_x, pos_y, 0, 0, Surf_Window->w - pos_x,
                               Surf_Window->h - pos_y);
        }
    }

    // if not minimized, draw the content now (this stands here to prevent the frames and corners from being overdrawn)
    if(!minimized)
    {
        for(auto& button : buttons)
        {
            if(button && button->getX() < Surf_Window->w && button->getY() < Surf_Window->h)
                CSurface::Draw(Surf_Window, button->getSurface(), button->getX(), button->getY());
        }
        for(auto& static_picture : static_pictures)
        {
            if(static_picture.pic >= 0 && static_picture.x_ < Surf_Window->w && static_picture.y_ < Surf_Window->h)
                CSurface::Draw(Surf_Window, global::bmpArray[static_picture.pic].surface, static_picture.x_, static_picture.y_);
        }
        for(auto& picture : pictures)
        {
            if(picture && picture->getX() < Surf_Window->w && picture->getY() < Surf_Window->h)
                CSurface::Draw(Surf_Window, picture->getSurface(), picture->getX(), picture->getY());
        }
        for(auto& text : texts)
        {
            if(text && text->getX() < Surf_Window->w && text->getY() < Surf_Window->h)
                CSurface::Draw(Surf_Window, text->getSurface(), text->getX(), text->getY());
        }
        for(auto& textfield : textfields)
        {
            if(textfield && textfield->getX() < Surf_Window->w && textfield->getY() < Surf_Window->h)
                CSurface::Draw(Surf_Window, textfield->getSurface(), textfield->getX(), textfield->getY());
        }
        for(auto& selectboxe : selectboxes)
        {
            if(selectboxe)
                CSurface::Draw(Surf_Window, selectboxe->getSurface(), selectboxe->getX(), selectboxe->getY());
        }
    }

    // now draw the upper frame to the top
    if(clicked)
        upperframe = WINDOW_UPPER_FRAME_CLICKED;
    else if(marked)
        upperframe = WINDOW_UPPER_FRAME_MARKED;
    else
        upperframe = WINDOW_UPPER_FRAME;

    if(w_ <= global::bmpArray[upperframe].w)
        pic_w = w_;
    else
        pic_w = global::bmpArray[upperframe].w;

    pos_x = 0;
    pos_y = 0;
    while(pos_x + pic_w <= Surf_Window->w)
    {
        CSurface::Draw(Surf_Window, global::bmpArray[upperframe].surface, pos_x, pos_y);
        pos_x += pic_w;
    }

    if(Surf_Window->w - pos_x > 0)
        CSurface::Draw(Surf_Window, global::bmpArray[upperframe].surface, pos_x, pos_y, 0, 0, Surf_Window->w - pos_x, pic_h);
    // write text in the upper frame
    if(title)
        CFont::writeText(Surf_Window, title, (int)w_ / 2, (int)((global::bmpArray[WINDOW_UPPER_FRAME].h - 9) / 2), 9, FONT_YELLOW,
                         ALIGN_MIDDLE);

    // now draw the other frames (left, right, down)
    // down
    if(w_ <= global::bmpArray[WINDOW_LOWER_FRAME].w)
        pic_w = w_;
    else
        pic_w = global::bmpArray[WINDOW_LOWER_FRAME].w;

    pic_h = global::bmpArray[WINDOW_LOWER_FRAME].h;
    pos_x = 0;
    pos_y = h_ - global::bmpArray[WINDOW_LOWER_FRAME].h;
    while(pos_x + pic_w <= Surf_Window->w)
    {
        CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_LOWER_FRAME].surface, pos_x, pos_y);
        pos_x += pic_w;
    }
    if(Surf_Window->w - pos_x > 0)
        CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_LOWER_FRAME].surface, pos_x, pos_y, 0, 0, Surf_Window->w - pos_x, pic_h);
    // left
    if(h_ <= global::bmpArray[WINDOW_LEFT_FRAME].h)
        pic_h = h_;
    else
        pic_h = global::bmpArray[WINDOW_LEFT_FRAME].h;

    pic_h = global::bmpArray[WINDOW_LEFT_FRAME].h;
    pos_x = 0;
    pos_y = 0;
    while(pos_y + pic_h <= Surf_Window->h)
    {
        CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_LEFT_FRAME].surface, pos_x, pos_y);
        pos_y += pic_h;
    }
    if(Surf_Window->w - pos_x > 0)
        CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_LEFT_FRAME].surface, pos_x, pos_y, 0, 0, Surf_Window->w - pos_x, pic_h);
    // right
    if(h_ <= global::bmpArray[WINDOW_RIGHT_FRAME].h)
        pic_h = h_;
    else
        pic_h = global::bmpArray[WINDOW_RIGHT_FRAME].h;

    pic_h = global::bmpArray[WINDOW_RIGHT_FRAME].h;
    pos_x = w_ - global::bmpArray[WINDOW_RIGHT_FRAME].w;
    pos_y = 0;
    while(pos_y + pic_h <= Surf_Window->h)
    {
        CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_RIGHT_FRAME].surface, pos_x, pos_y);
        pos_y += pic_h;
    }
    if(Surf_Window->w - pos_x > 0)
        CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_RIGHT_FRAME].surface, pos_x, pos_y, 0, 0, Surf_Window->w - pos_x, pic_h);

    // now draw the corners
    CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_LEFT_UPPER_CORNER].surface, 0, 0);
    CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].surface, w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w, 0);
    CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_CORNER_RECTANGLE].surface, 0, h_ - global::bmpArray[WINDOW_CORNER_RECTANGLE].h);
    CSurface::Draw(Surf_Window, global::bmpArray[WINDOW_CORNER_RECTANGLE].surface, w_ - global::bmpArray[WINDOW_CORNER_RECTANGLE].w,
                   h_ - global::bmpArray[WINDOW_CORNER_RECTANGLE].h);
    // now the corner buttons
    // close
    if(canClose)
    {
        if(canClose_clicked)
            closebutton = WINDOW_BUTTON_CLOSE_CLICKED;
        else if(canClose_marked)
            closebutton = WINDOW_BUTTON_CLOSE_MARKED;
        else
            closebutton = WINDOW_BUTTON_CLOSE;
        CSurface::Draw(Surf_Window, global::bmpArray[closebutton].surface, 0, 0);
    }
    // minimize
    if(canMinimize)
    {
        if(canMinimize_clicked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_CLICKED;
        else if(canMinimize_marked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_MARKED;
        else
            minimizebutton = WINDOW_BUTTON_MINIMIZE;
        CSurface::Draw(Surf_Window, global::bmpArray[minimizebutton].surface, w_ - global::bmpArray[minimizebutton].w, 0);
    }
    // resize
    if(canResize)
    {
        if(canResize_clicked)
            resizebutton = WINDOW_BUTTON_RESIZE_CLICKED;
        else if(canResize_marked)
            resizebutton = WINDOW_BUTTON_RESIZE_MARKED;
        else
            resizebutton = WINDOW_BUTTON_RESIZE;
        CSurface::Draw(Surf_Window, global::bmpArray[resizebutton].surface, w_ - global::bmpArray[resizebutton].w,
                       h_ - global::bmpArray[resizebutton].h);
    }

    return true;
}

void CWindow::setInactive()
{
    active = false;
    clicked = false;
    marked = false;
    needRender = true;

    for(auto& textfield : textfields)
    {
        if(textfield)
            textfield->setInactive();
    }
}
