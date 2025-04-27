cmake_minimum_required(VERSION 3.10)
project(ImPlot3DExample LANGUAGES CXX C)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

# Setup glad (OpenGL 3.2 core profile)
set(GLAD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glad)
set(GLAD_SOURCE
    ${GLAD_SOURCE_DIR}/src/gl.c
)
add_library(glad STATIC ${GLAD_SOURCE})
target_include_directories(glad PUBLIC "${GLAD_SOURCE_DIR}/include")

# Setup GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw"
    GIT_TAG "3.3.8"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# Setup ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui"
    GIT_TAG "v1.91.9b"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/imgui-src)
set(IMGUI_SOURCE
    ${IMGUI_SOURCE_DIR}/imgui.cpp
    ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
    ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
    ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
	${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCE})
target_include_directories(imgui PUBLIC "${IMGUI_SOURCE_DIR};${IMGUI_SOURCE_DIR}/backends/")
target_link_libraries(imgui PUBLIC glfw glad)

# Setup ImPlot
FetchContent_Declare(
    implot
    GIT_REPOSITORY "https://github.com/epezent/implot"
    GIT_TAG "3da8bd34299965d3b0ab124df743fe3e076fa222"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(implot)
set(IMPLOT_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/implot-src)
set(IMPLOT_SOURCE
    ${IMPLOT_SOURCE_DIR}/implot.cpp
    ${IMPLOT_SOURCE_DIR}/implot_demo.cpp
    ${IMPLOT_SOURCE_DIR}/implot_items.cpp
)
add_library(implot STATIC ${IMPLOT_SOURCE})
target_include_directories(imgui PUBLIC ${IMPLOT_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# Setup ImPlot3D
set(IMPLOT3D_SOURCE_DIR "..")
set(IMPLOT3D_SOURCE
    ${IMPLOT3D_SOURCE_DIR}/implot3d.cpp
    ${IMPLOT3D_SOURCE_DIR}/implot3d_demo.cpp
    ${IMPLOT3D_SOURCE_DIR}/implot3d_items.cpp
    ${IMPLOT3D_SOURCE_DIR}/implot3d_meshes.cpp
)
add_library(implot3d STATIC ${IMPLOT3D_SOURCE})
target_include_directories(implot3d PUBLIC ${IMPLOT3D_SOURCE_DIR})
target_link_libraries(implot3d PUBLIC imgui)

# Add the executable
set(EXAMPLE_SOURCE
    main.cpp
)
add_executable(example ${EXAMPLE_SOURCE})
target_link_libraries(example PRIVATE implot implot3d)
