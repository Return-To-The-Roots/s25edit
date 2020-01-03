#include "CSelectBox.h"
#include "../CSurface.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"

CSelectBox::CSelectBox(Sint16 x, Sint16 y, Uint16 w, Uint16 h, int fontsize, int text_color, int bg_color)
{
    this->x_ = x;
    this->y_ = y;
    this->w_ = w;
    this->h_ = h;
    last_text_pos_y = 10;
    this->fontsize = fontsize;
    this->text_color = text_color;
    setColor(bg_color);

    needRender = true;
    rendered = false;
    // button position is relative to the selectbox
    ScrollUpButton = std::make_unique<CButton>(nullptr, 0, w - 1 - 20, 0, 20, 20, BUTTON_GREY, nullptr, PICTURE_SMALL_ARROW_UP);
    ScrollDownButton =
      std::make_unique<CButton>(nullptr, 0, w - 1 - 20, h - 1 - 20, 20, 20, BUTTON_GREY, nullptr, PICTURE_SMALL_ARROW_DOWN);
}

void CSelectBox::setOption(const std::string& string, std::function<void(int)> callback, int param)
{
    // explanation: row_height = row_separator + fontsize
    int row_height = (fontsize == 9 ? 1 : (fontsize == 11 ? 3 : 4)) + fontsize;

    auto Entry = std::make_unique<CFont>(string, 10, last_text_pos_y, fontsize, FONT_YELLOW);
    Entry->setCallback(std::move(callback), param);
    Entries.emplace_back(std::move(Entry));
    last_text_pos_y += row_height;
}

bool CSelectBox::hasRendered()
{
    if(rendered)
    {
        rendered = false;
        return true;
    } else
        return false;
}

void CSelectBox::setColor(int color)
{
    switch(color)
    {
        case BUTTON_GREY:
            pic_foreground = BUTTON_GREY_DARK;
            pic_background = BUTTON_GREY_BACKGROUND;
            break;

        case BUTTON_RED1:
            pic_foreground = BUTTON_RED1_DARK;
            pic_background = BUTTON_RED1_BACKGROUND;
            break;

        case BUTTON_GREEN1:
            pic_foreground = BUTTON_GREEN1_DARK;
            pic_background = BUTTON_GREEN1_BACKGROUND;
            break;

        case BUTTON_GREEN2:
            pic_foreground = BUTTON_GREEN2_DARK;
            pic_background = BUTTON_GREEN2_BACKGROUND;
            break;

        case BUTTON_RED2:
            pic_foreground = BUTTON_RED2_DARK;
            pic_background = BUTTON_RED2_BACKGROUND;
            break;

        case BUTTON_STONE:
            pic_foreground = BUTTON_STONE_DARK;
            pic_background = BUTTON_STONE_BACKGROUND;
            break;

        default:
            pic_foreground = -1;
            pic_background = -1;
            break;
    }

    needRender = true;
}

void CSelectBox::setMouseData(SDL_MouseMotionEvent motion)
{
    // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
    //           the motion-structure before give it to the buttons: x_absolute - x_selectbox, y_absolute - y_selectbox
    motion.x -= x_;
    motion.y -= y_;
    ScrollUpButton->setMouseData(motion);
    ScrollDownButton->setMouseData(motion);
    needRender = true;
}

