// See LICENSE-mkw-sp

#pragma once

namespace UI {

enum class SectionId {
    None = -1,

    WifiSingle = 0x55,
    WifiSingleDC = 0x56,
    WifiSingleFriend = 0x57,

    WifiMulti = 0x5b,
    WifiMultiDC = 0x5c,
    WifiMultiFriend = 0x5d,

    Max = 0x95,
};

} // namespace UI
