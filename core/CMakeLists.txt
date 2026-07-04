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
# pybind11 ships its own CMake config inside the active Python environment's
# site-packages rather than as a system package, so find_package() alone
# won't locate it unless pybind11_DIR is already set. Auto-detect it from
# whichever `python3` is on PATH (the venv, if one is active) before falling
# back to a plain find_package() for systems where pybind11 was installed
# as a system package instead.
if(NOT pybind11_DIR)
  find_program(PYTHON_EXECUTABLE NAMES python3 python)
  if(PYTHON_EXECUTABLE)
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} -c "import pybind11; print(pybind11.get_cmake_dir())"
      OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(PYBIND11_CMAKE_DIR)
      set(pybind11_DIR "${PYBIND11_CMAKE_DIR}")
    endif()
  endif()
endif()
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
