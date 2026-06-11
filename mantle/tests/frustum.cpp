// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/math/frustum.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gtest/gtest.h>

using mantle::AABB;
using mantle::Frustum;

static glm::mat4 make_vp() {
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspectiveRH_ZO(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
    return proj * view;
}

TEST(frustum_tests, visible_aabb_in_front_of_camera) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-1.0f, -1.0f, -10.0f}, {1.0f, 1.0f, -5.0f}};
    EXPECT_TRUE(f.intersects(box));
}

TEST(frustum_tests, aabb_behind_camera_is_culled) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-1.0f, -1.0f, 5.0f}, {1.0f, 1.0f, 10.0f}};
    EXPECT_FALSE(f.intersects(box));
}

TEST(frustum_tests, aabb_beyond_far_plane_is_culled) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-1.0f, -1.0f, -200.0f}, {1.0f, 1.0f, -150.0f}};
    EXPECT_FALSE(f.intersects(box));
}

TEST(frustum_tests, aabb_before_near_plane_is_culled) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-1.0f, -1.0f, -0.05f}, {1.0f, 1.0f, -0.01f}};
    EXPECT_FALSE(f.intersects(box));
}

TEST(frustum_tests, aabb_to_the_left_is_culled) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-100.0f, -1.0f, -10.0f}, {-50.0f, 1.0f, -5.0f}};
    EXPECT_FALSE(f.intersects(box));
}

TEST(frustum_tests, aabb_to_the_right_is_culled) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{50.0f, -1.0f, -10.0f}, {100.0f, 1.0f, -5.0f}};
    EXPECT_FALSE(f.intersects(box));
}

TEST(frustum_tests, aabb_above_is_culled) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-1.0f, 50.0f, -10.0f}, {1.0f, 100.0f, -5.0f}};
    EXPECT_FALSE(f.intersects(box));
}

TEST(frustum_tests, aabb_below_is_culled) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-1.0f, -100.0f, -10.0f}, {1.0f, -50.0f, -5.0f}};
    EXPECT_FALSE(f.intersects(box));
}

TEST(frustum_tests, large_aabb_spanning_camera_is_visible) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-50.0f, -50.0f, -50.0f}, {50.0f, 50.0f, 50.0f}};
    EXPECT_TRUE(f.intersects(box));
}

TEST(frustum_tests, aabb_partially_inside_is_visible) {
    Frustum f;
    f.extract(make_vp());
    AABB box {{-1.0f, -1.0f, -110.0f}, {1.0f, 1.0f, -50.0f}};
    EXPECT_TRUE(f.intersects(box));
}
