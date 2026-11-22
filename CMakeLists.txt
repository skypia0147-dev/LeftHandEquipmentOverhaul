cmake_minimum_required(VERSION 3.21)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)

option(CommonLibSSE_POSH_GENERATION "Generate .posh.d.ts type definition file" OFF)
option(CommonLibSSE_TESTING "Build tests" OFF)
set(CommonLibSSE_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(LeftHandEquipmentOverhaul VERSION 2.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# CommonLibSSE-NG via the shared F:\Anti entry point (pinned commit, shared
# prebuilt install when the MSVC toolset matches, source build otherwise).
include("F:/Anti/_commonlib/UseCommonLibSSE.cmake")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_library(${PROJECT_NAME} MODULE ${SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "LeftHandEquipmentOverhaul"
    PREFIX ""
    SUFFIX ".dll"
)

# Test environment (Fablerim: MO2 overwrite is always active)
set(TEST_PLUGINS_DIR "H:/Game/Fablerim/overwrite/SKSE/Plugins")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${TEST_PLUGINS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${TEST_PLUGINS_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying DLL to the test environment"
)
