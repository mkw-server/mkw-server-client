#pragma once

#include <Common.hh>

extern "C" {
#include <revolution/os.h>
}

#include <game/util/Registry.hh>

namespace Net {

class GlobeHandler {
public:
    void reset();

    static GlobeHandler *Instance() {
        return s_instance;
    }

private:
    void calc();
    void exportPackets();
    void importPackets();

    bool m_prepared;
    u8 _001[0x004 - 0x001];
    // gets set when importing a players Race Info record, but doesn't get cleared until
    // it gets reset by a dc (from static analysis). Effectively, since it doesn't get cleared,
    // it's effectively an availableAids bitfield. Though it only gets read in the following way
    // m_availableAids != 0, not like a bitfield but checking that at least one aid has sent a
    // Race Info record
    u32 m_hasRecvRaceInfo; // bitfield by aid

    // this also gets set when importing a players Race Info record, but it gets cleared
    // after exporting Race Info to that player. bitfield if not clear
    u32 m_hasRecvRaceInfoFromAid;
    u8 _00c[0x018 - 0x00c];
    OSTime m_time;
    RaceInfoPlayer m_raceInfoPlayer[12];

    static GlobeHandler *s_instance;
};
static_assert(sizeof(GlobeHandler) == 0x260);

} // namespace Net
