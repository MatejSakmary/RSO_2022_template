cmake_minimum_required(VERSION 3.8)

project(RSO_2022_Template)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(${PROJECT_NAME} 
	"src/main.cpp"
	"src/application.cpp"
	"src/utils.cpp"
	"src/raytracing_backend/material.cpp"
	"src/raytracing_backend/scene.cpp"
	"src/raytracing_backend/operations.cpp"
	"src/raytracing_backend/raytracer.cpp"
	"src/raytracing_backend/camera.cpp"
)

target_include_directories(${PROJECT_NAME}
  	PRIVATE
  	"src"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Set GLFW variables so that we don't build GLFW test etc
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("src/dep/glfw")
add_subdirectory("src/dep/glm")

# Link libraries.
target_link_libraries(${PROJECT_NAME} OpenGL::GL)
target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)