#include "MiscPacketHandler.hh"

#include "game/system/RaceManager.hh"

#include <sp/net/mkw_server/MKWServer.hh>

namespace Net {

void MiscPacketHandler::setAckReady() {
    m_readyAcked = true;
}

void MiscPacketHandler::init() {
    REPLACED(init)();

    m_tryAgainFrames = 0;
    m_readyAcked = false;
}

void MiscPacketHandler::updateReadyTimer() {
    if (m_tryAgainFrames > 0) {
        m_tryAgainFrames--;
    };
    if (m_tryAgainFrames == 0 && System::RaceManager::Instance()->introTimer() > 0xd0) {
        SP_LOG("Sending Ready Packet!");
        MKWServer::sendReadyPacket();
        m_tryAgainFrames = 5;
    }
}

void MiscPacketHandler::updateAsRacer() {
    if (!m_readyAcked) {
        updateReadyTimer();
    }

    REPLACED(updateAsRacer)();
}

void MiscPacketHandler::processRH1Records() {
    REPLACED(processRH1Records)();

    if (m_scheduleDisconnect) {
        SP_LOG("Schedule disconnect was set!");
        m_scheduleDisconnect = false;
    }
}

} // namespace Net

void MiscPacketHandler_setAckReady() {
    // Need to check for nullptr in case if an ack is unexpectedly sent outside of race scene.
    if (auto *miscPacketHandler = Net::MiscPacketHandler::Instance()) {
        miscPacketHandler->setAckReady();
    }
}
