cmake_minimum_required(VERSION 3.10)
project(CompanyC_Group6)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Declare the Catch2 dependency
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1 # Matches your documentation version
)

# Make the library available
FetchContent_MakeAvailable(Catch2)

# Define the main simulation executable
add_executable(AgentSim
    main.cpp
    Classes/BehaviorTree.cpp
    Classes/CompositeNodes.cpp
    Classes/ActionMap.cpp
    Classes/WorldPath.cpp
    Classes/PathGenerator.cpp
    Classes/LeafNodes.cpp
    Classes/DecoratorNodes.cpp
)

# Ensure the compiler can find the headers in the Classes folder
target_include_directories(AgentSim PRIVATE Classes)

# 3. Define the unit test executable
add_executable(unit_tests
    Tests/TreeTests.cpp
    Classes/BehaviorTree.cpp
    Classes/CompositeNodes.cpp
    Classes/LeafNodes.cpp
    Classes/DecoratorNodes.cpp
)

# Link Catch2 to the test executable
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(unit_tests PRIVATE Classes)

# This allows us to use 'ctest' to run our tests
include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
