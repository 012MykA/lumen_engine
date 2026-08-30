#pragma once

#include "lumen_engine/core/export.hpp"

#include <string>

namespace lm {

class Event;

class LUMEN_API Layer {
public:
	explicit Layer(std::string debugName = "Unnamed");
	virtual ~Layer() = default;

	virtual void OnAttach() {}
	virtual void OnDetach() {}

	virtual void OnEvent(Event & /*e*/) {}
	virtual void OnUpdate(float /*ts*/) {}
	virtual void OnRender() {}
	virtual void OnUIRender() {}

private:
	std::string m_DebugName;
};

} // namespace lm
