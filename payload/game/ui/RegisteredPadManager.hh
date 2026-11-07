// See LICENSE-mkw-sp

#pragma once

#include <Common.hh>

namespace UI {

class RegisteredPadManager {
public:
    void onCreateSection(bool r4);
    void unk8061B5A4();

    // 0x8061b9bc
    // This function is called by UI code to display the disconnected controller popup or not.
    REPLACE s32 findDisconnectedControllers() const;
    s32 REPLACED(findDisconnectedControllers)() const;

private:
    u8 _00[0x5c - 0x00];
};
static_assert(sizeof(RegisteredPadManager) == 0x5c);

} // namespace UI
