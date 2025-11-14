cmake_minimum_required(VERSION 4.0)

project("fipsapp")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to your prebuilt libraries
set(PREBUILT_LIBS_DIR ${CMAKE_SOURCE_DIR}/../../../../prebuilt/${ANDROID_ABI})

# Add OpenSSL libraries
add_library(crypto SHARED IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIBS_DIR}/libcrypto.so)

add_library(ssl SHARED IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIBS_DIR}/libssl.so)

# for dynamic linking by libcrypto
#add_library(fips SHARED IMPORTED)
#set_target_properties(fips PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIBS_DIR}/fips.so)

# Add cURL library
add_library(curl SHARED IMPORTED)
set_target_properties(curl PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIBS_DIR}/libcurl.so)

# Include directories for OpenSSL and cURL headers
include_directories(${CMAKE_SOURCE_DIR}/../../../../prebuilt/include)

# Create the core library
add_library(core SHARED core.cpp ossl.cpp http.cpp)

# Link libraries
target_link_libraries(core
    android
    log
    crypto
    ssl
    curl
)

# Ensure libraries are loaded in correct order
set_target_properties(core PROPERTIES
    LINK_FLAGS "-Wl,--no-as-needed"
)