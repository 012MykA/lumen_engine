#pragma once

#include "lumen_engine/rhi/types.hpp"

#include <vector>

namespace lm::rhi {

template <typename T, typename Tag>
class ResourcePool final {
public:
	Handle<Tag> Insert(T value) {
		uint32_t index;
		if (!m_FreeList.empty()) {
			index = m_FreeList.back();
			m_FreeList.pop_back();
		} else {
			index = static_cast<uint32_t>(m_Slots.size());
			m_Slots.emplace_back();
		}
		Slot &slot = m_Slots[index];
		slot.value = value;
		slot.alive = true;
		return Handle<Tag>{ index, slot.generation };
	}

	void Remove(Handle<Tag> handle) {
		if (!IsValid(handle)) {
			return;
		}
		Slot &slot = m_Slots[handle.index];
		slot.alive = false;
		slot.generation++;
		m_FreeList.push_back(handle.index);
	}

	bool IsValid(Handle<Tag> handle) const {
		return handle.index != Handle<Tag>::k_InvalidIndex &&
				handle.index < m_Slots.size() &&
				m_Slots[handle.index].alive &&
				m_Slots[handle.index].generation == handle.generation;
	}

	template <typename Fn>
	void ForEach(Fn &&fn) {
		for (size_t i = 0; i < m_Slots.size(); ++i) {
			if (m_Slots[i].alive) {
				fn(Handle<Tag>{ static_cast<uint32_t>(i), m_Slots[i].generation }, m_Slots[i].value);
			}
		}
	}

	T *Get(Handle<Tag> handle) { return IsValid(handle) ? &m_Slots[handle.index].value : nullptr; }

	const T *Get(Handle<Tag> handle) const { return IsValid(handle) ? &m_Slots[handle.index].value : nullptr; }

private:
	struct Slot {
		T value{};
		uint32_t generation = 0;
		bool alive = false;
	};
	std::vector<Slot> m_Slots;
	std::vector<uint32_t> m_FreeList;
};

} // namespace lm::rhi
