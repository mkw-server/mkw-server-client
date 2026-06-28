#include "RacePage.hh"

#include "game/ui/ctrl/CtrlRacePing.hh"

#include <sp/net/mkw_server/MKWServer.hh>
#include <sp/settings/Settings.hh>

namespace UI {

namespace Control {

enum {
    Time = 1 << 1,
    Count = 1 << 2,
    Map = 1 << 3,
    RankNum = 1 << 4,
    Lap = 1 << 5,
    ItemWindow = 1 << 6,
    Point = 1 << 7,
    GhostTimeDiff = 1 << 8,
    Live = 1 << 9,
    Score = 1 << 10,
    ItemBalloon = 1 << 11,
    GhostMessage = 1 << 12,
};

} // namespace Control

bool RacePage::pingEnabled() const {
    return Settings::getSetting<Settings::Setting::Ping>() == Settings::Ping::Enabled &&
            MKWServer::hasMKWServerAddress();
}

// Vanilla passes the return into initChildren().
u8 RacePage::getControlCount(u32 controls) const {
    u8 count = REPLACED(getControlCount)(controls);

    if (pingEnabled()) {
        count++;
    }

    return count;
}

// This gets called right after initChildren(). We can hook it to load our controls.
void RacePage::initControls(u32 controls) {
    REPLACED(initControls)(controls);

    if (pingEnabled()) {
        auto *control = new CtrlRacePing;
        insertChild(getControlCount(controls) - 1, control, 0);
        control->load();
    }
}

} // namespace UI
