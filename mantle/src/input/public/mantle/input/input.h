#pragma once

#include <memory_resource>
#include <string>
#include <string_view>
#include <unordered_map>

#include <glm/glm.hpp>

#include "mantle/core/memory/memory_block.h"
#include "mantle/core/memory/pmr/tlsf_resource.h"
#include "mantle/core/memory/tlsf_allocator.h"
#include "mantle/core/types.h"

namespace mantle {
    class Window;

    enum class InputActionType {
        Digital,
        Analog,
    };

    struct InputAction {
        InputActionType type = InputActionType::Digital;
        f32             deadzone = 0.5f;
        u16             keys[8]{};
        u8              mouse_buttons[8]{};
        u8              controller_buttons[8]{};
        u8              controller_axes[8]{};
        u8              key_count = 0;
        u8              mouse_count = 0;
        u8              ctrl_btn_count = 0;
        u8              ctrl_axis_count = 0;
    };

    struct StringHash {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const noexcept {
            return std::hash<std::string_view>{}(sv);
        }
    };

    class Input final {
      public:
        static void init(MemoryBlock block);
        static void shutdown();
        static Input &get();

        static void   begin_frame(Window &window);

        static bool   is_action_pressed(std::string_view name);
        static bool   is_action_just_pressed(std::string_view name);
        static bool   is_action_just_released(std::string_view name);
        static f32    get_action_strength(std::string_view name);
        static f32    get_axis(std::string_view negative, std::string_view positive);
        static glm::vec2 get_vector(
            std::string_view negative_x, std::string_view positive_x,
            std::string_view negative_y, std::string_view positive_y);

        void add_action(std::string_view name, f32 deadzone = 0.5f,
                        InputActionType type = InputActionType::Digital);
        void bind_key(std::string_view action_name, u16 key);
        void bind_mouse_button(std::string_view action_name, u8 button);
        void bind_controller_button(std::string_view action_name, u8 button);
        void bind_controller_axis(std::string_view action_name, u8 axis);

        void start_vibration(u16 low, u16 high, u32 duration_ms);
        void stop_vibration();

      private:
        struct ActionState {
            f32  value = 0.0f;
            bool pressed = false;
            bool pressed_prev = false;
        };

        Input() = default;

        void update_action_state(std::string_view name, InputAction &action,
                                 const Window &window);

        using ActionMap =
            std::pmr::unordered_map<std::pmr::string, InputAction, StringHash, std::equal_to<>>;
        using StateMap =
            std::pmr::unordered_map<std::pmr::string, ActionState, StringHash, std::equal_to<>>;

        TlsfAllocator m_allocator{};
        TlsfResource  m_resource{};
        ActionMap     m_actions{&m_resource};
        StateMap      m_states{&m_resource};
        Window       *m_window = nullptr;
        bool          m_queried_this_frame = false;

        static Input *s_instance;
    };
} // namespace mantle
