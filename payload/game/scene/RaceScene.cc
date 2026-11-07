#include "RaceScene.hh"

#include "game/net/NetManager.hh"
#include "game/system/RaceConfig.hh"

#include <sp/SaveStateManager.hh>
#include <sp/net/mkw_server/race_verification/RaceFrame.hh>
#include <sp/net/mkw_server/race_verification/RaceManager.hh>
#include <sp/net/mkw_server/race_verification/ReplayFile.hh>

extern "C" {
#include <revolution.h>
}

namespace Scene {

void RaceScene::calcSubsystems() {
    REPLACED(calcSubsystems)();

    if (g_isInReplayMode) {
        checkForReplayDesyncs();
    } else {
        calcRaceManager();
    }
}

void RaceScene::calcRaceManager() {
    auto *raceConfig = System::RaceConfig::Instance();
    auto *sysRaceManager = System::RaceManager::Instance();
    if (!raceConfig->raceScenario().isOnline()) {
        return;
    }

    auto *raceManager = MKWServer::RaceManager::Instance();

    // Frames are only recorded while the race is ongoing.
    if (sysRaceManager->isRaceOngoing()) {
        recordFrame();
    }

    if (sysRaceManager->hasCountdownStarted()) {
        raceManager->updateRaceStarted(sysRaceManager->introTimer() - 1);
    }

    if (sysRaceManager->isRaceFinished()) {
        raceManager->updateRaceFinished(sysRaceManager->countdownTimer() - 1);
    }
}

void RaceScene::recordFrame() {
    auto *sysRaceManager = System::RaceManager::Instance();
    auto *raceManager = MKWServer::RaceManager::Instance();
    auto *netManager = Net::NetManager::Instance();
    auto *kartObjectManager = Kart::KartObjectManager::Instance();

    MKWServer::RaceFramePlayer players[2];
    u8 count = netManager->localPlayerCount();
    for (u8 i = 0; i < count; i++) {
        s8 playerId = netManager->localPlayersPlayerId(i);
        u16 inputs = sysRaceManager->player(playerId)
                             ->padProxy()
                             ->currentRaceInputState()
                             .inputBitfield();
        Vec3 pos = *kartObjectManager->object(playerId)->getVehiclePhysics()->pos();
        players[i] = MKWServer::RaceFramePlayer(inputs, pos);
    }
    raceManager->recordFrame(sysRaceManager->countdownTimer(), players, count);
}

void RaceScene::destroySubsystems() {
    REPLACED(destroySubsystems)();
    SP::SaveStateManager::DestroyInstance();
    MKWServer::RaceManager::DestroyInstance();
}

void RaceScene::createSubsystems() {
    REPLACED(createSubsystems)();
    SP::SaveStateManager::CreateInstance();

    if (g_isInReplayMode) {
        return;
    }

    auto *netManager = Net::NetManager::Instance();
    u8 myAid = netManager->myAid();
    u8 localPlayerCount = netManager->localPlayerCount();
    s8 localPlayerIds[2];
    for (u8 i = 0; i < localPlayerCount; i++) {
        localPlayerIds[i] = netManager->localPlayersPlayerId(i);
    }

    MKWServer::RaceManager::CreateInstance(myAid, localPlayerCount, localPlayerIds);

    // Race has been initialized so we can send mkw-server the config.
    auto raceScenario = System::RaceConfig::Instance()->raceScenario();
    if (!raceScenario.isOnline()) {
        return;
    }

    if (!MKWServer::RaceManager::Instance()->makeAndSendConfig()) {
        SP_LOG("makeAndSendConfig() failed!");
    }
}

void RaceScene::checkForReplayDesyncs() {
    auto *replay = MKWServer::ReplayFile::Instance();
    if (!replay) {
        SP_LOG("ReplayFile is nullptr");
        return;
    }

    u32 time = System::RaceManager::Instance()->countdownTimer();

    // Return early so frames before waiting for the countdown to start aren't compared (not
    // tracked).
    if (time == 0) {
        return;
    }

    auto *frame = replay->raceFrame(g_playerIdToReplay, time);
    auto *pad = System::InputManager::Instance()->playerProxy(g_localPlayerIdToReplay);
    u16 inputs = pad->currentRaceInputState().inputBitfield();
    if (inputs != frame->inputs) {
        // Just log desyncs for now.
        SP_LOG("input mismatch %d: actual 0x%x recorded 0x%x", time, inputs, frame->inputs);
    }

    const auto &recorded = frame->pos;
    auto *actual = Kart::KartObjectManager::Instance()
                           ->object(g_playerIdToReplay)
                           ->getVehiclePhysics()
                           ->pos();
    if (actual->x != recorded[0] || actual->y != recorded[1] || actual->z != recorded[2]) {
        auto *netManager = Net::NetManager::Instance();

        SP_LOG("myAid %d desync %d: actual {%f, %f, %f} - recorded {%f, %f, %f}",
                netManager->myAid(), time, actual->x, actual->y, actual->z, recorded[0],
                recorded[1], recorded[2]);
    }
}

} // namespace Scene
