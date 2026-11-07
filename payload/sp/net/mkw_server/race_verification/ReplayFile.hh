#pragma once

#include <Common.hh>

#include "sp/net/mkw_server/race_verification/Config.hh"
#include "sp/net/mkw_server/race_verification/RaceFrame.hh"
#include "sp/storage/Storage.hh"

#include <game/util/Registry.hh>

static constexpr u16 ReplayFileVersion = 0;

namespace MKWServer {

struct __attribute__((packed)) ReplayFileHeader {
    ReplayFileHeader() = default;

    // File magic is always "RPLY"
    u32 magic;

    // Checksum.
    u32 crc32;

    // Version of the replay file.
    u16 version;

    // Number of aids. Number of aids is equal to the # of non-guest players.
    u8 aidCount;

    // Number of players (includes guests).
    u8 playerCount;

    // Course being replayed.
    Registry::Course course : 8;

    // Engine class being replayed (100cc, 150cc).
    Registry::EngineClass engineClass : 8;

    // Gamemode of the replay (public/private vs/battle)
    Registry::GameMode gameMode : 8;

    // Coin runners or balloon battle if the game mode is a battle.
    Registry::BattleType battleType : 8;

    // Mode flags for the replay.
    Registry::ModeFlags modeFlags : 8;

    // Random seed shared across the entire room.
    // TODO: Might not be needed. I think its not used outside of the menu scene.
    u32 seed1;
};
static_assert(sizeof(ReplayFileHeader) == 0x15);

struct __attribute__((packed)) AidRecord {
    AidRecord() = default;

    // Aid for this record (id from 0-11)
    u8 aid;

    // See comment in System::RaceManager::update() for more info.
    u16 introWaitTime;

    // Either an aid's last frame before results appear or last (sent) frame before disconnecting.
    // Used as the number of inputs/packets received for a given aid.
    u16 finishTime;

    // Aid's number of local players.
    u8 localPlayerCount;

    // Indicates if an aid disconnected or not. Unused at the moment.
    bool disconnected;

    // Aid specifc information.
    Aid aidConfig;
};
static_assert(sizeof(AidRecord) == 0xac);

// Information pertaining what an aid received from other aids and when. Data is variable length.
struct AidRecvInfo {
    // Number of aids this aid sent/received packets from.
    // TODO: remoteAidCount is kind of stupid since it's header.aidCount - 1. Can be removed.
    u8 remoteAidCount;

    // Data for each aid is stored sequentially but aids can be out of order.
    // This mapping allows us to sequentially look up the aid at a given index belongs to.
    u8 indexForAid[11];

    // Maps the time this aid received packets from all other aids to the sent time of the sender.
    u16 *timeRecvEveryonesPackets[11] = {};
};

struct __attribute__((packed)) RacePacketInfo {
    u32 totalSize;

    // Number of race packets sent by a given aid.
    u16 racePacketCount;
};
static_assert(sizeof(RacePacketInfo) == 0x6);

struct RacePackets {
    RacePacketInfo info = {};

    // Maps the time a race packet was sent to an index into offsets. 0xffff if no packet.
    u16 *timeToIndex = nullptr;

    // Offset in data of each packet.
    u32 *offsets = nullptr;

    // All of this aid's race packets back to back. info.totalSize bytes.
    u8 *data = nullptr;
};

class ReplayFile {
public:
    ReplayFile();
    ~ReplayFile();

    bool open(const wchar_t *path, char *err);

    void clear();

    const ReplayFileHeader &header() const {
        return m_header;
    }

    const auto &aids() const {
        return m_aidRecords;
    }

    const auto &players() const {
        return m_playerRecords;
    }

    const AidRecord &aidRecord(u8 aid) const {
        return m_aidRecords[aid];
    }

    const Player &playerRecord(u8 playerId) const {
        assert(playerId < 12);
        return m_playerRecords[playerId];
    }

    const u8 *racePacket(u8 aid, u16 time, u32 &size) const;

    u16 recvSentTime(u8 receiverAid, u8 senderAid, u16 time) const;

    const RaceFramePlayer *raceFrame(u8 playerId, u16 time) const;

    static void CreateInstance();

    static void DestroyInstance();

    static ReplayFile *Instance() {
        return s_instance;
    }

private:
    bool read(SP::Storage::FileHandle &file, void *dst, u32 size, const char *what, char *err);

    u32 m_currentOffset;
    ReplayFileHeader m_header;
    AidRecord m_aidRecords[12];
    Player m_playerRecords[12];
    AidRecvInfo m_aidRecvInfo[12];
    RaceFramePlayer *m_raceFrames[12] = {};
    RacePackets m_racePackets[12];

    static ReplayFile *s_instance;
};

} // namespace MKWServer
