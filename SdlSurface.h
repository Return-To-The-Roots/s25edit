#ifndef SdlSurface_h__
#define SdlSurface_h__

#include <SDL.h>
#include <memory>

struct SdlSurfaceDeleter
{
    void operator()(SDL_Surface* p) { SDL_FreeSurface(p); }
};

using SdlSurface = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;
inline auto makeSdlSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask = 0, Uint32 Gmask = 0, Uint32 Bmask = 0,
                           Uint32 Amask = 0)
{
    return SdlSurface(SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask));
}

#endif // SdlSurface_h__
