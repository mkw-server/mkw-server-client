#include "MiscPacketHandler.hh"

#include <sp/net/mkw_server/MKWServer.hh>

namespace Net {

void MiscPacketHandler::setAckReady() {
    m_readyAcked = true;
}

void MiscPacketHandler::init() {
    REPLACED(init)();

    m_timeUntilReady = 120;
    m_readyAcked = false;
}

void MiscPacketHandler::updateAsRacer() {
    if (!m_readyAcked) {
        if (m_timeUntilReady > 0) {
            m_timeUntilReady--;
        } else {
            MKWServer::sendReadyPacket();
            // Until we get an ack, try to send again in 5 frames.
            m_timeUntilReady = 5;
        }
    }

    REPLACED(updateAsRacer)();
}

} // namespace Net
