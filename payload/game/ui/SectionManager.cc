#include "SectionManager.hh"

#include "game/host_system/SystemManager.hh"

namespace UI {

u8 SectionManager::getLocalPlayerCount() const {
    return m_sectionParams->getLocalPlayerCount();
}

void SectionManager::createSection() {
    // TODO: Document how this gets page extensions to work.
    if (m_firstLoad && System::SystemManager::Instance()->launchType() == 1) {
        m_registeredPadManager.unk8061B5A4();
    }
    auto nextId = m_nextSectionId;
    m_currentAnimDir = m_nextAnimDir;
    m_nextSectionId = UI::SectionId::None;
    m_changeTimer = 0;
    m_nextAnimDir = 0;
    m_transitionFrame = -1;
    m_state = 0;
    bool race = Section::GetSceneId(nextId) == System::SceneId::Race;
    m_registeredPadManager.onCreateSection(!race);
    m_currentSection = new Section;
    m_currentSection->init(nextId);
}

void SectionManager::init() {
    REPLACED(init)();

    // Boots the game into the online VS race section for replay builds.
    // TODO: Multiplayer support.
    // TODO: Set section based off the gamemode in the replay file.
    if (g_isInReplayMode) {
        m_bootSectionId = SectionId::WifiSingleVSGameplay;
        // The game runs OOM when m_nextSectionId isn't set. Not sure why.
        m_nextSectionId = SectionId::WifiSingleVSGameplay;
    }
}

} // namespace UI
