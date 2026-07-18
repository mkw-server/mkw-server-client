#include "MiscPacketHandler.hh"

#include "game/system/RaceManager.hh"

#include <sp/net/mkw_server/MKWServer.hh>

// Time the intro camera rotation will finish by. It actually finishes after 170 frames,
// but 30 frames are added to be safe.
static constexpr u16 INTRO_CAMERA_ROTATION_FINISH_TIME = 200;

static constexpr u8 READY_RETRY_COOLDOWN = 5;

namespace Net {

void MiscPacketHandler::setAckReady() {
    m_readyAcked = true;
}

void MiscPacketHandler::init() {
    REPLACED(init)();

    m_readyRetryCooldown = 0;
    m_readyAcked = false;
}

void MiscPacketHandler::trySendReady() {
    if (m_readyRetryCooldown > 0) {
        m_readyRetryCooldown--;
        return;
    }

    // Send a ready packet when the intro camera has finished rotating.
    if (System::RaceManager::Instance()->introTimer() > INTRO_CAMERA_ROTATION_FINISH_TIME) {
        SP_LOG("Sending Ready Packet!");
        MKWServer::sendReadyPacket();
        m_readyRetryCooldown = READY_RETRY_COOLDOWN;
    }
}

void MiscPacketHandler::updateAsRacer() {
    if (!m_readyAcked) {
        trySendReady();
    }

    REPLACED(updateAsRacer)();
}

} // namespace Net

void MiscPacketHandler_setAckReady() {
    // Need to check for nullptr in case if an ack is unexpectedly sent outside of race scene.
    if (auto *miscPacketHandler = Net::MiscPacketHandler::Instance()) {
        miscPacketHandler->setAckReady();
    } else {
        SP_LOG("MiscPacketHandler_setAckReady() called outside of RaceScene!");
    }
}
