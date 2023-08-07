cmake_minimum_required(VERSION 3.13)
project(EmbSettings)

# Help CMake to find conan.io modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

add_library(
	EmbSettings

	src/include/EmbSettings.hpp
	src/src/EmbSettings.cpp
)

# Need C++11
target_compile_features(EmbSettings PRIVATE cxx_std_11)

target_include_directories(EmbSettings PUBLIC src/include)
target_include_directories(EmbSettings PUBLIC C:/boost/boost_1_82_0)

set_target_properties(EmbSettings PROPERTIES PUBLIC_HEADER "src/include/EmbSettings.hpp")
install(TARGETS EmbSettings)

if(1)
    add_definitions(-DUSE_VLD)
    target_include_directories(EmbSettings PUBLIC "C:/Program Files (x86)/Visual Leak Detector/include")
    target_link_directories(EmbSettings PUBLIC "C:/Program Files (x86)/Visual Leak Detector/lib/Win64")
endif()

#add_custom_target(CHECK ALL "${CONAN_BIN_DIRS_CPPCHECK}/cppcheck" "../src/"])

if(MSVC)
    target_compile_options(EmbSettings PUBLIC "/Zc:__cplusplus")
endif()

add_executable(
	EmbSettingsExe
	src/main.cpp
)
target_link_libraries(EmbSettingsExe PRIVATE EmbSettings)
