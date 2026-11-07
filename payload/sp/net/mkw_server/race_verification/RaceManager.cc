#include "RaceManager.hh"

#include "sp/net/mkw_server/MKWServer.hh"
#include "sp/net/mkw_server/race_verification/Config.hh"
#include "sp/net/mkw_server/race_verification/MissingFrames.hh"
#include "sp/net/mkw_server/race_verification/TimePacket.hh"

extern "C" {
#include <revolution.h>
}

#include <ranges>
#include <string.h>

namespace MKWServer {

RaceManager *RaceManager::s_instance = nullptr;

bool RaceManager::recordFrame(u16 time, const RaceFramePlayer *players, u8 playerCount) {
    OSLockMutex(&m_framesMutex);

    resendFrames();

    if (findFrame(time) >= 0 || !m_raceFrames.push_back(RaceFrame(time))) {
        OSUnlockMutex(&m_framesMutex);
        return false;
    }

    auto *frame = m_raceFrames.back();
    memcpy(frame->recvPacketTimestamps, m_sentTimes, sizeof(m_sentTimes));
    memset(m_sentTimes, 0, sizeof(m_sentTimes));
    for (u8 i = 0; i < playerCount; i++) {
        frame->players[i] = players[i];
    }

    if (!MKWServer::sendRaceFrame(frame, raceFrameSize())) {
        SP_LOG("Failed to send frame %d", time);
    }

    OSUnlockMutex(&m_framesMutex);
    return true;
}

void RaceManager::markApplied(u8 aid, u16 time) {
    if (aid >= 12) {
        return;
    }

    m_sentTimes[aid] = time;
}

bool RaceManager::removeRaceFrame(u16 time) {
    OSLockMutex(&m_framesMutex);
    s32 idx = findFrame(time);

    bool removed = idx >= 0;
    if (removed) {
        m_raceFrames.remove(idx);
    }
    OSUnlockMutex(&m_framesMutex);
    return removed;
}

void RaceManager::handleMissingFrames(const u8 *message, s32 len) {
    if (!MissingFramesRequest::IsWellFormed(message, len)) {
        SP_LOG("Malformed missing frames request (len %d)", len);
        return;
    }

    u16 missingFrameCount = MissingFramesRequest::Count(message);

    OSLockMutex(&m_framesMutex);
    for (u32 i = 0; i < missingFrameCount; i++) {
        u16 time = MissingFramesRequest::TimeAt(message, i);

        s32 idx = findFrame(time);
        if (idx < 0) {
            // Uh oh! Can't find it! TODO: Inform server.
            continue;
        }

        if (!MKWServer::sendRaceFrame(m_raceFrames[idx], raceFrameSize())) {
            SP_LOG("Failed to resend missing frame %d", time);
            continue;
        }
    }
    OSUnlockMutex(&m_framesMutex);
}

void RaceManager::onMissingFramesAck() {
    if (!m_missingFramesAcked) {
        SP_LOG("missing frames acked");
    }

    m_missingFramesAcked = true;
}

bool RaceManager::canStartNextRace() {
    // For now, mkw-server just needs to send an ack for the missing race frames to transition
    // to the next race. But more requirements to transition may be added.
    return m_missingFramesAcked;
}

bool RaceManager::makeAndSendConfig() {
    auto &raceScenario = System::RaceConfig::Instance()->raceScenario();
    MKWServer::Race race(raceScenario);

    // outgoingRecord() will create a User record incase it has been cleared.
    auto *userRecord = Net::UserHandler::Instance()->outgoingRecord();
    if (!userRecord) {
        SP_LOG("Failed to make config because my User record is nullptr!");
        return false;
    }

    MKWServer::Aid aid(userRecord, raceScenario.seed2);
    MKWServer::ConfigPacket configPacket(race, aid);

    for (auto [i, playerId] : std::views::enumerate(m_playerIds)) {
        if (playerId < 0) {
            continue;
        }
        auto &player = raceScenario.players[playerId];
        configPacket.players[i] = MKWServer::Player(playerId, m_myAid, i, player);
    }

    return MKWServer::sendConfig(configPacket, ConfigPacket::CalcPacketSize(m_localPlayerCount));
}

void RaceManager::updateRaceStarted(u16 introTimer) {
    sendTime(m_introWaitTime, introTimer, IntroWaitTimeMagic);
}

void RaceManager::updateRaceFinished(u16 finishTime) {
    sendTime(m_finishTime, finishTime, FinishTimeMagic);
}

void RaceManager::onIntroWaitTimeAck() {
    m_introWaitTime.acked = true;
}

void RaceManager::onFinishTimeAck() {
    m_finishTime.acked = true;
}

void RaceManager::CreateInstance(u8 myAid, u8 localPlayerCount, s8 *localPlayersPlayerIds) {
    if (s_instance) {
        return;
    }

    s_instance = new RaceManager;
    OSInitMutex(&s_instance->m_framesMutex);

    s_instance->m_myAid = myAid;
    s_instance->m_localPlayerCount = localPlayerCount;
    for (u8 i = 0; i < localPlayerCount; i++) {
        s_instance->m_playerIds[i] = localPlayersPlayerIds[i];
    }
}

void RaceManager::DestroyInstance() {
    if (!s_instance) {
        return;
    }

    delete s_instance;
    s_instance = nullptr;
}

void RaceManager::resendFrames() {
    OSLockMutex(&m_framesMutex);
    for (u32 i = 0; i < m_raceFrames.count(); i++) {
        auto raceFrame = m_raceFrames[i];

        u16 timeSinceSend = m_time - raceFrame->time;
        // Start resending after 2 seconds, every half second.
        // TODO: Figure out what numbers/logic is good here.
        if (timeSinceSend > 30 && timeSinceSend % 10 == 0) {
            if (!MKWServer::sendRaceFrame(raceFrame, raceFrameSize())) {
                SP_LOG("RaceFrame %d failed to resend!", raceFrame->time);
            }
        }
    }
    OSUnlockMutex(&m_framesMutex);
}

s32 RaceManager::findFrame(u16 time) {
    for (u32 i = 0; i < m_raceFrames.count(); i++) {
        auto raceFrame = m_raceFrames[i];
        if (raceFrame->time == time) {
            return i;
        }
    }
    return -1;
}

RaceFrame *RaceManager::getFrame(u16 time) {
    s32 idx = findFrame(time);
    if (idx < 0) {
        return nullptr;
    }
    return m_raceFrames[idx];
}

RaceFrame RaceManager::makeFrame(u16 time) {
    RaceFrame frame(time);
    return frame;
}

u32 RaceManager::raceFrameSize() const {
    u32 playerRecordSize = m_localPlayerCount * sizeof(RaceFramePlayer);
    return sizeof(RaceFrame) - (2 * sizeof(RaceFramePlayer)) + playerRecordSize;
}

void RaceManager::sendTime(TimeSendInfo &state, u16 time, u16 magic) {
    if (state.acked) {
        return;
    }

    if (!state.hasTime) {
        state.time = time;
        state.hasTime = true;
        SP_LOG("magic 0x%x set to %d", magic, state.time);
    }

    u64 now = OSGetTick();
    if (state.sent && OSTicksToMilliseconds(now - state.lastSendTick) < TimeResendMs) {
        return;
    }

    if (state.sent) {
        SP_LOG("no 0x%x ack yet, resending %d", magic, state.time);
    }

    state.lastSendTick = now;
    state.sent = MKWServer::sendTimePacket(magic, state.time);
}

} // namespace MKWServer
