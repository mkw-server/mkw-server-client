#include "RegisteredPadManager.hh"

namespace UI {

s32 RegisteredPadManager::findDisconnectedControllers() const {
    // To prevent the disconnected controllers popup from appearing in replays on boot, pretend
    // there's no disconnected controllers in lieu of initializing everything properly.
    // -1 indicates no controllers are disconnected.
    // TODO: This is hacky. Look into alternatives.
    if (g_isInReplayMode) {
        return -1;
    }

    return REPLACED(findDisconnectedControllers)();
}

} // namespace UI
