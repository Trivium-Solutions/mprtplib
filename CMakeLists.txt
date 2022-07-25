cmake_minimum_required(VERSION 3.11)
project(mprtplib LANGUAGES C CXX)

# Soname
# MAJOR is incremented when symbols are removed or changed in an incompatible way
# MINOR is incremented when new symbols are added
set(PROJECT_MAJOR 0)
set(PROJECT_MINOR 0)

set(MPRTP_SOURCES
        src/mprtp_session.cpp
        src/mprtp_interface.cpp
        src/mprtp_packet.cpp
        src/mprtp_source.cpp
        src/mprtp_sender.cpp
        src/mprtp_receiver.cpp
        src/mprtp_stat.cpp
        src/mprtp_sr_packet.cpp
        src/mprtp_rr_packet.cpp
        src/mprtp_subflow.cpp
        src/mprtp_constants.cpp)

set(MPRTP_HEADERS
        include/mprtp_session.h
        include/mprtp_struct.h
        include/mprtp_interface.h
        include/mprtp_select.h
        include/mprtp_packet.h
        include/mprtp_defines.h
        include/mprtp_source.h
        include/mprtp_time.h
        include/mprtp_sender.h
        include/mprtp_subflow.h
        include/mprtp_address.h
        include/mprtp_receiver.h
        include/mprtp_stat.h
        include/mprtp_sr_packet.h
        include/mprtp_rr_packet.h
        include/mprtp_compound_packet.h
        include/mprtp_constants.h)

set(PROJECT_LIB_VERSION ${PROJECT_MAJOR}.${PROJECT_MINOR}.0)
set(PROJECT_SO_VERSION ${PROJECT_MAJOR})

add_library(${PROJECT_NAME} ${MPRTP_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_LIB_VERSION} SOVERSION ${PROJECT_SO_VERSION})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 11
        C_STANDARD 99
        PUBLIC_HEADER "${MPRTP_HEADERS}"
        )

# Export MPRTP includes.
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_include_directories(${PROJECT_NAME} PRIVATE include/${PROJECT_NAME})

add_executable(server examples/server.cpp)
set_target_properties(server PROPERTIES
    CXX_STANDARD 11
    C_STANDARD 99
        )
target_link_libraries(
        server
        ${PROJECT_NAME}
)
target_include_directories(server PRIVATE include)

add_executable(client examples/client.cpp)
set_target_properties(client PROPERTIES
        CXX_STANDARD 11
        C_STANDARD 99
        )
target_link_libraries(
        client
        ${PROJECT_NAME}
)
target_include_directories(client PRIVATE include)
