#pragma once

#include <Common.hh>

#include "game/ui/page/Page.hh"

namespace UI {

class RandomMatchingPage : public Page {
public:
    enum class JoinType {
        AnyoneWW = 0x0,
        AnyoneRegional = 0x1,
        FriendWW = 0x2,
        FriendRegional = 0x3,
    };

    void setJoinType(JoinType joinType) {
        m_joinType = joinType;
    }

    void setBattle(bool isBattle) {
        m_isBattle = isBattle;
    }

private:
    u8 _0044[0x1cf4 - 0x0044];
    JoinType m_joinType;
    bool m_isBattle;
    u8 _1cf9[0x1d00 - 0x1cf9];
    PageId m_replacement;
};
static_assert(sizeof(RandomMatchingPage) == 0x1d04);

} // namespace UI
