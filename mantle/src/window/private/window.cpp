#include <GLFW/glfw3.h>
#include <window/window.h>

#include <core/assert.h>


namespace mantle {
    Window::~Window() { destroy(); }

    void Window::init(const Properties &properties) {
        check(!m_is_initialized);

        if (s_windows_count == 0) {
            glfwSetErrorCallback([](int error, const char *desc) {
                spdlog::error("GLFW error {}: {}", error, desc);
            });
            fatal(!glfwInit(), "Failed to initialize GLFW");
            spdlog::info("GLFW initialized");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_native_window =
            glfwCreateWindow(static_cast<int>(properties.size.width),
                             static_cast<int>(properties.size.height),
                             properties.title.c_str(), nullptr, nullptr);

        fatal(!m_native_window, "Failed to create GLFW window");
        glfwSetInputMode(m_native_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        s_windows_count++;

        glfwSetWindowUserPointer(m_native_window, this);

        spdlog::info("Window created: {} ({}x{})", properties.title.c_str(),
                     properties.size.width, properties.size.height);
        m_is_initialized = true;
    }

    void Window::destroy() {
        if (m_is_initialized) {
            std::string title = glfwGetWindowTitle(m_native_window);
            glfwDestroyWindow(m_native_window);
            m_native_window = nullptr;
            s_windows_count--;
            spdlog::info("{} window destroyed", title.c_str());
            if (s_windows_count == 0) {
                glfwTerminate();
                spdlog::info("GLFW terminated");
            }
            m_is_initialized = false;
        }
    }

    void Window::update() const {
        check(m_is_initialized);
        glfwPollEvents();
    }

    bool Window::should_close() const {
        check(m_is_initialized);
        return glfwWindowShouldClose(m_native_window);
    }

    bool Window::is_key_pressed(Key key) const {
        check(m_is_initialized);
        std::array glfw_keys = {GLFW_KEY_W,     GLFW_KEY_A,
                                GLFW_KEY_S,     GLFW_KEY_D,
                                GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT};
        return glfwGetKey(m_native_window,
                          glfw_keys[std::to_underlying(key)]) == GLFW_PRESS;
    }

    bool Window::is_mouse_button_pressed(MouseButton mouse_button) const {
        std::array glfw_mb = {GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_MIDDLE,
                              GLFW_MOUSE_BUTTON_RIGHT};
        return glfwGetMouseButton(m_native_window,
                                  glfw_mb[std::to_underlying(mouse_button)]) ==
            GLFW_PRESS;
    }

    Window::MousePosition Window::get_mouse_position() const {
        check(m_is_initialized);
        f64 x;
        f64 y;
        glfwGetCursorPos(m_native_window, &x, &y);
        return {static_cast<f32>(x), static_cast<f32>(y)};
    }

    u32 Window::get_width() const { return get_size().width; }

    u32 Window::get_height() const { return get_size().height; }

    Window::Properties::Size Window::get_size() const {
        check(m_is_initialized);
        int width = 0;
        int height = 0;
        glfwGetWindowSize(m_native_window, &width, &height);
        return {static_cast<u32>(width), static_cast<u32>(height)};
    }

    Window::Properties::Size Window::get_framebuffer_size() const {
        check(m_is_initialized);
        int width, height;
        glfwGetFramebufferSize(m_native_window, &width, &height);
        return {static_cast<u32>(width), static_cast<u32>(height)};
    }

    GLFWwindow *Window::get_native_window() const {
        check(m_is_initialized);
        return m_native_window;
    }

    f64 Window::get_time() const {
        check(m_is_initialized);
        return glfwGetTime();
    }

    void Window::set_resize_callback(std::function<void(u32, u32)> callback) {
        m_resize_callback = std::move(callback);
        glfwSetFramebufferSizeCallback(
            m_native_window, [](GLFWwindow *w, int width, int height) {
                auto *window =
                    static_cast<Window *>(glfwGetWindowUserPointer(w));
                if (window->m_resize_callback) {
                    window->m_resize_callback(static_cast<u32>(width),
                                              static_cast<u32>(height));
                }
            });
    }
} // namespace mantle
