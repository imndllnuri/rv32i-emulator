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

# Collect all core source files, EXCEPT pycpu.cpp (which is only for bindings)
file(GLOB CORE_SOURCES
    "src/*.cpp"
    "src/*.cc"
    "src/device/*.cpp"
    "instructions/*.cc"
)
# Remove pycpu.cpp if it was caught by the glob (it lives in src/)
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/pycpu.cpp")

# Create the core library (static)
add_library(riscv_core STATIC ${CORE_SOURCES})
# Force -fPIC so the static library can be linked into a shared module
set_target_properties(riscv_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Ensure the include directory is available to the library itself
target_include_directories(riscv_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Also make it public so that targets linking to riscv_core inherit it
target_include_directories(riscv_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Emulator executable (optional)
add_executable(emulator main.cc)
target_link_libraries(emulator PRIVATE riscv_core)

# Python bindings using pybind11
find_package(pybind11 REQUIRED)

# Build the Python module – only the wrapper source is needed,
# the rest comes from the static library
pybind11_add_module(rv32i_core src/pycpu.cpp)
target_link_libraries(rv32i_core PRIVATE riscv_core)
# The module also needs access to the core headers
target_include_directories(rv32i_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Build tests if enabled
if(BUILD_TESTS)
    enable_testing()
    file(GLOB TEST_SOURCES "tests/test_*.cc")
    foreach(TEST_FILE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_FILE})
        target_link_libraries(${TEST_NAME} PRIVATE riscv_core)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()
