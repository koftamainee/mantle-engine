#include "camera/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gtest/gtest.h>

using namespace mantle;

static constexpr float EPSILON = 1e-5f;

TEST(camera_tests, default_orientation) {
    Camera cam;
    EXPECT_NEAR(cam.yaw, 0.0f, EPSILON);
    EXPECT_NEAR(cam.pitch, 0.0f, EPSILON);
    EXPECT_NEAR(cam.front.x, 0.0f, EPSILON);
    EXPECT_NEAR(cam.front.y, 0.0f, EPSILON);
    EXPECT_NEAR(cam.front.z, -1.0f, EPSILON);
    EXPECT_NEAR(cam.right.x, 1.0f, EPSILON);
    EXPECT_NEAR(cam.right.y, 0.0f, EPSILON);
    EXPECT_NEAR(cam.right.z, 0.0f, EPSILON);
}

TEST(camera_tests, rotate_yaw) {
    Camera cam;
    cam.rotate(90.0f, 0.0f);
    EXPECT_NEAR(cam.yaw, 90.0f, EPSILON);
    EXPECT_NEAR(cam.front.x, 1.0f, EPSILON);
    EXPECT_NEAR(cam.front.y, 0.0f, EPSILON);
    EXPECT_NEAR(cam.front.z, 0.0f, EPSILON);
}

TEST(camera_tests, rotate_pitch) {
    Camera cam;
    cam.rotate(0.0f, 45.0f);
    EXPECT_NEAR(cam.pitch, 45.0f, EPSILON);
    float s = std::sin(glm::radians(45.0f));
    float c = std::cos(glm::radians(45.0f));
    EXPECT_NEAR(cam.front.x, 0.0f, EPSILON);
    EXPECT_NEAR(cam.front.y, s, EPSILON);
    EXPECT_NEAR(cam.front.z, -c, EPSILON);
}

TEST(camera_tests, pitch_clamp_positive) {
    Camera cam;
    cam.rotate(0.0f, 200.0f);
    EXPECT_NEAR(cam.pitch, 89.0f, EPSILON);
}

TEST(camera_tests, pitch_clamp_negative) {
    Camera cam;
    cam.rotate(0.0f, -200.0f);
    EXPECT_NEAR(cam.pitch, -89.0f, EPSILON);
}

TEST(camera_tests, rotate_twice_accumulates) {
    Camera cam;
    cam.rotate(30.0f, 10.0f);
    cam.rotate(30.0f, 10.0f);
    EXPECT_NEAR(cam.yaw, 60.0f, EPSILON);
    EXPECT_NEAR(cam.pitch, 20.0f, EPSILON);
}

TEST(camera_tests, front_normalized_after_rotate) {
    Camera cam;
    cam.rotate(45.0f, 30.0f);
    float len = glm::length(cam.front);
    EXPECT_NEAR(len, 1.0f, EPSILON);
}

TEST(camera_tests, right_perpendicular_to_front) {
    Camera cam;
    cam.rotate(45.0f, 30.0f);
    float dot = glm::dot(cam.front, cam.right);
    EXPECT_NEAR(dot, 0.0f, EPSILON);
}

TEST(camera_tests, view_matrix_transform_origin) {
    Camera cam;
    cam.position = {10.0f, 20.0f, 30.0f};
    glm::mat4 v = cam.view();
    glm::vec4 origin_in_view = v * glm::vec4(cam.position, 1.0f);
    EXPECT_NEAR(origin_in_view.x, 0.0f, EPSILON);
    EXPECT_NEAR(origin_in_view.y, 0.0f, EPSILON);
    EXPECT_NEAR(origin_in_view.z, 0.0f, EPSILON);
    EXPECT_NEAR(origin_in_view.w, 1.0f, EPSILON);
}

TEST(camera_tests, view_determinant_orthogonal) {
    Camera cam;
    cam.position = {5.0f, 10.0f, 15.0f};
    glm::mat4 v = cam.view();
    float det = glm::determinant(v);
    EXPECT_NEAR(std::abs(det), 1.0f, 1e-3f);
}

TEST(camera_tests, projection_y_flip) {
    Camera cam;
    cam.aspect = 16.0f / 9.0f;
    glm::mat4 p = cam.projection();
    EXPECT_LT(p[1][1], 0.0f);
}

TEST(camera_tests, projection_x_scale) {
    Camera cam;
    cam.aspect = 16.0f / 9.0f;
    glm::mat4 p = cam.projection();
    EXPECT_GT(p[0][0], 0.0f);
}

TEST(camera_tests, projection_near_far) {
    Camera cam;
    EXPECT_LT(cam.z_near, cam.z_far);
    EXPECT_GT(cam.z_near, 0.0f);
}

TEST(camera_tests, gpu_data_matches_camera) {
    Camera cam;
    cam.position = {1.0f, 2.0f, 3.0f};
    cam.fov = 90.0f;
    cam.aspect = 2.0f;
    cam.rotate(45.0f, 15.0f);

    auto gpu = cam.gpu_data();
    EXPECT_EQ(gpu.position, cam.position);
    EXPECT_EQ(gpu.forward, cam.front);
    EXPECT_EQ(gpu.right, cam.right);
    EXPECT_NEAR(gpu.fov, glm::radians(90.0f), EPSILON);
    EXPECT_NEAR(gpu.aspect, 2.0f, EPSILON);
}

TEST(camera_tests, view_projection_non_zero) {
    Camera cam;
    cam.rotate(30.0f, 10.0f);

    auto gpu = cam.gpu_data();
    EXPECT_EQ(gpu.view_proj, cam.projection() * cam.view());
}
