// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>

void loadTexturesForEditorWorld(const std::string& gameDataPath);

/// Given a clean bitmap index (only bitmaps counted, as expected by MAPPIC_TREE_*),
/// return the actual archive index in the LOADER's MAP00 archive.
/// Returns -1 if not found.
int editorMapCleanToArchiveIdx(int cleanIdx);
