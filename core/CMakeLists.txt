cmake_minimum_required(VERSION 3.10)
project(riscv_core VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build tests
option(BUILD_TESTS "Build the test executables" ON)

# Include directories (public so that tests can use them)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Collect all core source files
file(GLOB CORE_SOURCES
    "src/*.cpp"
    "src/*.cc"
    "src/device/*.cpp"
    "instructions/*.cc"
)

# Create the core library
add_library(riscv_core STATIC ${CORE_SOURCES})

# Ensure the include directory is available to the library itself
target_include_directories(riscv_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optional: set a property to make the library's include directory available to targets that link to it
target_include_directories(riscv_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# If you need to link any external libraries, add them here:
# target_link_libraries(riscv_core PRIVATE some_lib)

# Build tests if enabled
if(BUILD_TESTS)
    # Create a list of test executables by scanning the tests directory
    file(GLOB TEST_SOURCES "tests/test_*.cc")
    foreach(TEST_FILE ${TEST_SOURCES})
        # Extract the test name (e.g., test_add -> add)
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_FILE})
        target_link_libraries(${TEST_NAME} PRIVATE riscv_core)
        # Optionally, add a test using CTest
        # enable_testing()
        # add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()
