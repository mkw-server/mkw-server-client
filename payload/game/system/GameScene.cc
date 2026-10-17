#include "GameScene.hh"

#include <sp/net/mkw_server/race_verification/ReplayFile.hh>

extern "C" {
#include <revolution.h>
}

namespace System {

void GameScene::enter() {
#ifdef REPLAY
    openReplayFile();
#endif
    REPLACED(enter)();
}

void GameScene::openReplayFile() const {
    MKWServer::ReplayFileManager::CreateInstance();

    auto *replayManager = MKWServer::ReplayFileManager::Instance();
    if (!replayManager) {
        SP_LOG("MKWServer::ReplayFileManager::Instance() is nullptr!");
        return;
    }

    // TODO: Make configurable.
    bool openResult = replayManager->open(L"/mkw-sp/replays/2026-10-12_18-41-39.rply");
    if (!openResult) {
        SP_LOG("!openResult");
        return;
    }
}

} // namespace System
