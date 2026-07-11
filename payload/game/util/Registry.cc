#include "Registry.hh"

#include <sp/settings/Settings.hh>

namespace Registry {

bool isUnlockEverythingEnabled() {
    return Settings::getSetting<Settings::Setting::UnlockEverything>() ==
            Settings::UnlockEverything::Enabled;
}

bool isCharacterUnlocked(Character character, bool r4) {
    if (isUnlockEverythingEnabled()) {
        return true;
    }
    return REPLACED(isCharacterUnlocked)(character, r4);
}

bool isVehicleUnlocked(Vehicle vehicle, bool r4) {
    if (isUnlockEverythingEnabled()) {
        return true;
    }
    return REPLACED(isVehicleUnlocked)(vehicle, r4);
}

bool isCupUnlocked(Cup cup, bool r4, bool r5) {
    if (isUnlockEverythingEnabled()) {
        return true;
    }
    return REPLACED(isCupUnlocked)(cup, r4, r5);
}

} // namespace Registry
