#pragma once

#include <Common.hh>

#include "game/kart/KartObjectProxy.hh"

namespace Kart {

class KartNetReceiver : public KartObjectProxy {
private:
    // Anti-zipper lag measures. Credits: MrBean35000vr (author), CLF78 (documentation)
    REPLACE void unpackRaceData();
    void REPLACED(unpackRaceData)();

    u8 _0c[0x62 - 0x0c];
    u16 m_zipperTimer; // added - was padding
    u8 _64[0xb4 - 0x64];
};
static_assert(sizeof(KartNetReceiver) == 0xb4);

} // namespace Kart
