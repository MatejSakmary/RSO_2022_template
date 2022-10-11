cmake_minimum_required(VERSION 3.8)

project(RSO_2022_Template)

set(OpenGl_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

add_executable((${PROJECT_NAME} 
	"src/demorso.cpp"
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

# Link libraries.
target_link_libraries(${PROJECT_NAME} OpenGL::GL)
target_link_libraries(${PROJECT_NAME} glfw)