#include "GameScene.hh"

#include <sp/net/mkw_server/race_verification/ReplayFile.hh>
#include <sp/storage/Storage.hh>

extern "C" {
#include <revolution.h>
}

static const wchar_t ReplayDir[] = L"/mkw-sp/replays";
static constexpr wchar_t Ext[] = L".rply";
static constexpr size_t ExtLen = std::size(Ext) - 1;

namespace System {

// Finds the newest replay file by the file name (in format YYYY-MM-DD_HH-MM-SS.rply).
static bool FindLatestReplayFile(wchar_t *path, size_t pathLen) {
    auto dir = SP::Storage::OpenDir(ReplayDir);
    if (!dir) {
        return false;
    }

    wchar_t latest[64] = {};
    while (auto info = dir->read()) {
        if (info->type != SP::Storage::NodeType::File) {
            continue;
        }

        size_t len = wcslen(info->name);
        // Check to make sure the file extension is .rply.
        if (len < ExtLen || wcscmp(info->name + len - ExtLen, Ext) != 0) {
            continue;
        }

        if (wcscmp(info->name, latest) > 0) {
            swprintf(latest, std::size(latest), L"%ls", info->name);
        }
    }

    if (!latest[0]) {
        // No file found.
        return false;
    }

    swprintf(path, pathLen, L"%ls/%ls", ReplayDir, latest);
    return true;
}

void GameScene::enter() {
    if (g_isInReplayMode) {
        openReplayFile();
    }

    REPLACED(enter)();
}

void GameScene::openReplayFile() const {
    MKWServer::ReplayFile::CreateInstance();

    auto *replay = MKWServer::ReplayFile::Instance();
    if (!replay) {
        SP_LOG("MKWServer::ReplayFile::Instance() is nullptr!");
        return;
    }

    // Open up newest replay file by default. TODO: Figure out some way to override this.
    wchar_t path[64];
    if (!FindLatestReplayFile(path, std::size(path))) {
        panic("No replay files found in /mkw-sp/replays");
    }

    char err[256];
    if (!replay->open(path, err)) {
        panic("Failed to open replay: %s", err);
    }
}

} // namespace System
