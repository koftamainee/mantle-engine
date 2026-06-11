include(GoogleTest)
include(CTest)

function(mantle_test)
    cmake_parse_arguments(TST "" "NAME" "DEPS" ${ARGN})

    add_executable(${TST_NAME} ${TST_NAME}.cpp)

    target_link_libraries(${TST_NAME} PRIVATE
            GTest::gtest_main
            ${TST_DEPS}
    )

    set_target_properties(${TST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${cfg} CFG_UPPER)
        set_target_properties(${TST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_BINARY_DIR}/tests")
    endforeach()

    gtest_discover_tests(${TST_NAME})
endfunction()

mantle_test(NAME aabb DEPS math)
mantle_test(NAME frustum DEPS math)

mantle_test(NAME arena DEPS core)
mantle_test(NAME virtual_heap DEPS core)
