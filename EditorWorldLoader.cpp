#include "EditorWorldLoader.h"
#include "Loader.h"
#include "libsiedler2/Archiv.h"
#include "libsiedler2/ArchivItem.h"
#include "libsiedler2/ArchivItem_Bitmap.h"
#include "libsiedler2/libsiedler2.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "ogl/glAllocator.h"
#include <boost/filesystem.hpp>
#include <vector>

/// Maps "clean" bitmap index (as used by MAPPIC_TREE_*) to actual archive index.
/// Built once after MAP00 is loaded.
static std::vector<unsigned> s_mapCleanIdxToArchive;

/// Given a "clean" bitmap index (where only bitmaps are counted), return the
/// corresponding archive index (where nulls/shadows are skipped).
/// Returns -1 if not found.
int editorMapCleanToArchiveIdx(int cleanIdx)
{
    if(cleanIdx >= 0 && static_cast<unsigned>(cleanIdx) < s_mapCleanIdxToArchive.size())
        return static_cast<int>(s_mapCleanIdxToArchive[cleanIdx]);
    return -1;
}

void loadTexturesForEditorWorld(const std::string& gameDataPath)
{
    static bool initDone = false;
    if(initDone) return;
    initDone = true;

    libsiedler2::setAllocator(new GlAllocator());

    const std::string texFiles[] = {"GFX/TEXTURES/TEX5.LBM", "GFX/TEXTURES/TEX6.LBM", "GFX/TEXTURES/TEX7.LBM"};
    for(const auto& f : texFiles) {
        auto path = boost::filesystem::path(gameDataPath) / f;
        if(boost::filesystem::exists(path))
            LOADER.Load(path, nullptr);
    }

    // Load MAP00.LST into the Loader (key "map00")
    const auto map00Path = boost::filesystem::path(gameDataPath) / "DATA/MAP00.LST";
    if(!boost::filesystem::exists(map00Path))
        return;
    LOADER.Load(map00Path, LOADER.GetPaletteN("pal5", 0));

    // Load MAP_0_Z.LST (greenland full sprites) for animals and other game objects
    const auto map0zPath = boost::filesystem::path(gameDataPath) / "DATA/MAP_0_Z.LST";
    if(boost::filesystem::exists(map0zPath))
        LOADER.Load(map0zPath, LOADER.GetPaletteN("pal5", 0));

    // Build the clean-index → archive-index lookup
    auto& arch = LOADER.GetArchive("map00");
    for(unsigned i = 0; i < arch.size(); i++)
    {
        auto* item = arch.get(i);
        if(!item) continue;
        auto bt = item->getBobType();
        if(bt == libsiedler2::BobType::Bitmap || bt == libsiedler2::BobType::BitmapRLE
           || bt == libsiedler2::BobType::Raw || bt == libsiedler2::BobType::BitmapPlayer)
            s_mapCleanIdxToArchive.push_back(i);
    }
}
