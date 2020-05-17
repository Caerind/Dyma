cmake_minimum_required(VERSION 3.12...3.15)

project(DymaProject
    VERSION 0.1
    DESCRIPTION "Dyma"
    LANGUAGES CXX)
	
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(DymaExamples
	
	src/Dyma.cpp
	src/Dyma.hpp
	
	examples/main.cpp
	examples/DoubleBufferedAllocator.hpp
	examples/DebugAllocator.hpp
)
	
enable_testing()
add_executable(DymaTests

	src/Dyma.cpp
	src/Dyma.hpp

	tests/Main_Tests.cpp
	tests/NullAllocator_Tests.cpp
)
	