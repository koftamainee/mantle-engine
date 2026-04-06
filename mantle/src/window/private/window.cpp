#include <window/window.h>
#include <GLFW/glfw3.h>

#include <core/assert.h>


namespace mantle {
    Window::~Window() {
        destroy();
    }

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

        m_native_window = glfwCreateWindow(static_cast<int>(properties.size.width),
                                           static_cast<int>(properties.size.height),
                                           properties.title.c_str(), nullptr, nullptr);

        fatal(!m_native_window, "Failed to create GLFW window");
        s_windows_count++;

        glfwSetWindowUserPointer(m_native_window, this);

        spdlog::info("Window created: {} ({}x{})",
                     properties.title.c_str(), properties.size.width, properties.size.height);
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

    void Window::on_update() const {
        check(m_is_initialized);
        glfwPollEvents();
    }

    bool Window::should_close() const {
        check(m_is_initialized);
        return glfwWindowShouldClose(m_native_window);
    }

    uint32_t Window::get_width() const {
        return get_size().width;
    }

    uint32_t Window::get_height() const {
        return get_size().height;
    }

    Window::Properties::Size Window::get_size() const {
        check(m_is_initialized);
        int width = 0;
        int height = 0;
        glfwGetWindowSize(m_native_window, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    Window::Properties::Size Window::get_framebuffer_size() const {
        check(m_is_initialized);
        int width, height;
        glfwGetFramebufferSize(m_native_window, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    GLFWwindow *Window::get_native_window() const {
        check(m_is_initialized);
        return m_native_window;
    }

    double Window::get_time() const {
        check(m_is_initialized);
        return glfwGetTime();
    }

    void Window::set_resize_callback(std::function<void(uint32_t, uint32_t)> callback) {
        m_resize_callback = std::move(callback);
        glfwSetFramebufferSizeCallback(m_native_window, [](GLFWwindow *w, int width, int height) {
            auto *window = static_cast<Window *>(glfwGetWindowUserPointer(w));
            if (window->m_resize_callback) {
                window->m_resize_callback(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            }
        });
    }
}
