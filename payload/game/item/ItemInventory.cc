#include "ItemInventory.hh"

#include "game/system/RaceConfig.hh"

#include <sp/settings/Settings.hh>

extern "C" {
#include <revolution.h>
}

namespace Item {

void ItemInventory::resetItem() {
    auto isItemWheelEnabled =
            Settings::getSetting<Settings::Setting::ItemWheel>() == Settings::ItemWheel::Enabled;

    auto *raceConfig = System::RaceConfig::Instance();
    if (isItemWheelEnabled &&
            raceConfig->raceScenario().gameMode == System::RaceConfig::GameMode::TimeAttack) {
        return;
    }
    REPLACED(resetItem)();
}

void ItemInventory::resetHeldItem() {
    auto isItemWheelEnabled =
            Settings::getSetting<Settings::Setting::ItemWheel>() == Settings::ItemWheel::Enabled;

    auto *raceConfig = System::RaceConfig::Instance();

    if (isItemWheelEnabled &&
            raceConfig->raceScenario().gameMode == System::RaceConfig::GameMode::TimeAttack) {
        return;
    }
    REPLACED(resetHeldItem)();
}

ItemId ItemInventory::getCurrentItem() const {
    return m_currentItemID;
}

void ItemInventory::setItemWheelPressed(bool pressed) {
    m_itemWheelPressed = pressed;
}

bool ItemInventory::getItemWheelPressed() const {
    return m_itemWheelPressed;
}

} // namespace Item
