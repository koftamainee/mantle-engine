include(GoogleTest)
include(CTest)

function(mantle_test)
    cmake_parse_arguments(TST "" "NAME" "DEPS" ${ARGN})

    add_executable(${TST_NAME} ${TST_NAME}.cpp)

    target_link_libraries(${TST_NAME} PRIVATE
            GTest::gtest_main
            ${TST_DEPS}
    )

    gtest_discover_tests(${TST_NAME})
endfunction()

mantle_test(NAME aabb DEPS math)
mantle_test(NAME frustum DEPS math)