cmake_minimum_required(VERSION 3.13)
project(embsettings)

# Help CMake to find conan.io modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

add_executable(
	embsettings 
	
	src/main.cpp
	src/include/embsettings.h
	src/src/embsettings.cpp
)

# Need C++20
target_compile_features(embsettings PRIVATE cxx_std_11)

# Need boost property_tree library
find_package(Boost REQUIRED)
target_link_libraries(embsettings Boost::Boost)

target_include_directories(embsettings PUBLIC src/include)

set_target_properties(embsettings PROPERTIES PUBLIC_HEADER "src/include/embsettings.hpp")
install(TARGETS embsettings)

if(1)
    add_definitions(-DUSE_VLD)
    target_include_directories(embsettings PRIVATE "C:/Program Files (x86)/Visual Leak Detector/include")
    target_link_directories(embsettings PRIVATE "C:/Program Files (x86)/Visual Leak Detector/lib/Win64")
endif()

#add_custom_target(CHECK ALL "${CONAN_BIN_DIRS_CPPCHECK}/cppcheck" "../src/"])

if(MSVC)
    target_compile_options(embsettings PUBLIC "/Zc:__cplusplus")
endif()

