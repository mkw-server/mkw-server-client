#pragma once

#include <Common.hh>

#include "game/net/records/Event.hh"
#include "game/net/records/RH1.hh"
#include "game/net/records/RH2.hh"
#include "game/net/records/RaceData.hh"
#include "game/net/records/Select.hh"

extern "C" {
#include <revolution/os.h>
}

namespace Net {

// MiscPacketHandler runs exclusively in race scene. It handles everything netcode during a race.
class MiscPacketHandler {
private:
    // 0x80653728
    // The main loop. Runs when m_isPrepared is true. This runs in online races
    // but also rival ghost races to update friend status
    void update();

    // 0x80654150
    // Runs only when racing. Synchronizes race start, imports, exports, and processes records
    // specifically as a racer.
    void updateAsRacer();

    // 0x80654d08
    // Exports relevant records as a racer (as opposed to a spectator)
    void exportAsRacer();

    // 0x806554a0
    // Unpacks and processes received RH1 records. Updates m_aidsLoadedIntoRace and time of other
    // players. Also checks if we're out of sync of other players and disconnects us if so.
    void processRecvRH1Records();

    // 0x80654038
    // Returns true when all players have loaded into the race
    bool isEveryoneInRace() const;

    // 0x80654b00
    // Processes other player's lag frames. Skip frames if someone has skipped more frames than us.
    // Skips one frame if framerate is 60 fps and two if 30 fps.
    void processLagFrames();

    // 0x806544a8
    // Gets the passed in player's time since countdown if its not terribly out of sync
    u32 getPlayerTimeSinceCountdown(s32 playerId) const;

    // Runs update() when true
    bool m_isPrepared;

    // Schecules a disconnect. Set if we're out of sync with other players.
    bool m_scheduleDisconnect;

    // Padding
    u8 _002[0x004 - 0x002];

    // Bitfield set by RH1.raceSeed
    u32 m_aidsLoadedIntoRace;

    // Bitfield set if a room or select record was sent
    u32 m_aidLastSentRoomOrSelect;

    // Bitfield that stops non-racers (the dc animation?)
    u32 m_aidShouldStop;

    // Number of frames skipped
    s16 m_localLagFrames;

    // Timer used to syncronize the countdown start with other players, adjusted on the fly.
    // Countdown starts when this is negative.
    s16 m_introCameraCountdown;

    // Outgoing RaceData records. One per local player
    RaceDataRecord m_sendRaceDataRecords[2];

    // Outgoing Event record.
    EventRecord m_sendEventRecords;

    // Padding
    u8 _18c[0x190 - 0x18c];

    // Time since friend status was last retrieved
    OSTime m_TimeSinceFriendStatusUpdated;

    // Set to the countdown time, but set after the countdown ends
    u32 m_aidsTimeSinceCountdown[MAX_PLAYER_COUNT];

    static MiscPacketHandler *s_instance;
};
static_assert(sizeof(MiscPacketHandler) == 0x1c8);

} // namespace Net
