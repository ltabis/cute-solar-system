cmake_minimum_required(VERSION 3.16)
project(cute-solar-system CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# if(CMAKE_CXX_COMPILCMake Error: CMAKE_g++-10_COMPILER not set, after EnableLanguageER_ID MATCHES ".*Clang")
# option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
# if(ENABLE_BUILD_WITH_TIME_TRACE) target_compile_options(project_options INTERFACE -ftime-trace) endif() endif()

add_subdirectory(dependencies/Kawaii_Engine)

# compiles the files defined by SOURCES to generante the executable defined by EXEC
add_executable(cute_solar_system src/main.cpp)
target_include_directories(cute_solar_system PUBLIC include)
target_link_libraries(cute_solar_system PRIVATE kawaii_engine)
target_compile_features(cute_solar_system PRIVATE cxx_std_20)
install(TARGETS cute_solar_system DESTINATION bin)

install(DIRECTORY asset DESTINATION .)
install(FILES asset/imgui.ini DESTINATION .)
