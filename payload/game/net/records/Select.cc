#include "Select.hh"

namespace Net {

void SelectHandler::decideEngineClass() {
    // Force engine class to be 150cc (for now).
    m_sendRecord.engineClass = Registry::EngineClass::CC150;
}

} // namespace Net
