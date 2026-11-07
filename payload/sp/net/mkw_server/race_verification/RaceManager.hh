#pragma once

#include <Common.hh>

#include "sp/CircularBuffer.hh"
#include "sp/net/mkw_server/race_verification/RaceFrame.hh"

extern "C" {
#include <revolution.h>
}

#include <array>

namespace MKWServer {

// TODO: Not sure what a good size is in practice.
static constexpr size_t FrameBufferSize = 300;
static constexpr u32 TimeResendMs = 2000;

struct TimeSendInfo {
    u16 time = 0;
    bool hasTime = false;
    bool sent = false;
    bool acked = false;
    u64 lastSendTick = 0;
};

class RaceManager {
public:
    bool recordFrame(u16 time, const RaceFramePlayer *players, u8 playerCount);

    void markApplied(u8 aid, u16 time);

    bool removeRaceFrame(u16 time);

    void handleMissingFrames(const u8 *message, s32 len);

    void onMissingFramesAck();

    bool canStartNextRace();

    bool makeAndSendConfig();

    void updateRaceStarted(u16 introTimer);

    void updateRaceFinished(u16 finishTime);

    void onIntroWaitTimeAck();

    void onFinishTimeAck();

    static void CreateInstance(u8 myAid, u8 localPlayerCount, s8 *localPlayersPlayerIds);

    static void DestroyInstance();

    static RaceManager *Instance() {
        return s_instance;
    }

private:
    void resendFrames();
    RaceFrame *getFrame(u16 time);
    s32 findFrame(u16 time);
    RaceFrame makeFrame(u16 time);
    u32 raceFrameSize() const;
    void sendTime(TimeSendInfo &info, u16 time, u16 magic);

    // Race frames sent but waiting to be acked by mkw-server.
    SP::CircularBuffer<RaceFrame, FrameBufferSize> m_raceFrames;

    // Number of frames since the countdown started (RaceManager::m_countdownTimer).
    // Sends mkw-server a frame when updated.
    u32 m_time;

    TimeSendInfo m_introWaitTime;
    TimeSendInfo m_finishTime;

    bool m_missingFramesAcked;

    u8 m_localPlayerCount;
    u8 m_myAid;
    std::array<s8, 2> m_playerIds;

    // Sent timestamp of packets we received. Reset each frame.
    u16 m_sentTimes[12] = {};

    // Mutex used around operations where frames get added, removed, sent, etc.
    OSMutex m_framesMutex;

    static RaceManager *s_instance;
};

} // namespace MKWServer
