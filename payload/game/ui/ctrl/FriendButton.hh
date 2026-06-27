// See LICENSE-mkw-sp

#pragma once

#include "game/ui/ctrl/Button.hh"

namespace UI {

class FriendButton : public PushButton {
public:
    FriendButton();
    ~FriendButton() override;

    void load(const char **animInfo, const char *dir, const char *file, const char *variant,
            bool r8, bool pointerOnly);

    void calcSelf() override;

private:
    u8 _254[0x25c - 0x254];
};
static_assert(sizeof(FriendButton) == 0x25c);

} // namespace UI
