#pragma once

#include <Common.hh>

#include "sp/net/mkw_server/race_verification/Config.hh"

#include <game/util/Registry.hh>

namespace MKWServer {

struct __attribute__((packed)) ReplayFileHeader {
    ReplayFileHeader() = default;

    u32 magic;
    u32 crc32;
    u16 version;
    u8 aidCount;
    u8 playerCount;
    Registry::Course course : 8;
    Registry::EngineClass engineClass : 8;
    Registry::GameMode gameMode : 8;
    Registry::BattleType battleType : 8;
    Registry::ModeFlags modeFlags : 8;
};
static_assert(sizeof(ReplayFileHeader) == 0x11);

struct __attribute__((packed)) AidRecord {
    AidRecord() = default;

    u8 aid;
    u16 finishTime;
    u8 localPlayerCount;
    bool disconnected;
    Aid aidRecord;
    u8 idxToAidMapLen;
};
static_assert(sizeof(AidRecord) == 0xa7);

struct __attribute__((packed)) PlayerRecord {
    u8 playerId;
    u8 aid;
    Player player;
    u16 raceFrameCount;
};
static_assert(sizeof(PlayerRecord) == 0x10);

class ReplayFileManager {
public:
    bool open(const wchar_t *path);

    static void CreateInstance();

    static void DestroyInstance();

    static ReplayFileManager *Instance() {
        return s_instance;
    }

private:
    ReplayFileHeader m_header;

    AidRecord m_aidRecords[12];
    PlayerRecord m_playerRecords[12];

    static ReplayFileManager *s_instance;
};

} // namespace MKWServer
