// Copyright (c) 2026 Mantle. All rights reserved.

#include "window/window.h"

#include <array>
#include <utility>

#include <SDL3/SDL.h>
#include <core/assert.h>

#include "sdl_allocator.h"

namespace mantle {
    void Window::init(const Properties &properties, MemoryBlock mem) {
        MANTLE_CHECK(!m_is_initialized);
        m_logger = spdlog::get("window").get();

        // damn it is singleton
        SDLAllocator &alloc = SDLAllocator::create(mem);

        bool set =
            SDL_SetMemoryFunctions(alloc.malloc(), alloc.calloc(), alloc.realloc(), alloc.free());

        if (!set) {
            m_logger->warn("Failed to set SDL memory functions. SDL will be using system malloc");
        }

        MANTLE_FATAL(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD), "Failed to initialize SDL");
        m_logger->info("SDL initialized");

        u32 window_w = properties.size.width;
        u32 window_h = properties.size.height;

        if (window_w == 0 || window_h == 0) {
            SDL_DisplayID display = SDL_GetPrimaryDisplay();
            if (const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display)) {
                window_w = static_cast<u32>(mode->w);
                window_h = static_cast<u32>(mode->h);
                m_logger->info("Deriving window display resolution: {}x{}", window_w, window_h);
            } else {
                window_w = 1920;
                window_h = 1080;
                m_logger->warn("Failed to query display mode, falling back to {}x{}", window_w,
                               window_h);
            }
        }

        m_native_window = SDL_CreateWindow(properties.title.data(), static_cast<int>(window_w),
                                           static_cast<int>(window_h),
                                           SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                               (properties.fullscreen ? SDL_WINDOW_FULLSCREEN : 0));

        MANTLE_FATAL(!m_native_window, "Failed to create SDL window");
        m_fullscreen = properties.fullscreen;

        SDL_SetWindowRelativeMouseMode(m_native_window, true);

        {
            SDL_DisplayID display = SDL_GetPrimaryDisplay();
            if (const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display)) {
                m_refresh_rate = static_cast<f32>(mode->refresh_rate);
                m_logger->info("Display: {} Hz", mode->refresh_rate);
            }
        }

        m_logger->info("Window created: {} ({}x{})", properties.title.data(), window_w, window_h);
        m_is_initialized = true;
    }

    void Window::destroy() {
        if (m_is_initialized) {
            if (m_controller) {
                SDL_CloseGamepad(m_controller);
                m_controller = nullptr;
            }
            SDL_DestroyWindow(m_native_window);
            m_native_window = nullptr;
            m_logger->info("SDL window destroyed");
            SDL_Quit();
            m_logger->info("SDL quit");

            m_is_initialized = false;
        }
    }

    void Window::update() {
        MANTLE_CHECK(m_is_initialized);

        m_pressed_keys_prev = m_pressed_keys;
        m_pressed_mb_prev = m_pressed_mb;
        m_pressed_cb_prev = m_pressed_cb;

        m_frame_event_count = 0;
        m_mouse_delta = {};
        m_mouse_wheel = {};

        m_keyboard_active = false;
        m_mouse_active = false;
        m_controller_active = false;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    m_should_close = true;
                } break;

                case SDL_EVENT_WINDOW_RESIZED: {
                    if (m_resize_callback) {
                        m_resize_callback(event.window.data1, event.window.data2);
                    }
                } break;

                case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
                    if (m_resize_callback) {
                        int w, h;
                        SDL_GetWindowSizeInPixels(m_native_window, &w, &h);
                        m_resize_callback(static_cast<u32>(w), static_cast<u32>(h));
                    }
                } break;

                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP: {
                    m_keyboard_active = true;
                    bool pressed = event.type == SDL_EVENT_KEY_DOWN;
                    if (event.key.scancode < static_cast<SDL_Scancode>(Key::Count)) {
                        Key key = static_cast<Key>(event.key.scancode);
                        m_pressed_keys.set(std::to_underlying(key), pressed);
                        if (m_frame_event_count < MAX_EVENTS_PER_FRAME) {
                            auto &re = m_frame_events[m_frame_event_count++];
                            re.type = RawEvent::Type::Key;
                            re.key = {key, pressed};
                        }
                    }
                } break;

                case SDL_EVENT_MOUSE_MOTION: {
                    m_mouse_active = true;
                    m_mouse_delta.x += event.motion.xrel;
                    m_mouse_delta.y -= event.motion.yrel;
                    if (m_frame_event_count < MAX_EVENTS_PER_FRAME) {
                        auto &re = m_frame_events[m_frame_event_count++];
                        re.type = RawEvent::Type::MouseMotion;
                        re.mouse_motion = {event.motion.xrel, -event.motion.yrel};
                    }
                } break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP: {
                    m_mouse_active = true;
                    bool pressed = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
                    if (event.button.button >= 1 && event.button.button < MOUSE_BUTTON_COUNT) {
                        MouseButton mb = static_cast<MouseButton>(event.button.button);
                        m_pressed_mb[std::to_underlying(mb)] = pressed;
                        if (m_frame_event_count < MAX_EVENTS_PER_FRAME) {
                            auto &re = m_frame_events[m_frame_event_count++];
                            re.type = RawEvent::Type::MouseButton;
                            re.mouse_button = {mb, pressed};
                        }
                    }
                } break;

                case SDL_EVENT_MOUSE_WHEEL: {
                    m_mouse_active = true;
                    m_mouse_wheel.x += event.wheel.x;
                    m_mouse_wheel.y += event.wheel.y;
                    if (m_frame_event_count < MAX_EVENTS_PER_FRAME) {
                        auto &re = m_frame_events[m_frame_event_count++];
                        re.type = RawEvent::Type::MouseWheel;
                        re.mouse_wheel = {event.wheel.x, event.wheel.y};
                    }
                } break;

                case SDL_EVENT_GAMEPAD_ADDED: {
                    if (!m_controller) {
                        m_controller = SDL_OpenGamepad(event.gdevice.which);
                        if (m_controller) {
                            m_logger->info("Controller connected: {}",
                                           SDL_GetGamepadName(m_controller));
                        }
                    }
                } break;

                case SDL_EVENT_GAMEPAD_REMOVED: {
                    if (m_controller && event.gdevice.which == SDL_GetGamepadID(m_controller)) {
                        m_logger->info("Controller disconnected: {}",
                                       SDL_GetGamepadName(m_controller));
                        SDL_CloseGamepad(m_controller);
                        m_controller = nullptr;
                    }
                } break;

                case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                    m_controller_active = true;
                    bool pressed = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
                    if (event.gbutton.button < CONTROLLER_BUTTON_COUNT) {
                        ControllerButton cb = static_cast<ControllerButton>(event.gbutton.button);
                        m_pressed_cb[std::to_underlying(cb)] = pressed;
                        if (m_frame_event_count < MAX_EVENTS_PER_FRAME) {
                            auto &re = m_frame_events[m_frame_event_count++];
                            re.type = RawEvent::Type::ControllerButton;
                            re.controller_button = {cb, pressed};
                        }
                    }
                } break;

                case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                    m_controller_active = true;
                    if (event.gaxis.axis < CONTROLLER_AXIS_COUNT) {
                        ControllerAxis ca = static_cast<ControllerAxis>(event.gaxis.axis);
                        f32            val = static_cast<f32>(event.gaxis.value) / 32767.0f;
                        if (val < -1.0f) {
                            val = -1.0f;
                        }
                        m_controller_axes[std::to_underlying(ca)] = val;
                        if (m_frame_event_count < MAX_EVENTS_PER_FRAME) {
                            auto &re = m_frame_events[m_frame_event_count++];
                            re.type = RawEvent::Type::ControllerAxis;
                            re.controller_axis = {ca, val};
                        }
                    }
                } break;

                default: {
                } break;
            }
        }
    }

    bool Window::should_close() const {
        MANTLE_CHECK(m_is_initialized);
        return m_should_close;
    }

    bool Window::is_key_pressed(Key key) const {
        MANTLE_CHECK(m_is_initialized);
        return m_pressed_keys[std::to_underlying(key)];
    }

    bool Window::is_key_just_pressed(Key key) const {
        MANTLE_CHECK(m_is_initialized);
        usize i = std::to_underlying(key);
        return m_pressed_keys[i] && !m_pressed_keys_prev[i];
    }

    bool Window::is_key_just_released(Key key) const {
        MANTLE_CHECK(m_is_initialized);
        usize i = std::to_underlying(key);
        return !m_pressed_keys[i] && m_pressed_keys_prev[i];
    }

    bool Window::is_mouse_button_pressed(MouseButton button) const {
        MANTLE_CHECK(m_is_initialized);
        return m_pressed_mb[std::to_underlying(button)];
    }

    bool Window::is_mouse_button_just_pressed(MouseButton button) const {
        MANTLE_CHECK(m_is_initialized);
        usize i = std::to_underlying(button);
        return m_pressed_mb[i] && !m_pressed_mb_prev[i];
    }

    bool Window::is_mouse_button_just_released(MouseButton button) const {
        MANTLE_CHECK(m_is_initialized);
        usize i = std::to_underlying(button);
        return !m_pressed_mb[i] && m_pressed_mb_prev[i];
    }

    Window::MousePosition Window::get_mouse_position() const {
        MANTLE_CHECK(m_is_initialized);
        float x, y;
        SDL_GetMouseState(&x, &y);
        return {x, y};
    }

    bool Window::is_keyboard_active() const {
        MANTLE_CHECK(m_is_initialized);
        return m_keyboard_active;
    }

    bool Window::is_mouse_active() const {
        MANTLE_CHECK(m_is_initialized);
        return m_mouse_active;
    }

    bool Window::is_controller_active() const {
        MANTLE_CHECK(m_is_initialized);
        return m_controller_active;
    }

    bool Window::is_controller_connected() const {
        MANTLE_CHECK(m_is_initialized);
        return m_controller != nullptr;
    }

    bool Window::is_controller_button_pressed(ControllerButton button) const {
        MANTLE_CHECK(m_is_initialized);
        if (!m_controller) {
            return false;
        }
        return m_pressed_cb[std::to_underlying(button)];
    }

    f32 Window::get_controller_axis(ControllerAxis axis) const {
        MANTLE_CHECK(m_is_initialized);
        if (!m_controller) {
            return 0.0f;
        }
        return m_controller_axes[std::to_underlying(axis)];
    }

    void Window::set_controller_rumble(u16 low_frequency, u16 high_frequency, u32 duration_ms) {
        MANTLE_CHECK(m_is_initialized);
        if (!m_controller) {
            return;
        }
        if (!SDL_RumbleGamepad(m_controller, low_frequency, high_frequency, duration_ms)) {
            m_logger->warn("set_controller_rumble failed");
        }
    }

    void Window::stop_controller_rumble() {
        MANTLE_CHECK(m_is_initialized);
        if (!m_controller) {
            return;
        }
        SDL_RumbleGamepad(m_controller, 0, 0, 0);
    }

    u32 Window::get_width() const { return get_size().width; }

    u32 Window::get_height() const { return get_size().height; }

    Window::Properties::Size Window::get_size() const {
        MANTLE_CHECK(m_is_initialized);
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(m_native_window, &width, &height);
        return {static_cast<u32>(width), static_cast<u32>(height)};
    }

    Window::Properties::Size Window::get_framebuffer_size() const {
        MANTLE_CHECK(m_is_initialized);
        int width, height;
        SDL_GetWindowSizeInPixels(m_native_window, &width, &height);
        return {static_cast<u32>(width), static_cast<u32>(height)};
    }

    SDL_Window *Window::get_native_window() const {
        MANTLE_CHECK(m_is_initialized);
        return m_native_window;
    }

    bool Window::is_fullscreen() const {
        MANTLE_CHECK(m_is_initialized);
        return m_fullscreen;
    }

    f32 Window::get_refresh_rate() const {
        MANTLE_CHECK(m_is_initialized);
        return m_refresh_rate;
    }

    u64 Window::get_time_ns() const {
        MANTLE_CHECK(m_is_initialized);
        return SDL_GetTicksNS();
    }

    u64 Window::get_time_ms() const { return get_time_ns() / 1'000'000; }

    std::span<const Window::RawEvent> Window::get_frame_events() const {
        return {m_frame_events.data(), m_frame_event_count};
    }

    Window::MouseDelta Window::get_mouse_delta() const { return m_mouse_delta; }

    Window::MouseDelta Window::get_mouse_wheel() const { return m_mouse_wheel; }

    void Window::set_resize_callback(std::function<void(u32, u32)> callback) {
        m_resize_callback = std::move(callback);
    }
} // namespace mantle
