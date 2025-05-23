cmake_minimum_required(VERSION 3.8)
project(imu_triplex)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)


# Define the libi2c_external static library
add_library(libi2c_external STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/libi2c/src/i2c.c
)

# Specify include directories for libi2c_external
target_include_directories(libi2c_external PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/libi2c/include
)


# Define the test_lib static library
add_library(test_lib STATIC
    src/test_lib.cpp
)

include_directories(test_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pkg1
)






# Define the i2c_main executable
add_executable(i2c_main src/i2c_main.cpp)

# Link i2c_main against its ROS 2 dependencies (rclcpp, std_msgs)
ament_target_dependencies(i2c_main rclcpp std_msgs)

# Link i2c_main against the static libraries it uses (test_lib, libi2c_external)
# This is crucial for the linker to find the symbols defined in these libraries.
target_link_libraries(i2c_main
    test_lib
    libi2c_external
)

install(TARGETS
  i2c_main
  DESTINATION lib/${PROJECT_NAME})


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
