#include "CGame.h"

bool CGame::Init()
{
    std::cout << "Return to the Roots Mapeditor\n";


    std::cout << "\nInitializing SDL...";
    if ( SDL_Init(SDL_INIT_EVERYTHING ) < 0)
    {
        std::cout << "failure";
        return false;
    }

    SDL_EnableKeyRepeat(100,100);
    SDL_ShowCursor(SDL_DISABLE);

    std::cout << "\nCreate Window...";
    Surf_Display = SDL_SetVideoMode(MenuResolutionX, MenuResolutionY, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
    if ( Surf_Display == NULL )
    {
        std::cout << "failure";
        return false;
    }

    SDL_WM_SetCaption("Return to the Roots Mapeditor",0);

    /*NOTE: its important to load a palette at first,
     *      otherwise all images will be black (in exception of the LBM-Files, they have their own palette).
     *      if its necessary to load pictures from a
     *      specified file with a special palette, so
     *      load this palette from a file and set
     *      CFile::palActual = CFile::palArray - 1 (--> last loaden palette)
     *      and after loading the images set
     *      CFile::palActual = CFile::palArray (--> first palette)
     */

    //load some pictures (after all the splash-screens)
    //at first /GFX/PICS/SETUP997.LBM, cause this is the S2-loading picture
    std::cout << "\nLoading file: /GFX/PICS/SETUP997.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP997.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }
/*
#ifndef _VIEWERMODE
    //now load the 'powered by sdl'-picture to blit this together to the display
    std::cout << "\nLoading file: sdl_powered.bmp...";
    SDL_Surface *poweredSDL = NULL;
    if ( (poweredSDL = SDL_LoadBMP("./sdl_powered.bmp")) == NULL)
    {
        std::cout << "failure";
        return false;
    }
    //make white the transparent color in the 'powered by sdl'-picture
    SDL_SetColorKey(poweredSDL, SDL_SRCCOLORKEY, SDL_MapRGB(poweredSDL->format, 255, 255, 255));
    //blit the pictures immediately together and show it while loading
    CSurface::Draw(global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, poweredSDL, 240, 400);
    //now free the no longer needed poweredSDL surface
    SDL_FreeSurface(poweredSDL);
#endif
*/
    //std::cout << "\nShow loading screen...";
    showLoadScreen = true;
    CSurface::Draw(Surf_Display, global::bmpArray[SPLASHSCREEN_LOADING_S2SCREEN].surface, 0, 0);
    SDL_Flip(Surf_Display);

    //continue loading pictures
    std::cout << "\nLoading file: /GFX/PICS/SETUP000.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP000.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP010.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP010.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP011.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP011.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP012.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP012.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP013.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP013.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP014.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP014.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP015.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP015.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP666.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP666.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP667.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP667.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP801.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP801.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP802.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP802.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP803.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP803.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP804.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP804.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP805.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP805.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP806.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP806.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP810.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP810.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP811.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP811.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP895.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP895.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP896.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP896.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP897.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP897.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP898.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP898.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP899.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP899.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/SETUP990.LBM...";
    if ( CFile::open_file("./GFX/PICS/SETUP990.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/WORLD.LBM...";
    if ( CFile::open_file("./GFX/PICS/WORLD.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /GFX/PICS/WORLDMSK.LBM...";
    if ( CFile::open_file("./GFX/PICS/WORLDMSK.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }

    //load gouraud data
    std::cout << "\nLoading file: /DATA/TEXTURES/GOU5.DAT...";
    if ( CFile::open_file("./DATA/TEXTURES/GOU5.DAT", GOU) == false )
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /DATA/TEXTURES/GOU6.DAT...";
    if ( CFile::open_file("./DATA/TEXTURES/GOU6.DAT", GOU) == false )
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /DATA/TEXTURES/GOU7.DAT...";
    if ( CFile::open_file("./DATA/TEXTURES/GOU7.DAT", GOU) == false )
    {
        std::cout << "failure";
        return false;
    }
    /*for (int i = 0; i < 3; i++)
    {
        puts("\n--------------------------------------------------------------------------------------------\n");
        for (int j = 0; j < 256; j++)
        {
            puts("");
            for (int k = 0; k < 256; k++)
            {
                printf ("%d ", gouData[i][j][k]);
            }
        }
    }*/

#ifdef _EDITORMODE
    //load only the palette at this time from editres.idx
    std::cout << "\nLoading palette from file: /DATA/EDITRES.IDX...";
    if ( CFile::open_file("./DATA/EDITRES.IDX", IDX, true) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << "\nLoading file: /DATA/EDITRES.IDX...";
    if ( CFile::open_file("./DATA/EDITRES.IDX", IDX) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set back palette
    CFile::set_palActual(CFile::get_palArray());
    //load only the palette at this time from editio.idx
    std::cout << "\nLoading palette from file: /DATA/IO/EDITIO.IDX...";
    if ( CFile::open_file("./DATA/IO/EDITIO.IDX", IDX, true) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << "\nLoading file: /DATA/IO/EDITIO.IDX...";
    if ( CFile::open_file("./DATA/IO/EDITIO.IDX", IDX) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set back palette
    CFile::set_palActual(CFile::get_palArray());
    std::cout << "\nLoading file: /DATA/EDITBOB.LST...";
    if ( CFile::open_file("./DATA/EDITBOB.LST", LST) == false )
    {
        std::cout << "failure";
        return false;
    }
#else
    //load only the palette at this time from resource.idx
    std::cout << "\nLoading palette from file: /DATA/RESOURCE.IDX...";
    if ( CFile::open_file("./DATA/RESOURCE.IDX", IDX, true) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << "\nLoading file: /DATA/RESOURCE.IDX...";
    if ( CFile::open_file("./DATA/RESOURCE.IDX", IDX) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set back palette
    CFile::set_palActual(CFile::get_palArray());
    //load only the palette at this time from io.idx
    std::cout << "\nLoading palette from file: /DATA/IO/IO.IDX...";
    if ( CFile::open_file("./DATA/IO/IO.IDX", IDX, true) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << "\nLoading file: /DATA/IO/IO.IDX...";
    if ( CFile::open_file("./DATA/IO/IO.IDX", IDX) == false )
    {
        std::cout << "failure";
        return false;
    }
    //set back palette
    CFile::set_palActual(CFile::get_palArray());
#endif

    //texture tilesets
    std::cout << "\nLoading file: /GFX/TEXTURES/TEX5.LBM...";
    if ( CFile::open_file("./GFX/TEXTURES/TEX5.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /GFX/TEXTURES/TEX6.LBM...";
    if ( CFile::open_file("./GFX/TEXTURES/TEX6.LBM", LBM) == false )
    {
        std::cout << "failure";
        return false;
    }
    std::cout << "\nLoading file: /GFX/TEXTURES/TEX7.LBM...";
    if ( CFile::open_file("./GFX/TEXTURES/TEX7.LBM", LBM) == false )
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


//EVERY MISSION-FILE SHOULD BE LOADED SEPARATLY IF THE SPECIFIED MISSION GOES ON -- SO THIS IS TEMPORARY
    std::cout << "\nLoading file: /DATA/MIS0BOBS.LST...";
    if ( CFile::open_file("./DATA/MIS0BOBS.LST", LST) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS1BOBS.LST...";
    if ( CFile::open_file("./DATA/MIS1BOBS.LST", LST) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS2BOBS.LST...";
    if ( CFile::open_file("./DATA/MIS2BOBS.LST", LST) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS3BOBS.LST...";
    if ( CFile::open_file("./DATA/MIS3BOBS.LST", LST) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS4BOBS.LST...";
    if ( CFile::open_file("./DATA/MIS4BOBS.LST", LST) == false )
    {
        std::cout << "failure";
        return false;
    }

    std::cout << "\nLoading file: /DATA/MIS5BOBS.LST...";
    if ( CFile::open_file("./DATA/MIS5BOBS.LST", LST) == false )
    {
        std::cout << "failure";
        return false;
    }

#ifdef _VIEWERMODE
    //load the MAP0x.LST's for all pictures --> in normal mode this is done on map load

    //load only the palette at this time from MAP00.LST
    std::cout << "\nLoading palette from file: /DATA/MAP00.LST...";
    if ( CFile::open_file("./DATA/MAP00.LST", LST, true) == false )
    {
        std::cout << "failure";
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << "\nLoading file: /DATA/MAP00.LST...";
    if ( CFile::open_file("./DATA/MAP00.LST", LST) == false )
    {
        std::cout << "failure";
    }
    //set back palette
    CFile::set_palActual(CFile::get_palArray());

    //load only the palette at this time from MAP01.LST
    std::cout << "\nLoading palette from file: /DATA/MAP01.LST...";
    if ( CFile::open_file("./DATA/MAP01.LST", LST, true) == false )
    {
        std::cout << "failure";
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << "\nLoading file: /DATA/MAP01.LST...";
    if ( CFile::open_file("./DATA/MAP01.LST", LST) == false )
    {
        std::cout << "failure";
    }
    //set back palette
    CFile::set_palActual(CFile::get_palArray());

    //load only the palette at this time from MAP02.LST
    std::cout << "\nLoading palette from file: /DATA/MAP02.LST...";
    if ( CFile::open_file("./DATA/MAP02.LST", LST, true) == false )
    {
        std::cout << "failure";
    }
    //set the right palette
    CFile::set_palActual(CFile::get_palArray()-1);
    std::cout << "\nLoading file: /DATA/MAP02.LST...";
    if ( CFile::open_file("./DATA/MAP02.LST", LST) == false )
    {
        std::cout << "failure";
    }
    //set back palette
    CFile::set_palActual(CFile::get_palArray());
#endif

#ifndef _VIEWERMODE
    //create the mainmenu
    callback::mainmenu(INITIALIZING_CALL);
#endif

#ifdef _VIEWERMODE
    CSurface::Draw(Surf_Display, global::bmpArray[index].surface, 0, 0);
#endif

	return true;
}
