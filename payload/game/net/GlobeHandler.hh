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
    // gets set when importing a players RH1 record, but doesn't get cleared until
    // it gets reset by a dc (from static analysis). Effectively, since it doesn't get cleared,
    // it's effectively an availableAids bitfield. Though it only gets read in the following way
    // m_availableAids != 0, not like a bitfield but checking that at least one aid has sent a RH1
    // record
    u32 m_hasRecvRH1; // bitfield by aid

    // this also gets set when importing a players RH1 record, but it gets cleared
    // after exporting rh1 to that player. bitfield if not clear
    u32 m_hasRecvRH1FromAid;
    u8 _00c[0x018 - 0x00c];
    OSTime m_time;
    RH1Player m_RH1Players[12];

    static GlobeHandler *s_instance;
};
static_assert(sizeof(GlobeHandler) == 0x260);

} // namespace Net
