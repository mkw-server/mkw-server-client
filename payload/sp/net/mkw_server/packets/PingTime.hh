#pragma once

#include <Common.hh>

namespace MKWServer {

struct PingTime {
    u8 ms[3];
    u8 ns[2];
} __attribute__((packed));
static_assert(sizeof(PingTime) == 5);

struct PingTimePacket {
    u8 magic[2];
    PingTime pingTime;
} __attribute__((packed));
static_assert(sizeof(PingTimePacket) == 7);

} // namespace MKWServer
