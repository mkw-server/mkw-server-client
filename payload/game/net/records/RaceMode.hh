#pragma once

#include <Common.hh>

namespace Net {

// RaceModeRecord contains data specific to a game mode. Though the data varies from game mode to
// game mode, the data size is consistently 0x28.
struct RaceModeRecord {
    u8 _00[0x28 - 0x00];
};
static_assert(sizeof(RaceModeRecord) == 0x28);

} // namespace Net
