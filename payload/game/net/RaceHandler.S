#include <Common.S>

PATCH_REPLACE_START(RaceHandler_updateAsRacer, 0x4c)
    // Original instruction sets RaceManager::m_canCountdownStart to true.
    // Nopped since mkw-server now handles this. Ideally, we'd rewrite in C++,
    // but this function is complex and therefore risky to functionally match.
    nop
PATCH_REPLACE_END(RaceHandler_updateAsRacer, 0x4c)
