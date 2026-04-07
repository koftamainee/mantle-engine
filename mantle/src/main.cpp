#include <iostream>
#include "camera/public/camera/camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "mesh/mesh.h"
#include "renderer/renderer.h"
#include "spdlog/spdlog.h"
#include "window/window.h"
#include "world/chunk_mesher.h"
#include "world/world.h"


namespace mantle {
    i32 mantle_main() {
        Window window;
        Window::Properties prop = {
            .title = "Mantle",
            .size = {.width = 2560, .height = 1600},
        };
        window.init(prop);

        Renderer renderer;
        renderer.init(window);

        Camera camera;
        camera.aspect = 2560.0f / 1600.0f;

        window.set_resize_callback([&](u32 w, u32 h) {
            renderer.resize(w, h);
            camera.aspect = static_cast<f32>(w) / static_cast<f32>(h);
        });

        GPUResourceManager &resources = renderer.get_resource_manager();

        World world;
        world.init();

        std::vector<glm::mat4> models;

        f32 voxel_size = 0.1;

        for (i32 x = -5; x < 5; x++) {
            for (i32 y = -5; y < 5; y++) {
                for (i32 z = -5; z < 5; z++) {
                    world.generate_chunk({x, y, z});
                    glm::vec3 world_pos = glm::vec3(x, y, z) * static_cast<f32>(Chunk::s_chunk_size) * voxel_size;
                    glm::mat4 model = glm::translate(glm::mat4{1.0f}, world_pos);
                    model = glm::scale(model, glm::vec3(voxel_size));
                    models.emplace_back(model);
                }
            }
        }

        std::vector<MeshHandle> meshes;
        world.for_each_chunk([&](Chunk &chunk) {
            Mesh mesh = ChunkMesher::build(chunk);
            if (mesh.vertices.empty()) return;

            glm::vec3 world_pos = glm::vec3(chunk.position()) * static_cast<f32>(Chunk::s_chunk_size) * voxel_size;
            glm::mat4 model = glm::translate(glm::mat4{1.0f}, world_pos);
            model = glm::scale(model, glm::vec3(voxel_size));

            models.emplace_back(model);
            meshes.push_back(resources.upload_mesh(mesh.vertices, mesh.indices));
        });

        f32 last_time = 0.0f;

        glm::vec3 target = {0.0f, 0.0f, 0.0f};

        f32 distance = 50.0f;
        f32 yaw = 0.0f;
        f32 pitch = glm::radians(20.0f); // фиксированный наклон

        while (!window.should_close()) {
            auto current_time = static_cast<f32>(window.get_time());
            f32 delta_time = current_time - last_time;
            last_time = current_time;

            window.on_update();

            f32 speed = 0.5f; // радианы в секунду

            yaw += speed * delta_time;

            glm::vec3 direction;
            direction.x = cos(yaw) * cos(pitch);
            direction.y = sin(pitch);
            direction.z = sin(yaw) * cos(pitch);

            camera.position = target - direction * distance;
            camera.front = glm::normalize(target - camera.position);

            renderer.set_camera(camera.view(), camera.projection());

            Renderer::Result result = renderer.begin_frame();
            if (result == Renderer::Result::FrameNeedsResize) {
                auto [width, height] = window.get_framebuffer_size();
                renderer.resize(width, height);
                continue;
            }

            renderer.begin_pass();

            for (i32 i = 0; i < meshes.size(); i++) {
                result = renderer.draw_mesh(meshes[i], models[i]);
                if (result == Renderer::Result::InvalidMeshHandle) {
                    spdlog::critical(
                        "Invalid mesh handle. Should be unreachable");
                    break;
                }
            }
            renderer.end_pass();

            result = renderer.end_frame();
            if (result == Renderer::Result::FrameNeedsResize) {
                auto [width, height] = window.get_framebuffer_size();
                renderer.resize(width, height);
            }
        }

        renderer.destroy();
        window.destroy();
        return 0;
    }
} // namespace mantle

int main() {
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    return mantle::mantle_main();
}
