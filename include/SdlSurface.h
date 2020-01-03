#ifndef SdlSurface_h__
#define SdlSurface_h__

#include <SDL.h>
#include <memory>

struct SdlSurfaceDeleter
{
    void operator()(SDL_Surface* p) { SDL_FreeSurface(p); }
};

using SdlSurface = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;
inline auto makeSdlSurface(Uint32 flags, int width, int height, int depth, bool withAlpha = false)
{
    return SdlSurface(SDL_CreateRGBSurface(flags, width, height, depth, 0xFF0000, 0xFF00, 0xFF, withAlpha ? 0xFF000000:0));
}

#endif // SdlSurface_h__
