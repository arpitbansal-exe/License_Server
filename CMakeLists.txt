cmake_minimum_required(VERSION 3.12)
project(LicenseServer)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

# Add include directories
include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${SQLite3_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/client  # Add client include directory
)

# Server executable
add_executable(license_server 
    server/main.cpp
    server/tls_server.cpp
    server/db_manager.cpp
    server/license_handler.cpp
)

# Link libraries for server
target_link_libraries(license_server 
    ${OPENSSL_LIBRARIES}
    ${SQLite3_LIBRARIES}
)

# Shared Client Library
add_library(license_client SHARED
    client/tls_client.cpp
)

target_link_libraries(license_client 
    ${OPENSSL_LIBRARIES}
)

# Client Executable
add_executable(client_app
    client/main.cpp
    client/tls_client.cpp
    client/licence.cpp
)

target_link_libraries(client_app
    ${OPENSSL_LIBRARIES}
)
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
endif()