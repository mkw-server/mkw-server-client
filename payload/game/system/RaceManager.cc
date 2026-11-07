#include "RaceManager.hh"

#include "game/item/ItemDirector.hh"
#include "game/net/NetManager.hh"
#include "game/system/RaceConfig.hh"

#include <sp/SaveStateManager.hh>
#include <sp/net/mkw_server/race_verification/RaceManager.hh>
#include <sp/net/mkw_server/race_verification/ReplayFile.hh>

extern "C" {
#include <revolution.h>
#include <revolution/kpad.h>
}

namespace System {

void RaceManager::Player::calc() {
    REPLACED(calc)();

    // Check if user wants to load state, rotate items, or use pow.
    auto *raceConfig = System::RaceConfig::Instance();
    auto gameMode = raceConfig->raceScenario().gameMode;
    // These settings are exclusive to TTs. We check for playerId to prevent unintended behavior
    // from ghosts.
    if (gameMode == Registry::GameMode::TimeAttack && m_playerId == 0) {
        auto *playerPadProxy = m_padProxy;
        auto buttons = playerPadProxy->currentRaceInputState().rawButtons;
        auto controller = playerPadProxy->pad()->getControllerId();

        bool shouldUpdateItem = false;
        bool isLoadButtonPressed = false;
        switch (controller) {
        case Registry::Controller::WiiWheel:
            shouldUpdateItem = (buttons & PAD_BUTTON_START) == PAD_BUTTON_START;
            isLoadButtonPressed = (buttons & WPAD_CL_BUTTON_HOME) == WPAD_CL_BUTTON_HOME;
            break;
        case Registry::Controller::WiiRemoteAndNunchuck:
            shouldUpdateItem = (buttons & WPAD_BUTTON_DOWN) == WPAD_BUTTON_DOWN;
            isLoadButtonPressed = (buttons & WPAD_CL_BUTTON_DOWN) == WPAD_CL_BUTTON_DOWN;
            break;
        case Registry::Controller::Classic:
            shouldUpdateItem = (buttons & KPAD_CL_TRIGGER_ZL) == KPAD_CL_TRIGGER_ZL;
            isLoadButtonPressed = (buttons & KPAD_CL_TRIGGER_ZR) == KPAD_CL_TRIGGER_ZR;

            break;
        case Registry::Controller::GameCube:
            shouldUpdateItem = (buttons & PAD_BUTTON_Y) == PAD_BUTTON_Y;
            isLoadButtonPressed = (buttons & PAD_TRIGGER_Z) == PAD_TRIGGER_Z;
            break;
        case Registry::Controller::None:
            return;
        }

        if (auto *itemDirector = Item::ItemDirector::Instance()) {
            itemDirector->processInput(shouldUpdateItem);
        }

        if (auto *saveStateManager = SP::SaveStateManager::Instance()) {
            saveStateManager->processInput(isLoadButtonPressed);
        }
    }
}

void RaceManager::update() {
    REPLACED(update)();

    if (g_isInReplayMode) {
        updateReplay();
        return;
    }

    Net::NetManager::Instance()->applyPendingPackets();
}

void RaceManager::updateReplay() {
    auto *replay = MKWServer::ReplayFile::Instance();
    if (!replay) {
        SP_LOG("!replay (shouldn't happen)");
        return;
    }

    auto *netManager = Net::NetManager::Instance();
    u8 myAid = netManager->myAid();

    u16 introWaitTime = replay->aidRecord(myAid).introWaitTime;
    // While waiting for the race to start, which takes a variable amount of time, the
    // character bobs up and down. Funnily enough, this changes the y position by a tiny amount and
    // causes the replay to desync. Replays need to wait the same amount of time as the actual race.
    // TODO: Look into if the intro wait time can be modulo-ed and still sync to save time waiting.
    if (m_introTimer == introWaitTime) {
        m_canCountdownStart = true;
    }

    auto *playerPad = System::InputManager::Instance()->playerProxy(g_localPlayerIdToReplay);
    playerPad->setInputStateFromReplayFile(m_countdownTimer);

    if (!isRaceOngoing()) {
        return;
    }

    for (u8 aid = 0; aid < 12; aid++) {
        if (aid == myAid) {
            continue;
        }

        u16 sentTime = replay->recvSentTime(myAid, aid, m_countdownTimer);
        if (sentTime == 0) {
            continue;
        }

        u32 size;
        const u8 *packet = replay->racePacket(aid, sentTime, size);
        if (!packet || size == 0 || size > MAX_RACE_PACKET_SIZE) {
            SP_LOG("frame %d: aid %d sent time %d has no usable race packet", m_countdownTimer, aid,
                    sentTime);
            continue;
        }

        u8 buffer[MAX_RACE_PACKET_SIZE];
        memcpy(buffer, packet, size);
        netManager->applyRacePacket(aid, buffer, size);
    }
}

} // namespace System
