#include "CGame.h"
#include "CIO/CFile.h"
#include "CSurface.h"
#include "callbacks.h"
#include "globals.h"
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <vector>

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

    SDL_WM_SetCaption("Return to the Roots Mapeditor [BETA]", 0);
    SetAppIcon();

    CFile::init();

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
    if(!CFile::open_file(global::gameDataFilePath + "/GFX/PICS/SETUP997.LBM", LBM))
    {
        std::cout << "failure";
        // if SETUP997.LBM doesn't exist, it's probably settlers2+missioncd and there we have SETUP998.LBM instead
        std::cout << "\nTry to load file: /GFX/PICS/SETUP998.LBM instead...";
        if(!CFile::open_file(global::gameDataFilePath + "/GFX/PICS/SETUP998.LBM", LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    // std::cout << "\nShow loading screen...";
    showLoadScreen = true;
    // CSurface::Draw(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0);
    SDL_Surface* surfSplash = global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface;
    sge_TexturedRect(Surf_Display, 0, 0, Surf_Display->w - 1, 0, 0, Surf_Display->h - 1, Surf_Display->w - 1, Surf_Display->h - 1,
                     surfSplash, 0, 0, surfSplash->w - 1, 0, 0, surfSplash->h - 1, surfSplash->w - 1, surfSplash->h - 1);
    SDL_Flip(Surf_Display);

    // continue loading pictures
    using namespace boost::assign;
    std::vector<std::string> paths;
    paths += "/GFX/PICS/SETUP000.LBM", "/GFX/PICS/SETUP010.LBM", "/GFX/PICS/SETUP011.LBM", "/GFX/PICS/SETUP012.LBM",
      "/GFX/PICS/SETUP013.LBM", "/GFX/PICS/SETUP014.LBM", "/GFX/PICS/SETUP015.LBM";
    BOOST_FOREACH(const std::string& file, paths)
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath + file, LBM))
        {
            std::cout << "failure";
            // if it doesn't exist, it's probably settlers2+missioncd and we simply load SETUP010.LBM instead
            std::cout << "\nLoading file: /GFX/PICS/SETUP010.LBM instead...";
            if(!CFile::open_file(global::gameDataFilePath + "/GFX/PICS/SETUP010.LBM", LBM))
            {
                std::cout << "failure";
                return false;
            }
        }
    }

    paths.clear();
    paths += "/GFX/PICS/SETUP666.LBM", "/GFX/PICS/SETUP667.LBM", "/GFX/PICS/SETUP801.LBM", "/GFX/PICS/SETUP802.LBM",
      "/GFX/PICS/SETUP803.LBM", "/GFX/PICS/SETUP804.LBM", "/GFX/PICS/SETUP805.LBM", "/GFX/PICS/SETUP806.LBM", "/GFX/PICS/SETUP810.LBM",
      "/GFX/PICS/SETUP811.LBM", "/GFX/PICS/SETUP895.LBM", "/GFX/PICS/SETUP896.LBM";
    BOOST_FOREACH(const std::string& file, paths)
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath + file, LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    paths.clear();
    paths += "/GFX/PICS/SETUP897.LBM", "/GFX/PICS/SETUP898.LBM";
    BOOST_FOREACH(const std::string& file, paths)
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath + file, LBM))
        {
            std::cout << "failure";
            // if it doesn't exist, it's probably settlers2+missioncd and we simply load SETUP896.LBM instead
            std::cout << "\nLoading file: /GFX/PICS/SETUP896.LBM instead...";
            if(!CFile::open_file(global::gameDataFilePath + "/GFX/PICS/SETUP896.LBM", LBM))
            {
                std::cout << "failure";
                return false;
            }
        }
    }

    paths.clear();
    paths += "/GFX/PICS/SETUP899.LBM", "/GFX/PICS/SETUP990.LBM", "/GFX/PICS/WORLD.LBM", "/GFX/PICS/WORLDMSK.LBM";
    std::cout << "\nLoading file: /GFX/PICS/SETUP899.LBM...";
    BOOST_FOREACH(const std::string& file, paths)
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath + file, LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    // load gouraud data
    paths.clear();
    paths += "/DATA/TEXTURES/GOU5.DAT", "/DATA/TEXTURES/GOU6.DAT", "/DATA/TEXTURES/GOU7.DAT";
    std::cout << "\nLoading file: /DATA/TEXTURES/GOU5.DAT...";
    BOOST_FOREACH(const std::string& file, paths)
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath + file, GOU))
        {
            std::cout << "failure";
            return false;
        }
    }

#ifdef _EDITORMODE
    // load only the palette at this time from editres.idx
    std::cout << "\nLoading palette from file: /DATA/EDITRES.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/EDITRES.IDX", IDX, true))
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/EDITRES.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/EDITRES.IDX", IDX))
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    // load only the palette at this time from editio.idx
    std::cout << "\nLoading palette from file: /DATA/IO/EDITIO.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/IO/EDITIO.IDX", IDX, true))
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/IO/EDITIO.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/IO/EDITIO.IDX", IDX))
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    std::cout << "\nLoading file: /DATA/EDITBOB.LST...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/EDITBOB.LST", LST))
    {
        std::cout << "failure";
        return false;
    }
#else
    // load only the palette at this time from resource.idx
    std::cout << "\nLoading palette from file: /DATA/RESOURCE.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/RESOURCE.IDX", IDX, true))
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/RESOURCE.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/RESOURCE.IDX", IDX))
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
    // load only the palette at this time from io.idx
    std::cout << "\nLoading palette from file: /DATA/IO/IO.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/IO/IO.IDX", IDX, true))
    {
        std::cout << "failure";
        return false;
    }
    // set the right palette
    CFile::set_palActual(CFile::get_palArray() - 1);
    std::cout << "\nLoading file: /DATA/IO/IO.IDX...";
    if(!CFile::open_file(global::gameDataFilePath + "/DATA/IO/IO.IDX", IDX))
    {
        std::cout << "failure";
        return false;
    }
    // set back palette
    CFile::set_palActual(CFile::get_palArray());
#endif

    // texture tilesets
    paths.clear();
    paths += "/GFX/TEXTURES/TEX5.LBM", "/GFX/TEXTURES/TEX6.LBM", "/GFX/TEXTURES/TEX7.LBM";
    BOOST_FOREACH(const std::string& file, paths)
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath + file, LBM))
        {
            std::cout << "failure";
            return false;
        }
    }

    /*
    std::cout << "\nLoading palette from file: /GFX/PALETTE/PAL5.BBM...";
    if ( !CFile::open_file(global::gameDataFilePath + "/GFX/PALETTE/PAL5.BBM", BBM, true) )
    {
        std::cout << "failure";
        return false;
    }
    */

    // EVERY MISSION-FILE SHOULD BE LOADED SEPARATLY IF THE SPECIFIED MISSION GOES ON -- SO THIS IS TEMPORARY
    paths.clear();
    paths += "/DATA/MIS0BOBS.LST", "/DATA/MIS1BOBS.LST", "/DATA/MIS2BOBS.LST", "/DATA/MIS3BOBS.LST", "/DATA/MIS4BOBS.LST",
      "/DATA/MIS5BOBS.LST";
    BOOST_FOREACH(const std::string& file, paths)
    {
        std::cout << "\nLoading file: " << file << "...";
        if(!CFile::open_file(global::gameDataFilePath + file, LST))
        {
            std::cout << "failure";
            return false;
        }
    }

    // create the mainmenu
    callback::mainmenu(INITIALIZING_CALL);

    return true;
}
