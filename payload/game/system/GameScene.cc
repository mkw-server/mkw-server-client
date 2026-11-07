#include "GameScene.hh"

#include <sp/net/mkw_server/race_verification/ReplayFile.hh>

// Included for REPLAY_FILE_PATH. After it creates a replay file, mkw-server updates LatestReplay.hh
// to the newest replay file name to speed up testing.
#include <LatestReplay.hh>

extern "C" {
#include <revolution.h>
}

namespace System {

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

    // TODO: Make configurable.

    char err[256];
    bool openResult = replay->open(REPLAY_FILE_PATH, err);
    if (!openResult) {
        panic(err);
        return;
    }
}

} // namespace System
