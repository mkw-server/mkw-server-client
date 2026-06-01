#include <Common.S>

PATCH_REPLACE_START(MiscPacketHandler_updateAsRacer, 0x4c)
    // Original instruction sets RaceManager::m_canCountdownStart to true
    // Nopped since mkw-server now handles this. Ideally, we'd rewrite in C++,
    // but this function is complex and difficult to functionally match.
    nop
PATCH_REPLACE_END(MiscPacketHandler_updateAsRacer, 0x4c)
