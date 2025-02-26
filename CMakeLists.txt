# 基本設定
cmake_minimum_required(VERSION 3.22)
project(vkrenderkit LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)



file(GLOB_RECURSE PROJECT_SOURCES "code/*.cpp")


add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})



# GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# GLTF
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${TINYGLTF_INCLUDE_DIRS})


# Include
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

if(MSVC)
  target_compile_options(${PROJECT_NAME} PUBLIC "/utf-8")
endif()