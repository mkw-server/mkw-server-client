#pragma once

#include <Common.hh>

#include "game/ui/ctrl/CtrlRaceBase.hh"

namespace UI {

class CtrlRacePing : public CtrlRaceBase {
public:
    CtrlRacePing();
    ~CtrlRacePing() override;

    void initSelf() override;

    // Updates the control to the ping.
    void calcPingTime();

    void calcSelf() override;

    void load();
};

} // namespace UI
