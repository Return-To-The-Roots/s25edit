#include "CSelectBox.h"

CSelectBox::CSelectBox(Uint16 x, Uint16 y, Uint16 w, Uint16 h, int fontsize, int text_color, int bg_color)
{
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    last_text_pos_y = 10;
    this->fontsize = fontsize;
    this->text_color = text_color;
    setColor(bg_color);
    // initialize CFont array
    for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
        Entries[i] = NULL;

    Surf_SelectBox = NULL;
    needSurface = true;
    needRender = true;
    rendered = false;
    // button position is relative to the selectbox
    ScrollUpButton = new CButton(NULL, 0, w - 1 - 20, 0, 20, 20, BUTTON_GREY, NULL, PICTURE_SMALL_ARROW_UP);
    ScrollDownButton = new CButton(NULL, 0, w - 1 - 20, h - 1 - 20, 20, 20, BUTTON_GREY, NULL, PICTURE_SMALL_ARROW_DOWN);
}

CSelectBox::~CSelectBox()
{
    for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
    {
        if(Entries[i] == NULL)
        {
            delete Entries[i];
            Entries[i] = NULL;
        }
    }
    delete ScrollUpButton;
    delete ScrollDownButton;
    SDL_FreeSurface(Surf_SelectBox);
}

void CSelectBox::setOption(const char* string, void (*callback)(int), int param)
{
    setOption((unsigned char*)string, callback, param);
}

void CSelectBox::setOption(unsigned char* string, void (*callback)(int), int param)
{
    // explanation: row_height = row_separator + fontsize
    int row_height = (fontsize == 9 ? 1 : (fontsize == 11 ? 3 : 4)) + fontsize;

    for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
    {
        if(Entries[i] == NULL)
        {
            Entries[i] = new CFont(string, 10, last_text_pos_y, fontsize, FONT_YELLOW);
            Entries[i]->setCallback(callback, param);
            last_text_pos_y += row_height;
            break;
        }
    }
}

bool CSelectBox::hasRendered(void)
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
    motion.x -= x;
    motion.y -= y;
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
            if((button.x >= x) && (button.x < x + w) && (button.y >= y) && (button.y < y + h))
            {
                // scroll up button
                if((button.x > x + w - 20) && (button.y < y + 20))
                {
                    scroll_up_button_marked = true;
                }
                // scroll down button
                else if((button.x > x + w - 20) && (button.y > y + h - 20))
                {
                    scroll_down_button_marked = true;
                }

                // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
                //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox, y_absolute - y_selectbox
                button.x -= x;
                button.y -= y;
                manipulated = true;

                for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
                {
                    if(Entries[i] != NULL)
                        Entries[i]->setMouseData(button);
                }
            }
        } else if(button.state == SDL_RELEASED)
        {
            if((button.x >= x) && (button.x < x + w) && (button.y >= y) && (button.y < y + h))
            {
                // scroll up button
                if(scroll_up_button_marked)
                {
                    if((button.x > x + w - 20) && (button.y < y + 20))
                    {
                        // test if first entry is on the most upper position
                        if(Entries[0] != NULL && Entries[0]->getY() < 10)
                        {
                            for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
                            {
                                if(Entries[i] != NULL)
                                    Entries[i]->setY(Entries[i]->getY() + 10);
                            }
                        }
                    }
                }
                // scroll down button
                else if(scroll_down_button_marked)
                {
                    if((button.x > x + w - 20) && (button.y > y + h - 20))
                    {
                        // test if last entry is on the most lower position
                        int j;
                        for(j = 0; j < MAXSELECTBOXENTRIES; j++)
                        {
                            if(Entries[j] == NULL)
                                break;
                        }
                        j--;
                        if(Entries[j] != NULL && Entries[j]->getY() > h - 10)
                        {
                            for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
                            {
                                if(Entries[i] != NULL)
                                    Entries[i]->setY(Entries[i]->getY() - 10);
                            }
                        }
                    }
                }

                // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
                //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox, y_absolute - y_selectbox
                button.x -= x;
                button.y -= y;
                manipulated = true;

                for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
                {
                    if(Entries[i] != NULL)
                        Entries[i]->setMouseData(button);
                }
            }
            scroll_up_button_marked = false;
            scroll_down_button_marked = false;
        }
        // IMPORTANT: we use the left upper corner of the selectbox as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons and entries: x_absolute - x_selectbox, y_absolute - y_selectbox
        if(!manipulated)
        {
            button.x -= x;
            button.y -= y;
        }
        ScrollUpButton->setMouseData(button);
        ScrollDownButton->setMouseData(button);
    }

    needRender = true;
}

bool CSelectBox::render(void)
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
    if(needSurface)
    {
        SDL_FreeSurface(Surf_SelectBox);
        Surf_SelectBox = NULL;
        if((Surf_SelectBox = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0)) == NULL)
            return false;
        needSurface = false;
    }

    // draw the pictures for background and foreground or, if not set, fill with black color
    if(pic_background >= 0 && pic_foreground >= 0)
    {
        int pic;
        pic = pic_foreground;

        // at first completly fill the background (not the fastest way, but simplier)
        if(w <= global::bmpArray[pic].w)
            pic_w = w;
        else
            pic_w = global::bmpArray[pic].w;

        if(h <= global::bmpArray[pic].h)
            pic_h = h;
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
        SDL_FillRect(Surf_SelectBox, NULL, SDL_MapRGB(Surf_SelectBox->format, 0, 0, 0));

    for(int i = 0; i < MAXSELECTBOXENTRIES; i++)
    {
        if(Entries[i] != NULL)
            CSurface::Draw(Surf_SelectBox, Entries[i]->getSurface(), Entries[i]->getX(), Entries[i]->getY());
    }

    CSurface::Draw(Surf_SelectBox, ScrollUpButton->getSurface(), w - 1 - 20, 0);
    CSurface::Draw(Surf_SelectBox, ScrollDownButton->getSurface(), w - 1 - 20, h - 1 - 20);

    rendered = true;

    return true;
}
