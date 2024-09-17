cmake_minimum_required (VERSION 3.15)

project (samp-plugin-imgui)

set(DIRECTX_SDK_PATH "C:/Program Files (x86)/DirectX SDK")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_CXX_STANDARD 11)

set(IMGUI_EXAMPLES OFF)
set(IMGUI_DEMO OFF)
set(IMGUI_IMPL_SDL OFF)
set(IMGUI_IMPL_GLFW OFF)
set(IMGUI_IMPL_GLUT OFF)
set(IMGUI_IMPL_OPENGL OFF)
set(IMGUI_IMPL_OPENGL2 OFF)
set(IMGUI_IMPL_DX9 ON)
set(IMGUI_IMPL_DX10 OFF)
set(IMGUI_IMPL_DX11 OFF)
set(IMGUI_IMPL_DX12 OFF)

include_directories(${DIRECTX_SDK_PATH}/Include)
link_directories(${DIRECTX_SDK_PATH}/Lib/x86)

include(FetchContent)

FetchContent_Declare(ktsignal GIT_REPOSITORY https://github.com/kin4stat/ktsignal.git GIT_TAG f08a19d9a00ad5284871bf44178d835da7b26ebf)
FetchContent_Declare(kthook GIT_REPOSITORY https://github.com/kin4stat/kthook.git GIT_TAG 21ca70481273d1efcc98ca49063178e46277009b)
FetchContent_Declare(rakhook GIT_REPOSITORY https://github.com/waparabka/rakhook.git)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/Qix-/imgui.git GIT_TAG 6c3b86dd78bac7ac7ce045b10825153f27603229)

FetchContent_MakeAvailable(rakhook imgui)

add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} PUBLIC rakhook imgui imgui-win32 imgui-dx9 d3d9 d3dx9 dxguid)
target_sources(${PROJECT_NAME} PRIVATE source/main.cpp source/plugin.cpp source/render.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".asi")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)