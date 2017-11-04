#include "CGame.h"

bool CGame::Init()
{
    std::cout << "Return to the Roots Mapeditor\n";

    std::cout << "\nInitializing SDL...";
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "failure";
        return false;
    }

    SDL_EnableKeyRepeat(100, 100);
    SDL_ShowCursor(SDL_DISABLE);

    std::cout << "\nCreate Window...";
    if(CSurface::useOpenGL)
    {
        Surf_DisplayGL = SDL_SetVideoMode(GameResolutionX, GameResolutionY, 32, SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
        Surf_Display = SDL_CreateRGBSurface(SDL_SWSURFACE, GameResolutionX, GameResolutionY, 32, 0, 0, 0, 0);
        if(Surf_Display == NULL || Surf_DisplayGL == NULL)
        {
            std::cout << "failure";
            return false;
        }
    } else
    {
        Surf_Display =
          SDL_SetVideoMode(GameResolutionX, GameResolutionY, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
        if(Surf_Display == NULL)
        {
            std::cout << "failure";
            return false;
        }
    }

    SDL_WM_SetCaption("Return to the Roots Mapeditor", 0);

    /*NOTE: its important to load a palette at first,
     *      otherwise all images will be black (in exception of the LBM-Files, they have their own palette).
     *      if its necessary to load pictures from a
     *      specified file with a special palette, so
     *      load this palette from a file and set
     *      CFile::palActual = CFile::palArray - 1 (--> last loaden palette)
     *      and after loading the images set
     *      CFile::palActual = CFile::palArray (--> first palette)
     */

    // load some pictures (after all the splash-screens)
    // at first /GFX/PICS/SETUP997.LBM, cause this is the S2-loading picture
    std::cout << "\nLoading file: /GFX/PICS/SETUP997.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP997.LBM", LBM) == false)
    {
        std::cout << "failure";
        // if SETUP997.LBM doesn't exist, it's probably settlers2+missioncd and there we have SETUP998.LBM instead
        std::cout << "\nTry to load file: /GFX/PICS/SETUP998.LBM instead...";
        if(CFile::open_file("./GFX/PICS/SETUP998.LBM", LBM) == false)
        {
            std::cout << "failure";
            return false;
        }
    }

    // std::cout << "\nShow loading screen...";
    showLoadScreen = true;
    // CSurface::Draw(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0);
    sge_TexturedRect(
      Surf_Display, 0, 0, Surf_Display->w - 1, 0, 0, Surf_Display->h - 1, Surf_Display->w - 1, Surf_Display->h - 1,
      global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->w - 1, 0, 0,
      global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->h - 1, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->w - 1,
      global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface->h - 1);
    SDL_Flip(Surf_Display);

    // continue loading pictures
    std::cout << "\nLoading file: /GFX/PICS/SETUP000.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP000.LBM", LBM) == false)
    {
        std::cout << "failure";
        // if SETUP000.LBM doesn't exist, it's probably settlers2+missioncd and we simply load SETUP010.LBM instead
        std::cout << "\nLoading file: /GFX/PICS/SETUP010.LBM instead...";
        if(CFile::open_file("./GFX/PICS/SETUP010.LBM", LBM) == false)
        {
            std::cout << "failure";
            return false;
        }
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP010.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP010.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP011.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP011.LBM", LBM) == false)
    {
        std::cout << "failure";
        // if SETUP011.LBM doesn't exist, it's probably settlers2+missioncd and we simply load SETUP010.LBM instead
        std::cout << "\nLoading file: /GFX/PICS/SETUP010.LBM instead...";
        if(CFile::open_file("./GFX/PICS/SETUP010.LBM", LBM) == false)
        {
            std::cout << "failure";
            return false;
        }
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP012.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP012.LBM", LBM) == false)
    {
        std::cout << "failure";
        // if SETUP012.LBM doesn't exist, it's probably settlers2+missioncd and we simply load SETUP010.LBM instead
        std::cout << "\nLoading file: /GFX/PICS/SETUP010.LBM instead...";
        if(CFile::open_file("./GFX/PICS/SETUP010.LBM", LBM) == false)
        {
            std::cout << "failure";
            return false;
        }
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP013.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP013.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP014.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP014.LBM", LBM) == false)
    {
        std::cout << "failure";
        // if SETUP014.LBM doesn't exist, it's probably settlers2+missioncd and we simply load SETUP010.LBM instead
        std::cout << "\nLoading file: /GFX/PICS/SETUP010.LBM instead...";
        if(CFile::open_file("./GFX/PICS/SETUP010.LBM", LBM) == false)
        {
            std::cout << "failure";
            return false;
        }
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP015.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP015.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP666.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP666.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP667.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP667.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP801.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP801.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP802.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP802.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP803.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP803.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP804.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP804.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP805.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP805.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP806.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP806.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP810.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP810.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP811.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP811.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP895.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP895.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP896.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP896.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP897.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP897.LBM", LBM) == false)
    {
        std::cout << "failure";
        // if SETUP897.LBM doesn't exist, it's probably settlers2+missioncd and we simply load SETUP896.LBM instead
        std::cout << "\nLoading file: /GFX/PICS/SETUP896.LBM instead...";
        if(CFile::open_file("./GFX/PICS/SETUP896.LBM", LBM) == false)
        {
            std::cout << "failure";
            return false;
        }
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP898.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP898.LBM", LBM) == false)
    {
        std::cout << "failure";
        // if SETUP897.LBM doesn't exist, it's probably settlers2+missioncd and we simply load SETUP896.LBM instead
        std::cout << "\nLoading file: /GFX/PICS/SETUP896.LBM instead...";
        if(CFile::open_file("./GFX/PICS/SETUP896.LBM", LBM) == false)
        {
            std::cout << "failure";
            return false;
        }
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP899.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP899.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP990.LBM...";
    if(CFile::open_file("./GFX/PICS/SETUP990.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/WORLD.LBM...";
    if(CFile::open_file("./GFX/PICS/WORLD.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/WORLDMSK.LBM...";
    if(CFile::open_file("./GFX/PICS/WORLDMSK.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    // load gouraud data
    std::cout << "\nLoading file: /DATA/TEXTURES/GOU5.DAT...";
    if(CFile::open_file("./DATA/TEXTURES/GOU5.DAT", GOU) == false)
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /DATA/TEXTURES/GOU6.DAT...";
    if(CFile::open_file("./DATA/TEXTURES/GOU6.DAT", GOU) == false)
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /DATA/TEXTURES/GOU7.DAT...";
    if(CFile::open_file("./DATA/TEXTURES/GOU7.DAT", GOU) == false)
    {
        std::cout << "failure";
        return false;
    }

#ifdef _EDITORMODE
    // load only the palette at this time from editres.idx
    std::cout << "\nLoading palette from file: /DATA/EDITRES.IDX...";
    if(CFile::open_file("./DATA/EDITRES.IDX", IDX, true) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/EDITRES.IDX...";
    if(CFile::open_file("./DATA/EDITRES.IDX", IDX) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    // load only the palette at this time from editio.idx
    std::cout << "\nLoading palette from file: /DATA/IO/EDITIO.IDX...";
    if(CFile::open_file("./DATA/IO/EDITIO.IDX", IDX, true) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/IO/EDITIO.IDX...";
    if(CFile::open_file("./DATA/IO/EDITIO.IDX", IDX) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    std::cout << "\nLoading file: /DATA/EDITBOB.LST...";
    if(CFile::open_file("./DATA/EDITBOB.LST", LST) == false)
    {
        std::cout << "failure";
        return false;
    }
#else
    // load only the palette at this time from resource.idx
    std::cout << "\nLoading palette from file: /DATA/RESOURCE.IDX...";
    if(CFile::open_file("./DATA/RESOURCE.IDX", IDX, true) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/RESOURCE.IDX...";
    if(CFile::open_file("./DATA/RESOURCE.IDX", IDX) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    // load only the palette at this time from io.idx
    std::cout << "\nLoading palette from file: /DATA/IO/IO.IDX...";
    if(CFile::open_file("./DATA/IO/IO.IDX", IDX, true) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/IO/IO.IDX...";
    if(CFile::open_file("./DATA/IO/IO.IDX", IDX) == false)
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
#endif

    // texture tilesets
    std::cout << "\nLoading file: /GFX/TEXTURES/TEX5.LBM...";
    if(CFile::open_file("./GFX/TEXTURES/TEX5.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /GFX/TEXTURES/TEX6.LBM...";
    if(CFile::open_file("./GFX/TEXTURES/TEX6.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /GFX/TEXTURES/TEX7.LBM...";
    if(CFile::open_file("./GFX/TEXTURES/TEX7.LBM", LBM) == false)
    {
        std::cout << "failure";
        return false;
    }

    /*
    std::cout << "\nLoading palette from file: /GFX/PALETTE/PAL5.BBM...";
    if ( CFile::open_file("./GFX/PALETTE/PAL5.BBM", BBM, true) == false )
    {
        std::cout << "failure";
        return false;
    }
    */

    // EVERY MISSION-FILE SHOULD BE LOADED SEPARATLY IF THE SPECIFIED MISSION GOES ON -- SO THIS IS TEMPORARY
    std::cout << "\nLoading file: /DATA/MIS0BOBS.LST...";
    if(CFile::open_file("./DATA/MIS0BOBS.LST", LST) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS1BOBS.LST...";
    if(CFile::open_file("./DATA/MIS1BOBS.LST", LST) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS2BOBS.LST...";
    if(CFile::open_file("./DATA/MIS2BOBS.LST", LST) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS3BOBS.LST...";
    if(CFile::open_file("./DATA/MIS3BOBS.LST", LST) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS4BOBS.LST...";
    if(CFile::open_file("./DATA/MIS4BOBS.LST", LST) == false)
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS5BOBS.LST...";
    if(CFile::open_file("./DATA/MIS5BOBS.LST", LST) == false)
    {
        std::cout << "failure";
        return false;
    }

    // create the mainmenu
    callback::mainmenu(INITIALIZING_CALL);

    return true;
}
