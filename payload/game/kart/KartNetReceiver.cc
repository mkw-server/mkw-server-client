#include "KartNetReceiver.hh"

#include "game/kart/KartState.hh"

extern "C" {
#include <revolution.h>
}

namespace Kart {

void KartNetReceiver::unpackRaceData() {
    auto *kartState = m_accessor->state;

    // update zipper timer. reset if player's on a zipper for too long zipper
    if (kartState->overZipper()) {
        if (m_zipperTimer == 0) {
            m_zipperTimer = 210 + 1;
        }

        m_zipperTimer--;

        if (m_zipperTimer == 0) {
            kartState->resetZipper();
        }
    } else {
        m_zipperTimer = 0;
    }

    REPLACED(unpackRaceData)();
}

} // namespace Kart
