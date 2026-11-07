#pragma once

#include <Common.hh>

namespace MKWServer {

constexpr u16 FinishTimeMagic = 0x4654;    // "FT"
constexpr u16 IntroWaitTimeMagic = 0x4954; // "IT"

struct TimePacket {
    TimePacket(u16 magic, u16 time) : magic(magic), time(time) {}

    static bool IsAck(const u8 *message, s32 len, u16 expectedMagic) {
        if (!message || len != sizeof(u16)) {
            return false;
        }

        u16 magic = (static_cast<u16>(message[0]) << 8) | message[1];
        return magic == expectedMagic;
    }

    u16 magic;
    u16 time;
};
static_assert(sizeof(TimePacket) == 0x4);

} // namespace MKWServer
