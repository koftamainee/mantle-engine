option(MANTLE_ENABLE_FORMAT "Enable mantle-format target" ON)
if(MANTLE_ENABLE_FORMAT)
    find_program(CLANG_FORMAT_BIN clang-format)
    find_program(PYTHON_BIN python3)

    set(MANTLE_FORMAT_DEPS)

    if(CLANG_FORMAT_BIN AND PYTHON_BIN)
        message(STATUS "clang-format: ${CLANG_FORMAT_BIN}")
        message(STATUS "run-clang-format.py: ${CMAKE_SOURCE_DIR}/scripts/run-clang-format.py")
        add_custom_target(mantle-format-clang
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format: formatting sources..."
            COMMAND ${PYTHON_BIN} scripts/run-clang-format.py "${CMAKE_SOURCE_DIR}/mantle/src" "${CMAKE_SOURCE_DIR}/mantle/tests"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "clang-format: formatting sources"
        )
        list(APPEND MANTLE_FORMAT_DEPS mantle-format-clang)
    else()
        message(STATUS "clang-format: NOT FOUND (skip)")
    endif()

    if(PYTHON_BIN AND EXISTS "${CMAKE_SOURCE_DIR}/scripts/add_headers.py")
        message(STATUS "add_headers.py: ${CMAKE_SOURCE_DIR}/scripts/add_headers.py")
        add_custom_target(mantle-format-headers
            COMMAND ${PYTHON_BIN} scripts/add_headers.py --fix
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "add_headers.py: adding copyright headers"
        )
        list(APPEND MANTLE_FORMAT_DEPS mantle-format-headers)
    else()
        message(STATUS "add_headers.py: NOT FOUND (skip)")
    endif()

    if(PYTHON_BIN AND EXISTS "${CMAKE_SOURCE_DIR}/scripts/fix_switches.py")
        message(STATUS "fix_switches.py: ${CMAKE_SOURCE_DIR}/scripts/fix_switches.py")
        add_custom_target(mantle-format-switches
            COMMAND ${PYTHON_BIN} scripts/fix_switches.py --fix
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "fix_switches.py: normalizing switch-case style"
        )
        list(APPEND MANTLE_FORMAT_DEPS mantle-format-switches)
    else()
        message(STATUS "fix_switches.py: NOT FOUND (skip)")
    endif()

    if(PYTHON_BIN AND EXISTS "${CMAKE_SOURCE_DIR}/scripts/fix_includes.py")
        message(STATUS "fix_includes.py: ${CMAKE_SOURCE_DIR}/scripts/fix_includes.py")
        add_custom_target(mantle-format-includes
            COMMAND ${PYTHON_BIN} scripts/fix_includes.py --fix
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "fix_includes.py: normalizing includes"
        )
        list(APPEND MANTLE_FORMAT_DEPS mantle-format-includes)
    else()
        message(STATUS "fix_includes.py: NOT FOUND (skip)")
    endif()

    if(MANTLE_FORMAT_DEPS)
        add_custom_target(mantle-format
            COMMAND ${CMAKE_COMMAND} -E echo "Formatting complete"
            DEPENDS ${MANTLE_FORMAT_DEPS}
        )
    endif()
endif()
