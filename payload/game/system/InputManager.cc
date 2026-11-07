#include "InputManager.hh"

#include <sp/net/mkw_server/race_verification/ReplayFile.hh>

#include <bit>

extern "C" {
#include <revolution.h>
}

namespace System {

bool RaceInputState::isInputValid() const {
    if (trick > Trick::Right) {
        SP_LOG("Trick %d is greater than %d (invalid)", trick, Trick::Right);
        return false;
    }

    // Stick values range from 0-14, anything else is invalid.
    if (rawStick.x > 14 || rawStick.y > 14) {
        SP_LOG("rawStick x (%d) || rawStick y (%d) > 14", rawStick.x, rawStick.y);
        return false;
    }

    return true;
}

u16 RaceInputState::inputBitfield() const {
    if (!isInputValid()) {
        return -1;
    }

    u16 inputBitfield = 0;

    if (trick != Trick::Off) {
        // Subtract trick by 1 to account for the offset in Trick and RaceInput values.
        inputBitfield |= 1 << (trick - 1);
    }

    if (accelerate) {
        inputBitfield |= RaceInput::Accel;
    }

    if (drift) {
        inputBitfield |= RaceInput::Drift;
    }

    if (brake) {
        inputBitfield |= RaceInput::Brake;
    }

    if (item) {
        inputBitfield |= RaceInput::Item;
    }

    inputBitfield |= (rawStick.x & 0xf) << 8;
    inputBitfield |= (rawStick.y & 0xf) << 12;

    return inputBitfield;
}

const RaceInputState &PadProxy::currentRaceInputState() const {
    return m_currentRaceInputState;
}

void PadProxy::setInputStateFromReplayFile(u16 time) {
    auto *inputManager = System::InputManager::Instance();
    auto *replay = MKWServer::ReplayFile::Instance();
    if (!inputManager || !replay) {
        return;
    }

    // TODO: Support local-multiplayer.
    if (m_pad != inputManager->playerProxy(g_localPlayerIdToReplay)->pad()) {
        return;
    }

    auto frame = replay->raceFrame(g_playerIdToReplay, time);

    auto inputs = frame->inputs;
    auto &inputState = m_currentRaceInputState;

    u8 trickBits = inputs & 0xf;
    // Add 1 to account for the offset between Trick and RaceState.
    u8 trick = trickBits != 0 ? std::countr_zero(trickBits) + 1 : Trick::Off;
    RaceInputState::SetTrick(inputState, trick);

    inputState.accelerate = inputs & RaceInput::Accel;
    inputState.drift = inputs & RaceInput::Drift;
    inputState.brake = inputs & RaceInput::Brake;
    inputState.item = inputs & RaceInput::Item;

    u8 rawStickX = (inputs >> 8) & 0xf;
    u8 rawStickY = (inputs >> 12) & 0xf;

    RaceInputState::SetStickX(inputState, rawStickX);
    RaceInputState::SetStickY(inputState, rawStickY);
}

Pad *PadProxy::pad() const {
    return m_pad;
}

PlayerPadProxy *InputManager::playerProxy(u32 i) {
    assert(i <= 3);

    return &m_playerProxies[i];
}

void InputManager::setLocalPlayerDriftType(Registry::DriftType driftType) {
    // TODO: Local multiplayer support.
    m_playerProxies[g_localPlayerIdToReplay].setDriftType(driftType);
}

} // namespace System
