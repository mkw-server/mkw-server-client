#include "ReplayFile.hh"

#include "sp/net/mkw_server/race_verification/RaceFrame.hh"
#include "sp/storage/Storage.hh"

extern "C" {
#include <revolution.h>
}

namespace MKWServer {

ReplayFileManager *ReplayFileManager::s_instance = nullptr;

bool ReplayFileManager::open(const wchar_t *path) {
    auto file = SP::Storage::Open(path, "r");
    if (!file) {
        SP_LOG("Opening replay file (%ls) failed!", path);
        return false;
    }

    u32 currOffset = 0;

    bool headerReadResult = file->read(&m_header, sizeof(ReplayFileHeader), 0);
    if (!headerReadResult) {
        SP_LOG("m_handle->read() (header) failed");
        return false;
    }
    currOffset += sizeof(ReplayFileHeader);

    u8 aidCount = m_header.aidCount;

    for (u8 i = 0; i < 12; i++) {
        m_aidRecords[i].aid = 0xff;
        m_playerRecords[i].playerId = 0xff;
        m_playerRecords[i].aid = 0xff;
    }

    // Read in each AidRecord.
    for (u8 aid = 0; aid < aidCount; aid++) {
        AidRecord a;
        bool aidRecordReadResult = file->read(&a, sizeof(AidRecord), currOffset);
        if (!aidRecordReadResult) {
            SP_LOG("Reading AidRecord where aid = %d failed", aid);
            return false;
        }
        currOffset += sizeof(AidRecord);

        m_aidRecords[a.aid] = a;

        u8 idxsLen = a.idxToAidMapLen;

        // idxToAidMap is technically part of the AidRecord but its read in seperately since its
        // size is dynamic.
        u8 idxToAidMap[12];
        bool idxToAidMapReadResult = file->read(&idxToAidMap, idxsLen, currOffset);
        if (!idxToAidMapReadResult) {
            SP_LOG("Reading idxToAidMap failed on aid %d!", aid);
            return false;
        }
        currOffset += idxsLen;

        // Read in the times this aid received each race packet from each aid.
        for (u8 i = 0; i < idxsLen; i++) {
            // Read in the buffer length.
            u16 recvTimesLen;
            bool readRecvTimesLenResult =
                    file->read(&recvTimesLen, sizeof(recvTimesLen), currOffset);
            if (!readRecvTimesLenResult) {
                SP_LOG("Reading readRecvTimesLenResult failed (aid %d, i %d)", aid, i);
                return false;
            }
            currOffset += sizeof(recvTimesLen);

            // Size of the buffer to read is the number of frames times the size per time (short =
            // 2)
            u32 framesBufferLen = recvTimesLen * sizeof(recvTimesLen);
            u16 *recvTimes = new u16[recvTimesLen];

            bool readRecvTimesResult = file->read(recvTimes, framesBufferLen, currOffset);
            if (!readRecvTimesResult) {
                SP_LOG("Reading readRecvTimesResult failed (aid %d, i %d, recvTimesLen %d, "
                       "framesBufferLen %d)",
                        aid, i, recvTimesLen, framesBufferLen);
                return false;
            }
            currOffset += framesBufferLen;

            delete[] recvTimes;
        }
    }

    u8 playerCount = m_header.playerCount;
    for (u8 playerId = 0; playerId < playerCount; playerId++) {
        PlayerRecord p;
        bool playerReadResult = file->read(&p, sizeof(PlayerRecord), currOffset);
        if (!playerReadResult) {
            SP_LOG("Reading playerReadResult failed. playerId %d", playerId);
            return false;
        }
        currOffset += sizeof(PlayerRecord);

        m_playerRecords[playerId] = p;
        u16 raceFrameCount = p.raceFrameCount;
        u32 raceFrameBufferLen = sizeof(RaceFramePlayer) * raceFrameCount;

        RaceFramePlayer *raceFrames = new RaceFramePlayer[raceFrameCount];

        bool playerRaceFramesReadResult = file->read(raceFrames, raceFrameBufferLen, currOffset);
        if (!playerRaceFramesReadResult) {
            SP_LOG("Reading RaceFramePlayer buffer failed for playerId %d", playerId);
        }
        currOffset += raceFrameBufferLen;

        delete[] raceFrames;
    }

    return true;
}

void ReplayFileManager::CreateInstance() {
    if (s_instance) {
        return;
    }

    s_instance = new ReplayFileManager;
}

void ReplayFileManager::DestroyInstance() {
    if (!s_instance) {
        return;
    }

    delete s_instance;
    s_instance = nullptr;
}

} // namespace MKWServer
