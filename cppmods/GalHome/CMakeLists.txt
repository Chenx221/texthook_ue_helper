project(GalHome)

set(TARGET GalHome)
project(${TARGET})

set(${TARGET}_Sources
        "${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.cpp"
        )

add_library(${TARGET} SHARED ${${TARGET}_Sources})

target_link_libraries(${TARGET} PRIVATE UE4SS)
