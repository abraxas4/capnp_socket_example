cmake_minimum_required(VERSION 3.10)
project(CarDataDisplay)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find required packages
find_package(Qt6 COMPONENTS Widgets Network REQUIRED)  # Make sure to find the network component
find_package(CapnProto REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)


set(CAPNP_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/car_data.capnp.c++)
set(CAPNP_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/car_data.capnp.h)

add_executable(car_data_display main.cpp ${CAPNP_SRCS} ${CAPNP_HDRS})

# Set the library directories for Release libraries
target_link_directories(car_data_display PRIVATE
    "C:/capnp/capnproto-c++-1.0.2/build/src/kj/Release"
    "C:/capnp/capnproto-c++-1.0.2/build/src/capnp/Release"
)

# Link the libraries
target_link_libraries(car_data_display
    Qt6::Widgets
    Qt6::Network  # Link against the Qt6 network module
    KJ
    CapnProto::capnp
)
