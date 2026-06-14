#pragma once

#include <Common.hh>

#include <vendor/magic_enum/magic_enum.hpp>

namespace SP {

// Don't assign values when adding new categories to avoid conflicts and diff noise.
enum class Category {};

enum class Setting {};

template <Setting S>
struct OptionType;

// For each new setting, make a new OptionType for the new setting

template <Setting S>
using Option = typename OptionType<S>::Type;

class SettingEntry {
public:
    constexpr SettingEntry(Category category, const s32 settingNameMessageId,
            const s32 *optionMessageIds, const s32 *optionDescriptionMessageIds, s32 defaultOption,
            s32 selectedOption)
        : m_category(category), m_settingNameMessageId(settingNameMessageId),
          m_optionMessageIds(optionMessageIds),
          m_optionDescriptionMessageIds(optionDescriptionMessageIds),
          m_defaultOption(defaultOption), m_selectedOption(selectedOption) {}

    void setSetting(s32 option) {
        m_selectedOption = option;
    }

    s32 getSetting() const {
        return m_selectedOption;
    }

private:
    const Category m_category;
    const s32 m_settingNameMessageId;
    const s32 *m_optionMessageIds;
    const s32 *m_optionDescriptionMessageIds;
    s32 m_defaultOption;
    s32 m_selectedOption;
};

SettingEntry settings[]{

};

template <Setting S>
void setSetting(Option<S> option) {
    settings[static_cast<size_t>(S)].setSetting(static_cast<s32>(option));
}

template <Setting S>
Option<S> getSetting() {
    return static_cast<Option<S>>(settings[static_cast<size_t>(S)].getSetting());
}

} // namespace SP
