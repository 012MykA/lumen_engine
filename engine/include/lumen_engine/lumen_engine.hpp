#pragma once

// --- Core ---
#include "lumen_engine/core/events/event.hpp"
#include "lumen_engine/core/events/event_dispatcher.hpp"
#include "lumen_engine/core/events/keyboard_events.hpp"
#include "lumen_engine/core/events/mouse_events.hpp"
#include "lumen_engine/core/events/window_events.hpp"
#include "lumen_engine/core/key_codes.hpp"
#include "lumen_engine/core/layer.hpp"
#include "lumen_engine/core/layer_stack.hpp"
#include "lumen_engine/core/log.hpp"
#include "lumen_engine/core/mouse_codes.hpp"
#include "lumen_engine/core/window_desc.hpp"

// --- Windowing ---
#include "lumen_engine/windowing/window.hpp"
#include "lumen_engine/windowing/window_context.hpp"
#include "lumen_engine/windowing/window_manager.hpp"

// --- Runtime ---
#include "lumen_engine/runtime/application.hpp"

// --- Vulkan Core ---
#include "lumen_engine/vulkan_core/check_vk.hpp"
#include "lumen_engine/vulkan_core/device.hpp"
#include "lumen_engine/vulkan_core/instance.hpp"
#include "lumen_engine/vulkan_core/physical_device.hpp"
#include "lumen_engine/vulkan_core/vma_allocator.hpp"
#include "lumen_engine/vulkan_core/volk_loader.hpp"

// --- Graphics ---
#include "lumen_engine/gfx/graphics_context.hpp"