void CSelectBox::setMouseData(SDL_MouseButtonEvent button)
{
    bool manipulated = false;
    static bool scroll_up_button_marked = false;
    static bool scroll_down_button_marked = false;

    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // if mouse button is pressed ON the selectbox
        if(button.state == SDL_PRESSED)
        {
            if((button.x >= x_) && (button.x < x_ + w_) && (button.y >= y_) && (button.y < y_ + h_))
            {
                // scroll up button
                if((button.x > x_ + w_ - 20) && (button.y < y_ + 20))
                {
                    scroll_up_button_marked = true;
                }
                // scroll down button
                else if((button.x > x_ + w_ - 20) && (button.y > y_ + h_ - 20))
                {
                    scroll_down_button_marked = true;
                }

                // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
                //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox, y_absolute - y_selectbox
                button.x -= x_;
                button.y -= y_;
                manipulated = true;

                for(auto& entry : Entries)
                {
                    entry->setMouseData(button);
                }
            }
        } else if(button.state == SDL_RELEASED)
        {
            if((button.x >= x_) && (button.x < x_ + w_) && (button.y >= y_) && (button.y < y_ + h_))
            {
                // scroll up button
                if(scroll_up_button_marked)
                {
                    if((button.x > x_ + w_ - 20) && (button.y < y_ + 20))
                    {
                        // test if first entry is on the most upper position
                        if(Entries[0] != nullptr && Entries[0]->getY() < 10)
                        {
                            for(auto& entry : Entries)
                            {
                                entry->setPos(Position(entry->getX(), entry->getY() + 10));
                            }
                        }
                    }
                }
                // scroll down button
                else if(scroll_down_button_marked)
                {
                    if((button.x > x_ + w_ - 20) && (button.y > y_ + h_ - 20))
                    {
                        // test if last entry is on the most lower position
                        if(!Entries.empty() && Entries.back()->getY() > h_ - 10)
                        {
                            for(auto& entry : Entries)
                            {
                                entry->setPos(Position(entry->getX(), entry->getY() - 10));
                            }
                        }
                    }
                }

                // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
                //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox, y_absolute - y_selectbox
                button.x -= x_;
                button.y -= y_;
                manipulated = true;

                for(auto& entry : Entries)
                {
                    entry->setMouseData(button);
                }
            }
            scroll_up_button_marked = false;
            scroll_down_button_marked = false;
        }
        // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox, y_absolute - y_selectbox
        if(!manipulated)
        {
            button.x -= x_;
            button.y -= y_;
        }
        ScrollUpButton->setMouseData(button);
        ScrollDownButton->setMouseData(button);
    }

    needRender = true;
}

bool CSelectBox::render()
{
    // position in the Surface 'Surf_Button'
    Uint16 pos_x = 0;
    Uint16 pos_y = 0;
    // width and height of the button color source picture
    Uint16 pic_w = 0;
    Uint16 pic_h = 0;

    // if we don't need to render, all is up to date, return true
    if(!needRender)
        return true;
    needRender = false;
    // if we need a new surface
    if(!Surf_SelectBox)
    {
        if((Surf_SelectBox = makeSdlSurface(SDL_SWSURFACE, w_, h_, 32)) == nullptr)
            return false;
    }

    // draw the pictures for background and foreground or, if not set, fill with black color
    if(pic_background >= 0 && pic_foreground >= 0)
    {
        int pic;
        pic = pic_foreground;

        // at first completly fill the background (not the fastest way, but simplier)
        if(w_ <= global::bmpArray[pic].w)
            pic_w = w_;
        else
            pic_w = global::bmpArray[pic].w;

        if(h_ <= global::bmpArray[pic].h)
            pic_h = h_;
        else
            pic_h = global::bmpArray[pic].h;

        while(pos_x + pic_w <= Surf_SelectBox->w)
        {
            while(pos_y + pic_h <= Surf_SelectBox->h)
            {
                CSurface::Draw(Surf_SelectBox, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, pic_w, pic_h);
                pos_y += pic_h;
            }

            if(Surf_SelectBox->h - pos_y > 0)
                CSurface::Draw(Surf_SelectBox, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, pic_w, Surf_SelectBox->h - pos_y);

            pos_y = 0;
            pos_x += pic_w;
        }

        if(Surf_SelectBox->w - pos_x > 0)
        {
            while(pos_y + pic_h <= Surf_SelectBox->h)
            {
                CSurface::Draw(Surf_SelectBox, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, Surf_SelectBox->w - pos_x, pic_h);
                pos_y += pic_h;
            }

            if(Surf_SelectBox->h - pos_y > 0)
                CSurface::Draw(Surf_SelectBox, global::bmpArray[pic].surface, pos_x, pos_y, 0, 0, Surf_SelectBox->w - pos_x,
                               Surf_SelectBox->h - pos_y);
        }
    } else
        SDL_FillRect(Surf_SelectBox.get(), nullptr, SDL_MapRGB(Surf_SelectBox->format, 0, 0, 0));

    for(auto& entry : Entries)
    {
        CSurface::Draw(Surf_SelectBox, entry->getSurface(), entry->getX(), entry->getY());
    }

    CSurface::Draw(Surf_SelectBox, ScrollUpButton->getSurface(), w_ - 1 - 20, 0);
    CSurface::Draw(Surf_SelectBox, ScrollDownButton->getSurface(), w_ - 1 - 20, h_ - 1 - 20);

    rendered = true;

    return true;
}
