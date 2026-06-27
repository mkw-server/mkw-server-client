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

} // namespace UI
