#ifndef _CMENU_H
#define _CMENU_H

#include "CControlContainer.h"

class CMenu final : public CControlContainer
{
    // if active is false, the menu will not be render within the game loop
    bool active = true;

    bool render() final;

public:
    CMenu(int pic_background);
    void setActive() { active = true; };
    void setInactive() { active = false; };
    bool isActive() { return active; };
};

#endif
