file(GLOB_RECURSE ALL_ASSETS
        LIST_DIRECTORIES false
        "${SRC}/*"
)

foreach(FILE ${ALL_ASSETS})
    if(FILE MATCHES "\\.slang$")
        continue()
    endif()

    file(RELATIVE_PATH REL "${SRC}" "${FILE}")
    configure_file("${FILE}" "${DST}/${REL}" COPYONLY)
endforeach()