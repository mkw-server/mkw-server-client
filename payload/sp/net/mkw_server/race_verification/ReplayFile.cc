#include "ReplayFile.hh"

extern "C" {
#include <revolution.h>
}

#include <cstdarg>
#include <cstdio>

namespace MKWServer {

static bool fail(char *err, const char *format, ...) {
    if (err) {
        va_list args;
        va_start(args, format);
        vsnprintf(err, 256, format, args);
        va_end(args);
    }
    return false;
}

// Checks that aid/playerId is within 0-11.
static bool isIdValid(u8 aidOrPlayerId) {
    return aidOrPlayerId < 12;
}

ReplayFile *ReplayFile::s_instance = nullptr;

ReplayFile::ReplayFile() {
    clear();
}

ReplayFile::~ReplayFile() {
    clear();
}

void ReplayFile::clear() {
    m_currentOffset = 0;
    m_header = {};

    for (u8 aid = 0; aid < 12; aid++) {
        m_aidRecords[aid] = {};
        m_aidRecords[aid].aid = 0xff;

        auto &recvInfo = m_aidRecvInfo[aid];
        recvInfo.remoteAidCount = 0;
        for (u8 idx = 0; idx < 11; idx++) {
            recvInfo.indexForAid[idx] = 0xff;
            delete[] recvInfo.timeRecvEveryonesPackets[idx];
            recvInfo.timeRecvEveryonesPackets[idx] = nullptr;
        }

        auto &racePackets = m_racePackets[aid];
        delete[] racePackets.timeToIndex;
        delete[] racePackets.offsets;
        delete[] racePackets.data;
        racePackets = RacePackets{};
    }

    for (u8 playerId = 0; playerId < 12; playerId++) {
        m_playerRecords[playerId] = {};
        m_playerRecords[playerId].playerId = 0xff;
        m_playerRecords[playerId].aid = 0xff;
        delete[] m_raceFrames[playerId];
        m_raceFrames[playerId] = nullptr;
    }
}

bool ReplayFile::read(SP::Storage::FileHandle &file, void *dst, u32 size, const char *what,
        char *err) {
    if (size == 0) {
        return true;
    }
    if (!dst) {
        return fail(err, "Allocating %d bytes for %s failed", size, what);
    }
    if (!file.read(dst, size, m_currentOffset)) {
        return fail(err, "Reading %s failed (m_currentOffset 0x%x, size %d)", what, m_currentOffset,
                size);
    }
    m_currentOffset += size;
    return true;
}

bool ReplayFile::open(const wchar_t *path, char *err) {
    clear();

    auto file = SP::Storage::Open(path, "r");
    if (!file) {
        return fail(err, "Opening replay file %ls failed!", path);
    }
    auto &handle = *file;

    if (!read(handle, &m_header, sizeof(m_header), "header", err)) {
        return false;
    }

    if (m_header.version != ReplayFileVersion) {
        return fail(err, "Replay version %d, expected %d", m_header.version, ReplayFileVersion);
    }

    if (m_header.aidCount > 12 || m_header.playerCount > 12) {
        return fail(err, "Bad header counts: aidCount %d, playerCount %d", m_header.aidCount,
                m_header.playerCount);
    }

    // Read in the aid records.
    for (u8 i = 0; i < m_header.aidCount; i++) {
        AidRecord a;
        if (!read(handle, &a, sizeof(a), "AidRecord", err)) {
            return false;
        }
        if (!isIdValid(a.aid)) {
            return fail(err, "Bad AidRecord: aid %d", a.aid);
        }

        m_aidRecords[a.aid] = a;
        auto &recvInfo = m_aidRecvInfo[a.aid];
        if (!read(handle, &recvInfo.remoteAidCount, sizeof(recvInfo.remoteAidCount), "recvInfoLen",
                    err)) {
            return false;
        }
        if (recvInfo.remoteAidCount > 11) {
            return fail(err, "aid: %d, remoteAidCount (%d) > 11", a.aid, recvInfo.remoteAidCount);
        }

        if (!read(handle, recvInfo.indexForAid, recvInfo.remoteAidCount, "indexForAid", err)) {
            return false;
        }

        for (u8 senderIdx = 0; senderIdx < recvInfo.remoteAidCount; senderIdx++) {
            u8 senderAid = recvInfo.indexForAid[senderIdx];
            if (!isIdValid(senderAid)) {
                return fail(err, "Bad indexForAidMap entry %d (aid %d)", senderAid, a.aid);
            }

            u32 n = a.finishTime;
            recvInfo.timeRecvEveryonesPackets[senderIdx] = new u16[n];
            if (!read(handle, recvInfo.timeRecvEveryonesPackets[senderIdx], n * sizeof(u16),
                        "recvTimes", err)) {
                return false;
            }
        }
    }

    // Player records
    for (u8 i = 0; i < m_header.playerCount; i++) {
        Player p;
        if (!read(handle, &p, sizeof(p), "player config", err)) {
            return false;
        }
        u8 playerId = p.playerId;
        if (!isIdValid(playerId)) {
            return fail(err, "Bad playerId %d", playerId);
        }
        u8 aid = p.aid;
        if (!isIdValid(aid) || m_aidRecords[aid].aid == 0xff) {
            return fail(err, " %d has bad aid value %d", playerId, aid);
        }

        m_playerRecords[playerId] = p;
        u16 n = m_aidRecords[aid].finishTime;
        m_raceFrames[playerId] = new RaceFramePlayer[n];
        if (!read(handle, m_raceFrames[playerId], n * sizeof(RaceFramePlayer), "raceFrames", err)) {
            return false;
        }
    }

    // Lastly read in all race packets.
    for (u8 aid = 0; aid < 12; aid++) {
        if (m_aidRecords[aid].aid == 0xff) {
            continue;
        }

        auto &racePacket = m_racePackets[aid];
        if (!read(handle, &racePacket.info, sizeof(RacePacketInfo), "totalSize and racePacketCount",
                    err)) {
            return false;
        }

        u16 frameCount = m_aidRecords[aid].finishTime;
        racePacket.timeToIndex = new u16[frameCount];
        if (!read(handle, racePacket.timeToIndex, frameCount * sizeof(u16),
                    "RacePacket.timeToIndex", err)) {
            return false;
        }

        u16 count = racePacket.info.racePacketCount;
        racePacket.offsets = new u32[count];
        if (!read(handle, racePacket.offsets, count * sizeof(u32), "racePacketOffsets", err)) {
            return false;
        }

        for (u16 i = 0; i < count; i++) {
            u32 offset = racePacket.offsets[i];
            // Since we store offsets and not lengths, we have a special case for the last packet.
            u32 nextOffset = i + 1 < count ? racePacket.offsets[i + 1] : racePacket.info.totalSize;
            if (offset >= nextOffset) {
                return fail(err, "aid %d: packet offset %d >= nextOffset %d. racePacket %d", aid,
                        offset, nextOffset, i);
            }
        }

        racePacket.data = new u8[racePacket.info.totalSize];
        if (!read(handle, racePacket.data, racePacket.info.totalSize, "race packets", err)) {
            return false;
        }
    }

    SP_LOG("Loaded replay %ls (%d bytes)", path, m_currentOffset);
    return true;
}

const u8 *ReplayFile::racePacket(u8 aid, u16 time, u32 &size) const {
    size = 0;

    if (!isIdValid(aid) || m_aidRecords[aid].aid == 0xff) {
        SP_LOG("Passed in aid %d doesn't exist", aid);
        return nullptr;
    }

    u16 aidFinishTime = m_aidRecords[aid].finishTime;
    if (time >= aidFinishTime) {
        SP_LOG("Passed in time (%d) for aid (%d) is larger than their finishTime %d", time, aid,
                aidFinishTime);
        return nullptr;
    }

    auto &packets = m_racePackets[aid];
    u16 i = packets.timeToIndex[time];
    u16 packetCount = packets.info.racePacketCount;
    if (i >= packetCount) {
        SP_LOG("aid %d: mapped index (%d) is larger than # of packets (%d)", aid, i, packetCount);
        return nullptr;
    }

    u32 start = packets.offsets[i];
    u32 end = i + 1 < packetCount ? packets.offsets[i + 1] : packets.info.totalSize;
    size = end - start;
    return packets.data + start;
}

const RaceFramePlayer *ReplayFile::raceFrame(u8 playerId, u16 time) const {
    if (!isIdValid(playerId) || !m_raceFrames[playerId]) {
        return nullptr;
    }

    u8 aid = m_playerRecords[playerId].aid;
    if (!isIdValid(aid) || time >= m_aidRecords[aid].finishTime) {
        return nullptr;
    }

    return &m_raceFrames[playerId][time];
}

u16 ReplayFile::recvSentTime(u8 receiverAid, u8 senderAid, u16 time) const {
    if (!isIdValid(receiverAid) || m_aidRecords[receiverAid].aid == 0xff ||
            time >= m_aidRecords[receiverAid].finishTime) {
        return 0;
    }

    const auto &info = m_aidRecvInfo[receiverAid];
    for (u8 idx = 0; idx < info.remoteAidCount; idx++) {
        if (info.indexForAid[idx] == senderAid && info.timeRecvEveryonesPackets[idx]) {
            return info.timeRecvEveryonesPackets[idx][time];
        }
    }
    return 0;
}

void ReplayFile::CreateInstance() {
    if (s_instance) {
        return;
    }

    s_instance = new ReplayFile;
}

void ReplayFile::DestroyInstance() {
    if (!s_instance) {
        return;
    }

    delete s_instance;
    s_instance = nullptr;
}

} // namespace MKWServer
