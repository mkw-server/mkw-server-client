#include "CtrlRacePing.hh"

namespace UI {

namespace GroupId {

// These are the numbers going left to right.
enum {
    Int0 = 0,
    Int1 = 1,
    Int2 = 2,
    Fract0 = 3,
    Fract1 = 4,
};

} // namespace GroupId

CtrlRacePing::CtrlRacePing() = default;

CtrlRacePing::~CtrlRacePing() = default;

void CtrlRacePing::initSelf() {
    CtrlRaceBase::initSelf();

    // clang-format off
    const char *panes[] = {
        // 000 00
        "ping_00",
        "ping_01",
        "ping_02",
        "ping_03",
        "ping_04",
        // .
        "coron",
        // PING
        "ping_text"
    };
    // clang-format off

    for (u32 i = 0; i < std::size(panes); i++) {
        setPaneColor(panes[i], true);
    }
}

void CtrlRacePing::calcSelf() {
    process();

    // TODO: update() ping_0X panes with calculated ping.
    // setAnimationInactive() has to be called or the game will crash.
    m_animator.setAnimationInactive(GroupId::Int0, 0, 0);
    m_animator.setAnimationInactive(GroupId::Int1, 0, 0);
    m_animator.setAnimationInactive(GroupId::Int2, 0, 0);
    m_animator.setAnimationInactive(GroupId::Fract0, 0, 0);
    m_animator.setAnimationInactive(GroupId::Fract1, 0, 0);
}

void CtrlRacePing::load() {
    const char *variant = "CtrlRacePing";

    // clang-format off
    const char *groups[] = {
        "eAFInt0", "texture_pattern_0_9_0", nullptr,
        "eAFInt1", "texture_pattern_0_9_1", nullptr,
        "eAFInt2", "texture_pattern_0_9_2", nullptr,
        "eAFFract0", "texture_pattern_0_9_3", nullptr,
        "eAFFract1", "texture_pattern_0_9_4", nullptr,
        nullptr,
    };
    // clang-format on

    LayoutUIControl::load("game_image", "ping_number", variant, groups);
}

} // namespace UI
