#include "RaceConfig.hh"

#include "game/net/NetManager.hh"

#include <sp/net/mkw_server/race_verification/ReplayFile.hh>

namespace System {

bool RaceConfig::Scenario::isOnline() const {
    return gameMode >= Registry::GameMode::OnlinePrivateVS &&
            gameMode <= Registry::GameMode::OnlinePrivateBT;
}

bool RaceConfig::Scenario::isOnlineVS() const {
    return gameMode >= Registry::GameMode::OnlinePrivateVS &&
            gameMode <= Registry::GameMode::OnlinePublicVS;
}

bool RaceConfig::Scenario::isOnlineBattle() const {
    return gameMode >= Registry::GameMode::OnlinePrivateBT &&
            gameMode <= Registry::GameMode::OnlinePublicBT;
}

RaceConfig::Scenario &RaceConfig::raceScenario() {
    return m_raceScenario;
}

void RaceConfig::initRace() {
    if (g_isInReplayMode) {
        initReplay();
    }

    REPLACED(initRace)();
}

void RaceConfig::initReplay() {
    auto *replayFile = MKWServer::ReplayFile::Instance();
    if (!replayFile) {
        SP_LOG("replayFile is nullptr!");
        return;
    }

    auto &header = replayFile->header();
    m_menuScenario.playerCount = header.playerCount;
    m_menuScenario.course = header.course;
    m_menuScenario.engineClass = header.engineClass;
    m_menuScenario.gameMode = header.gameMode;
    m_menuScenario.battleType = header.battleType;
    m_menuScenario.modeFlags = header.modeFlags;
    m_menuScenario.seed1 = header.seed1;

    u8 myAid = 0xff;
    for (u8 playerId = 0; playerId < 12; playerId++) {
        auto &playerRecord = replayFile->playerRecord(playerId);
        auto &player = m_menuScenario.players.at(playerId);
        u8 currPlayerId = playerRecord.playerId;
        if (currPlayerId == 0xff) {
            player.type = Registry::PlayerType::None;
            continue;
        }

        if (currPlayerId == g_playerIdToReplay) {
            myAid = playerRecord.aid;
            auto &aidRecord = replayFile->aidRecord(myAid);
            m_menuScenario.localPlayerCount = aidRecord.localPlayerCount;
            m_menuScenario.hudCount = aidRecord.localPlayerCount;
            m_menuScenario.seed2 = aidRecord.aidConfig.seed2;
            // TODO: Pass in localPlayerId.
            auto *inputManager = System::InputManager::Instance();
            inputManager->setLocalPlayerDriftType(playerRecord.driftType);
            player.type = Registry::PlayerType::Local;
        } else {
            player.type = Registry::PlayerType::Online;
        }

        player.character = playerRecord.character;
        player.vehicle = playerRecord.vehicle;
        player.startingSpot = playerRecord.startingSpot;
        player.team = playerRecord.team;
        player.rankIconId = playerRecord.rankIconId;
        player.points = playerRecord.points;
        player.rating = playerRecord.rating;
    }

    if (myAid == 0xff) {
        SP_LOG("Unable to find myAid for playerId %d (shouldn't happen)", g_playerIdToReplay);
        return;
    }

    Net::NetManager::Instance()->initReplayMMInfo(myAid);
}

RaceConfig *RaceConfig::Instance() {
    return s_instance;
}

} // namespace System
