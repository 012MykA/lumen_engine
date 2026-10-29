#include "lumen_engine/core/layer.hpp"

namespace lm {

Layer::Layer(std::string debugName) : m_DebugName{ std::move(debugName) } {}

} // namespace lm
